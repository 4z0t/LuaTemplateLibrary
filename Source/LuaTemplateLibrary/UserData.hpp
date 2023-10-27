#pragma once
#include "LuaAux.hpp"
#include "LuaTypes.hpp"

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
            if constexpr (std::is_destructible_v<T>)
            {
                T* data = static_cast<T*>(lua_touserdata(l, 1));
                data->~T();
            }
            return 0;
        }


        static void ThrowInvalidUserData(lua_State* l, int index)
        {
            //...
        }

        static T* ValidateUserData(lua_State* l, int index)
        {
            if (!lua_isuserdata(l, index))
            {
                ThrowInvalidUserData(l, index);
                return nullptr;
            }
            //...

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
            return lua_rawgetp(l, LUA_REGISTRYINDEX, GetMetaTableKey());
        }

        static int PushClassTable(lua_State* l)
        {
            return lua_rawgetp(l, LUA_REGISTRYINDEX, GetClassTableKey());
        }
    };
}