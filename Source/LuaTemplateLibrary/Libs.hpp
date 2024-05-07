#pragma once
#include "LuaAux.hpp"
#include "Types.hpp"
#include "CState.hpp"

namespace LTL
{
    class Lib
    {
    public:
        Lib() = delete;
        constexpr Lib(const char* const name, lua_CFunction const func) : name(name), func(func) {}

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
        constexpr Lib base{ LUA_GNAME, luaopen_base };
        constexpr Lib package{ LUA_LOADLIBNAME, luaopen_package };
        constexpr Lib coroutine{ LUA_COLIBNAME, luaopen_coroutine };
        constexpr Lib table{ LUA_TABLIBNAME, luaopen_table };
        constexpr Lib io{ LUA_IOLIBNAME, luaopen_io };
        constexpr Lib os{ LUA_OSLIBNAME, luaopen_os };
        constexpr Lib string{ LUA_STRLIBNAME, luaopen_string };
        constexpr Lib math{ LUA_MATHLIBNAME, luaopen_math };
        constexpr Lib utf8{ LUA_UTF8LIBNAME, luaopen_utf8 };
        constexpr Lib debug{ LUA_DBLIBNAME, luaopen_debug };
    }

}