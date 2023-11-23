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

inline void lua_setregp(lua_State* l, const void* p)
{
    return lua_rawsetp(l, LUA_REGISTRYINDEX, p);
}

inline int lua_getregp(lua_State* l, const void* p)
{
    return lua_rawgetp(l, LUA_REGISTRYINDEX, p);
}

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

    template<typename T>
    struct StringParser
    {
        static T Get(lua_State* l, int index)
        {
            return { luaL_checkstring(l, index) };
        }

        static bool Check(lua_State* l, int index)
        {
            return lua_isstring(l, index);
        }

        static void Push(lua_State* l, const T& value)
        {
            if constexpr (std::is_pointer_v<T>)
            {
                lua_pushstring(l, value);
            }
            else
            {
                lua_pushstring(l, value.data());
            }
        }
    };

    template<typename OptType, typename T>
    struct OptionalInteger
    {
        static_assert(std::is_integral_v<T>, "Provided not integral type");

        static constexpr T Get(lua_State* l, int index)
        {
            return static_cast<T>(
                luaL_optinteger(l, index, static_cast<lua_Integer>(OptType::value))
                );
        }
    };

    template<typename OptType, typename T>
    struct OptionalFloat
    {
        static_assert(std::is_floating_point_v<T>, "Provided not floating point type");

        static constexpr T Get(lua_State* l, int index)
        {
            return static_cast<T>(
                luaL_optnumber(l, index, static_cast<lua_Number>(OptType::value))
                );
        }
    };

    struct UserDataValueBase {};

    template<typename T>
    struct UserDataValue :UserDataValueBase
    {
        UserDataValue() :UserDataValue(nullptr) {}

        UserDataValue(T* value) :m_value(value) {}

        operator T& ()
        {
            return *m_value;
        }

        operator T* const ()
        {
            return m_value;
        }

        const T* const operator->()const
        {
            return m_value;
        }

        const T& operator*()const
        {
            return *m_value;
        }

        T* const operator->()
        {
            return m_value;
        }

        T& operator*()
        {
            return *m_value;
        }

    private:
        T* m_value = nullptr;
    };
}