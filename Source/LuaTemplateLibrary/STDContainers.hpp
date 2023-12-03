#pragma once
#include "LuaTypes.hpp"
#include <map>
#include <unordered_map>
#include <optional>
#include "StackObject.hpp"
#include "FuncArguments.hpp"

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
            StackObjectView table{ l , index };

            auto size = table.RawLen();
            std::vector<T> result(size);
            for (size_t i = 0; i < size; i++) {
                result[i] = table.RawGetI<T>(i + 1);
            }

            return result;
        }

        static void Push(lua_State* l, const std::vector<T>& value)
        {
            lua_createtable(l, value.size(), 0);
            StackObjectView table{ l };
            for (size_t i = 0; i < value.size(); i++) {
                table.RawSetI(i + 1, value[i]);
            }
        }
    };

    template<typename K, typename V>
    struct StackType<std::unordered_map<K, V>> :TableChecker
    {
        using Type = std::unordered_map<K, V>;

        static Type Get(lua_State* l, int index)
        {
            StackObjectView table{ l, index };
            Type result{};

            lua_pushnil(l);
            while (lua_next(l, table.GetIndex()))
            {
                StackObjectView key{ l,-2 };
                StackObjectView value{ l,-1 };
                result.insert({ key.To<K>(),value.To<V>() });
                lua_pop(l, 1);
            }

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
        }
    };

    template<typename T>
    struct StackType<std::optional<T>> : CheckOptional<T>
    {
        using Type = std::optional<T>;

        static Type Get(lua_State* l, int index)
        {
            if (lua_isnoneornil(l, index))
            {
                return std::nullopt;
            }
            return { StackType<T>::Get(l, index) };
        }

        static void Push(lua_State* l, const Type& value)
        {
            if (value.has_value())
            {
                StackType<T>::Push(l, value.value());
            }
            else
            {
                lua_pushnil(l);
            }
        }
    };

}