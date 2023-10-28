#pragma once
#include "LuaAux.hpp"
#include "LuaTypes.hpp"
#include "FuncArguments.hpp"

namespace Lua
{
    template<typename T>
    struct UserData
    {
        static int ConstructorFunction(lua_State* l)
        {
            new(lua_newuserdata(l, sizeof(T))) T();
            if (PushMetaTable(l) != LUA_TTABLE)
            {
                throw std::logic_error("The class was't registered");
            }
            lua_setmetatable(l, -2);
            return 1;
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

            PushMetaTable(l);
            bool res = lua_compare(l, -2, -1, LUA_OPEQ);
            lua_pop(l, 2);

            if (!res)
            {
                ThrowWrongUserDataType(l, index);
                return nullptr;
            }

            return static_cast<T*>(lua_touserdata(l, index));
        }

        static const void* GetMetaTableKey()
        {
            static const char key;
            return &key;
        }

        static const void* GetClassTableKey()
        {
            static const char key;
            return &key;
        }


        static int PushMetaTable(lua_State* l)
        {
            return lua_getregp(l, GetMetaTableKey());
        }

        static int PushClassTable(lua_State* l)
        {
            return lua_getregp(l, GetClassTableKey());
        }

        static const char* GetClassName(lua_State* l)
        {
            PushClassTable(l);
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


    };

}