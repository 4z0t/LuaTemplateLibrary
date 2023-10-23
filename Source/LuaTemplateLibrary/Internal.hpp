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
#include "Lua/lua.hpp"

#define STATIC_FAIl(message) static_assert(false, (message))

namespace Lua::Internal
{
    template<typename T>
    struct IntParser
    {
        static_assert(std::is_integral_v<T>, "Provided not integral type");

        static T Get(lua_State* l, int index)
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

        static T Get(lua_State* l, int index)
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


}