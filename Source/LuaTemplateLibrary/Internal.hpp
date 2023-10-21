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
#include "lua.hpp"

namespace Lua::Internal
{
    template<typename T>
    struct IntParser
    {
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


}