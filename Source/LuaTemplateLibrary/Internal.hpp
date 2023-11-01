#pragma once
#include <memory>
#include <assert.h>
#include <iostream>
#include <tuple>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <stdint.h>
#include "Lua/LuaLibrary.h"

#define STATIC_FAIL(message) static_assert(false, message)

namespace Lua::Internal
{
    template<typename T>
    struct IntParser
    {
        static_assert(std::is_integral_v<T>, "Provided not integral type");

        using TReturn = T;

        static TReturn Get(lua_State* l, int index)
        {
            return static_cast<T>(luaL_checkinteger(l, index));
        }

        static bool Check(lua_State* l, int index)
        {
            return lua_isinteger(l, index);
        }

        static void Push(lua_State* l, const T& value)
        {
            lua_pushinteger(l, static_cast<lua_Integer>(value));
        }
    };

    template<typename T>
    struct FloatParser
    {
        static_assert(std::is_floating_point_v<T>, "Provided not floating point type");

        using TReturn = T;

        static TReturn Get(lua_State* l, int index)
        {
            return static_cast<T>(luaL_checknumber(l, index));
        }

        static bool Check(lua_State* l, int index)
        {
            return lua_isnumber(l, index);
        }

        static void Push(lua_State* l, const T& value)
        {
            lua_pushnumber(l, static_cast<lua_Number>(value));
        }
    };

    inline void lua_setregp(lua_State* l, const void* p)
    {
        return lua_rawsetp(l, LUA_REGISTRYINDEX, p);
    }

    inline int lua_getregp(lua_State* l, const void* p)
    {
        return lua_rawgetp(l, LUA_REGISTRYINDEX, p);
    }
}