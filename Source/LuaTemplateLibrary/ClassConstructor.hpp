#pragma once
#include "FuncArguments.hpp"
#include "FuncUtils.hpp"
#include "LuaAux.hpp"
#include "LuaTypes.hpp"
#include "UserData.hpp"

namespace LTL
{
    /**
     * @brief Класс для преобразования конструктора в универсальную функцию.
     *
     * @tparam C Класс.
     * @tparam TArgs Типы аргументов конструктора.
     */
    template <class C, typename... TArgs>
    struct Constructor
    {
        using ArgsTuple = std::tuple<Unwrap_t<TArgs>...>;

        static int Function(lua_State* l)
        {
            ArgsTuple args;
            Constructor::GetArgs(l, args);
            Create(l, args, std::index_sequence_for<TArgs...>{});
            return 1;
        }

    private:
        static constexpr size_t GetArgs(lua_State* l, ArgsTuple& args)
        {
            return FuncUtility::GetArgs<ArgsTuple, TArgs...>(l, args);
        }

        template <size_t... Is>
        static constexpr void Create(lua_State* l, ArgsTuple& args, const std::index_sequence<Is...>)
        {
            return UnpackArgs(l, std::get<Is>(args)...);
        }

        static constexpr void UnpackArgs(lua_State* l, Unwrap_t<TArgs> &...args)
        {
            UserData<C>::New(l, std::forward<Unwrap_t<TArgs>>(args)...);
        }
    };

    template <typename... TArgs>
    struct MatchArgumentTypes
    {
        static constexpr size_t max_arg_count = FuncUtility::MaxArgumentCount<TArgs...>();
        static constexpr size_t min_arg_count = FuncUtility::MinArgumentCount<TArgs...>();

        static constexpr bool Predicate(lua_State* l)
        {
            return FuncUtility::MatchesTypes<TArgs...>(l);
        }

        static constexpr bool MatchArgumentCount(lua_State* l)
        {
            if constexpr (min_arg_count == max_arg_count)
            {
                return max_arg_count == lua_gettop(l);
            }
            else
            {
                const int n = lua_gettop(l);
                return n >= min_arg_count && n <= max_arg_count;
            }
        }

        static int Function(lua_State* l)
        {
            lua_pushboolean(l, MatchArgumentCount(l) && Predicate(l));
            return 1;
        }
    };
}