#pragma once
#include "LuaAux.hpp"
#include "LuaTypes.hpp"
#include "FuncArguments.hpp"
#include "Exception.hpp"
#include "RefObject.hpp"

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
    struct UserData : Internal::UserDataValue<T>
    {
        using UserDataValue::UserDataValue;

        static void* const  Allocate(lua_State* l)
        {
            void* const obj = lua_newuserdata(l, sizeof(T));
            SetClassMetaTable(l);
            return obj;
        }

        static void PushCopy(lua_State* l, T&& other)
        {
            static_assert(std::is_move_constructible_v<T>, "Can't move-construct type T!");
            new(Allocate(l)) T(std::forward<T>(other));
        }

        template<typename ...TArgs>
        static GRefObject Make(lua_State* l, TArgs&&... args)
        {
            new(Allocate(l)) T(std::forward<TArgs>(args)...);
            return GRefObject::FromTop(l);
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

        static bool IsUserData(lua_State* l, int index)
        {
            if (!lua_isuserdata(l, index))
            {
                return false;
            }

            if (!lua_getmetatable(l, index))
            {
                return false;
            }

            if (MetaTable::Push(l) == LUA_TNIL)
            {
                lua_pop(l, 1);
                return false;
            }
            StackPopper pop{ l, 2 };
            return lua_rawequal(l, -2, -1);
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

            if (MetaTable::Push(l) == LUA_TNIL)
            {
                lua_pop(l, 1);
                ThrowNoMetaTableForUD(l, index);
                return nullptr;
            }

            bool res = lua_rawequal(l, -2, -1);
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
            IndexTable::Push(l);
            lua_pushvalue(l, 2);
            int type = lua_rawget(l, -2);
            if (type == LUA_TNIL)
            {
                lua_pop(l, 2); // pop nil and index table

                MetaTable::Push(l);
                lua_pushvalue(l, 2);
                lua_rawget(l, -2); // get value in metatable

                lua_remove(l, -2); // remove metatable

                return 1;
            }

            lua_remove(l, -2); // remove index table
            lua_pushvalue(l, 1); // push userdata

            assert(lua_isuserdata(l, -1));
            assert(lua_isfunction(l, -2));

            lua_call(l, 1, 1);

            return 1;
        }

        static int NewIndexMethod(lua_State* l)
        {
            NewIndexTable::Push(l);
            lua_pushvalue(l, 2);
            int type = lua_rawget(l, -2);
            if (type == LUA_TNIL)
            {
                lua_pop(l, 2); // pop nil and index table

                const char* s = lua_tostring(l, 2);
                luaL_error(l, "Attempt to set field '%s' on %s", s ? s : "NONSTRINGVALUE", GetClassName(l));

                return 1;
            }
            lua_remove(l, -2); // remove newindex table
            lua_pushvalue(l, 1); // push userdata
            lua_pushvalue(l, 3); // push value

            assert(lua_isuserdata(l, -2));
            assert(lua_isfunction(l, -3));

            lua_call(l, 2, 0);

            return 0;
        }

        static const char* GetClassName(lua_State* l)
        {
            StackPopper pop{ l, 2 };
            ClassTable::Push(l);
            lua_pushstring(l, "className");
            lua_rawget(l, -2);
            return lua_tostring(l, -1);
        }

    private:
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

        static void ThrowNoMetaTableForUD(lua_State* l, int index)
        {
            luaL_argerror(l, index, "Expected userdata with metatable");
        }
    };

    template<typename T>
    struct StackType<UserData<T>>
    {
        using UD = UserData<T>;

        static bool Check(lua_State* l, int index)
        {
            return UD::IsUserData(l, index);
        }

        static UD Get(lua_State* l, int index)
        {
            return UD::ValidateUserData(l, index);
        }

        static void Push(lua_State* l, T&& value)
        {
            UD::PushCopy(l, std::forward<T>(value));
        }
    };

}