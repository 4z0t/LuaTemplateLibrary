#pragma once
#include "Internal.hpp"

namespace Lua
{

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

    template<typename T>
    struct TypeParser
    {
        static T Get(lua_State* l, int index)
        {
            STATIC_FAIl("Not provided implementation for Get function");
        }

        static bool Check(lua_State* l, int index)
        {
            STATIC_FAIl("Not provided implementation for Check function");
        }

        static void Push(lua_State* l, const T& value)
        {
            STATIC_FAIl("Not provided implementation for Push function");
        }
    };

    template<>
    struct TypeParser<char> : Internal::IntParser<char> {};
    template<>
    struct TypeParser<unsigned char> : Internal::IntParser<unsigned char> {};
    template<>
    struct TypeParser<int> : Internal::IntParser<int> {};
    template<>
    struct TypeParser<unsigned int> : Internal::IntParser<unsigned int> {};
    template<>
    struct TypeParser<short> : Internal::IntParser<short> {};
    template<>
    struct TypeParser<unsigned short> : Internal::IntParser<unsigned short> {};
    template<>
    struct TypeParser<long long> : Internal::IntParser<long long> {};
    template<>
    struct TypeParser<unsigned long long> : Internal::IntParser<unsigned long long> {};


    /* template<>
     struct TypeParser<std::nullptr_t>
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
    struct TypeParser<float> : Internal::FloatParser<float> {};
    template<>
    struct TypeParser<double> : Internal::FloatParser<double> {};
    template<>
    struct TypeParser<long double> : Internal::FloatParser<long double> {};

    template<>
    struct TypeParser<bool>
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
    struct TypeParser<const char*>
    {
        static const char* Get(lua_State* l, int index)
        {
            return luaL_checkstring(l, index);
        }

        static bool Check(lua_State* l, int index)
        {
            return lua_isstring(l, index);
        }

        static void Push(lua_State* l, const char* value)
        {
            lua_pushstring(l, value);
        }
    };

    template<>
    struct TypeParser<std::string> : public TypeParser<const char*>
    {
        static std::string Get(lua_State* l, int index)
        {
            return { TypeParser<const char*>::Get(l, index) };
        }

        static void Push(lua_State* l, const std::string& value)
        {
            TypeParser<const char*>::Push(l, value.c_str());
        }
    };

    template<>
    struct TypeParser<lua_CFunction>
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
    struct TypeParser<std::vector<T>>
    {
        static std::vector<T> Get(lua_State* l, int index)
        {
            lua_pushvalue(l, index);
            auto size = lua_rawlen(l, -1);
            std::vector<T> result(size);
            for (size_t i = 0; i < size; i++) {
                lua_rawgeti(l, -1, i + 1);
                result[i] = TypeParser<T>::Get(l, -1);
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
                TypeParser<T>::Push(l, value[i]);
                lua_rawseti(l, -2, i + 1);
            }
        }
    };
}
