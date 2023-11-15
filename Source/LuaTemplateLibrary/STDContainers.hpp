#pragma once
#include "LuaTypes.hpp"
#include <map>
#include <unordered_map>
#include "StackObject.hpp"

namespace Lua
{

    struct TableChecker
    {
        static bool Check(lua_State* l, int index)
        {
            return lua_istable(l, index);
        }
    };

    template<typename T>
    struct StackType<std::vector<T>> :TableChecker
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

        static void Push(lua_State* l, const std::vector<T>& value)
        {
            lua_createtable(l, value.size(), 0);
            for (size_t i = 0; i < value.size(); i++) {
                PushValue(l, value[i]);
                lua_rawseti(l, -2, i + 1);
            }
        }
    };

    template<typename K, typename V>
    struct StackType<std::unordered_map<K, V>> :TableChecker
    {
        using Type = std::unordered_map<K, V>;

        static Type Get(lua_State* l, int index)
        {
            lua_pushvalue(l, index);
            StackObjectView table{ l };
            Type result{};

            lua_pushnil(l);
            while (lua_next(l, table.GetIndex()))
            {
                StackObjectView key{ l,-2 };
                StackObjectView value{ l,-1 };
                result.insert({ key.To<K>(),value.To<V>() });
                lua_pop(l, 1);
            }
            lua_pop(l, 1);

            return result;
        }

        static void Push(lua_State* l, const Type& value)
        {
            lua_createtable(l, 0, value.size());
            StackObjectView table{ l };
            
            for (const auto& [k, v] : value)
            {
                table.RawSet(k, v);
            }

            return result;
        }
    };


}