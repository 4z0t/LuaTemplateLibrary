#pragma once
#include "LuaAux.hpp"
#include "LuaTypes.hpp"
#include "CState.hpp"

namespace LTL
{
    class Lib
    {
    public:
        Lib() = delete;
        Lib(const char* const name, lua_CFunction const func) : name(name), func(func) {}

        inline void Register(lua_State* l) const
        {
            luaL_requiref(l, name, func, 1);
            lua_pop(l, 1);
        }

        ~Lib() = default;
    private:
        const char* const name;
        lua_CFunction const func;
    };


    namespace Libs
    {
        const Lib global{ LUA_GNAME, luaopen_base };
        const Lib package{ LUA_LOADLIBNAME, luaopen_package };
        const Lib coroutine{ LUA_COLIBNAME, luaopen_coroutine };
        const Lib table{ LUA_TABLIBNAME, luaopen_table };
        const Lib io{ LUA_IOLIBNAME, luaopen_io };
        const Lib os{ LUA_OSLIBNAME, luaopen_os };
        const Lib string{ LUA_STRLIBNAME, luaopen_string };
        const Lib math{ LUA_MATHLIBNAME, luaopen_math };
        const Lib utf8{ LUA_UTF8LIBNAME, luaopen_utf8 };
        const Lib debug{ LUA_DBLIBNAME, luaopen_debug };
    }

}