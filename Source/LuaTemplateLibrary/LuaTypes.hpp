#pragma once
#include "Internal.hpp"

namespace Lua
{
    class CState;

    CState* WrapState(lua_State* l);

    enum class Type
    {
        None = LUA_TNONE,
        Nil = LUA_TNIL,
        Boolean = LUA_TBOOLEAN,
        Lightdata = LUA_TLIGHTUSERDATA,
        Number = LUA_TNUMBER,
        String = LUA_TSTRING,
        Table = LUA_TTABLE,
        Function = LUA_TFUNCTION,
        Userdata = LUA_TUSERDATA,
        Thread = LUA_TTHREAD,
    };

    struct AlwaysValid
    {
        static bool Check(lua_State* l, int index)
        {
            return true;
        }
    };

    template<typename T>
    struct StackType
    {
        static T Get(lua_State* l, int index)
        {
            STATIC_FAIL("Not provided implementation for Get function");
            return {};
        }

        static bool Check(lua_State* l, int index)
        {
            STATIC_FAIL("Not provided implementation for Check function");
        }

        static void Push(lua_State* l, const T& value)
        {
            STATIC_FAIL("Not provided implementation for Push function");
        }
    };

    template<>
    struct StackType<lua_State*> : AlwaysValid
    {
        static lua_State* Get(lua_State* l, int index)
        {
            return l;
        }
    };

    template<>
    struct StackType<CState*> : AlwaysValid
    {
        static CState* Get(lua_State* l, int index)
        {
            return WrapState(l);
        }
    };

    template<>
    struct StackType<char> : Internal::IntParser<char> {};
    template<>
    struct StackType<unsigned char> : Internal::IntParser<unsigned char> {};
    template<>
    struct StackType<int> : Internal::IntParser<int> {};
    template<>
    struct StackType<unsigned int> : Internal::IntParser<unsigned int> {};
    template<>
    struct StackType<short> : Internal::IntParser<short> {};
    template<>
    struct StackType<unsigned short> : Internal::IntParser<unsigned short> {};
    template<>
    struct StackType<long long> : Internal::IntParser<long long> {};
    template<>
    struct StackType<unsigned long long> : Internal::IntParser<unsigned long long> {};


    /* template<>
     struct StackType<std::nullptr_t>
     {
         static bool Check(lua_State* l, int index)
         {
             return lua_isnil(l, index);
         }

         static void Push(lua_State* l, std::nullptr_t)
         {
             lua_pushnil(l);
         }
     };*/

    template<>
    struct StackType<void>
    {
        static bool Check(lua_State* l, int index)
        {
            return lua_isnil(l, index);
        }
    };

    template<>
    struct StackType<float> : Internal::FloatParser<float> {};
    template<>
    struct StackType<double> : Internal::FloatParser<double> {};
    template<>
    struct StackType<long double> : Internal::FloatParser<long double> {};

    template<>
    struct StackType<bool>
    {
        static bool Get(lua_State* l, int index)
        {
            return lua_toboolean(l, index);
        }

        static bool Check(lua_State* l, int index)
        {
            return lua_isboolean(l, index);
        }

        static void Push(lua_State* l, bool value)
        {
            lua_pushboolean(l, value);
        }
    };

    template<>
    struct StackType<const char*> : Internal::StringParser<const char*> {};
    template<>
    struct StackType<std::string> : Internal::StringParser<std::string> {};
    template<>
    struct StackType<std::string_view> : Internal::StringParser<std::string_view> {};

    template<>
    struct StackType<lua_CFunction>
    {
        static lua_CFunction Get(lua_State* l, int index)
        {
            return lua_tocfunction(l, index);
        }

        static bool Check(lua_State* l, int index)
        {
            return lua_iscfunction(l, index);
        }

        static void Push(lua_State* l, lua_CFunction value)
        {
            lua_pushcfunction(l, value);
        }
    };

    template<typename T>
    struct StackType<std::vector<T>>
    {
        static std::vector<T> Get(lua_State* l, int index)
        {
            lua_pushvalue(l, index);
            auto size = lua_rawlen(l, -1);
            std::vector<T> result(size);
            for (size_t i = 0; i < size; i++) {
                lua_rawgeti(l, -1, i + 1);
                result[i] = StackType<T>::Get(l, -1);
                lua_pop(l, 1);
            }
            lua_pop(l, 1);
            return result;
        }

        static bool Check(lua_State* l, int index)
        {
            return lua_istable(l, index);
        }

        static void Push(lua_State* l, const std::vector<T>& value)
        {
            lua_createtable(l, value.size(), 0);
            for (size_t i = 0; i < value.size(); i++) {
                PushValue(l, value[i]);
                lua_rawseti(l, -2, i + 1);
            }
        }
    };
}
