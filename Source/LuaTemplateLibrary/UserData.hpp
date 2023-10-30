#pragma once
#include "LuaAux.hpp"
#include "LuaTypes.hpp"
#include "FuncArguments.hpp"
#include "Exception.hpp"

namespace Lua
{
    template<typename T>
    struct RegistryTableBase
    {
        static const void* GetKey()
        {
            static const char key;
            return &key;
        }

        static int Push(lua_State* l)
        {
            return lua_getregp(l, RegistryTableBase<T>::GetKey());
        }
    };

    template<typename T>
    struct UserData
    {
        static void CopyFunction(lua_State* l, T&& other)
        {
            static_assert(std::is_copy_constructible_v<T>, "Can't copy construct type T!");
            new(lua_newuserdata(l, sizeof(T))) T(std::forward<T>(other));
            SetClassMetaTable(l);
        }

        static void SetClassMetaTable(lua_State* l)
        {
            if (MetaTable::Push(l) != LUA_TTABLE)
            {
                throw std::logic_error("The class was't registered");
            }
            lua_setmetatable(l, -2);
        }

        static int DestructorFunction(lua_State* l)
        {
            T* data = static_cast<T*>(lua_touserdata(l, 1));
            data->~T();

            return 0;
        }


        static void ThrowInvalidUserData(lua_State* l, int index)
        {
            luaL_argerror(l, index, "invalind UserData");
        }

        static void ThrowWrongUserDataType(lua_State* l, int index)
        {
            luaL_error(l, "Expected %s but got userdata", GetClassName(l));
        }

        static void ThrowWrongType(lua_State* l, int index)
        {
            luaL_error(l, "Expected %s but got %s", GetClassName(l), lua_typename(l, lua_type(l, index)));
        }

        static T* ValidateUserData(lua_State* l, int index)
        {
            if (!lua_isuserdata(l, index))
            {
                ThrowWrongType(l, index);
                return nullptr;
            }

            if (!lua_getmetatable(l, index))
            {
                ThrowInvalidUserData(l, index);
                return nullptr;
            }

            MetaTable::Push(l);
            bool res = lua_compare(l, -2, -1, LUA_OPEQ);
            lua_pop(l, 2);

            if (!res)
            {
                ThrowWrongUserDataType(l, index);
                return nullptr;
            }

            return static_cast<T*>(lua_touserdata(l, index));
        }

        struct MetaTable : public RegistryTableBase<MetaTable> {};
        struct ClassTable : public RegistryTableBase<ClassTable> {};
        struct IndexTable : public RegistryTableBase<IndexTable> {};
        struct NewIndexTable : public RegistryTableBase<NewIndexTable> {};

        static int IndexMethod(lua_State* l)
        {

            return 0;
        }

        static int NewIndexMethod(lua_State* l)
        {

            return 0;
        }

        static const char* GetClassName(lua_State* l)
        {
            ClassTable::Push(l);
            lua_pushstring(l, "className");
            lua_rawget(l, -2);
            const char* s = lua_tostring(l, -1);
            lua_pop(l, 2);
            return s;
        }

    };

    template<typename T>
    struct UserDataValue : TypeBase<T*> {};

    template<typename T>
    struct StackType<UserDataValue<T>>
    {
        using TReturn = T*;

        static TReturn Get(lua_State* l, int index)
        {
            return UserData<T>::ValidateUserData(l, index);
        }

        static void Push(lua_State* l, T&& value)
        {
            UserData<T>::CopyFunction(l, std::forward<T>(value));
        }

    };

}