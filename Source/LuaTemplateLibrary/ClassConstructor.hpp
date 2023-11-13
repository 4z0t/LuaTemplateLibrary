#pragma once
#include "LuaAux.hpp"
#include "LuaTypes.hpp"
#include "FuncArguments.hpp"
#include "UserData.hpp"
#include "FuncUtils.hpp"


namespace Lua
{
    template<class C, typename ...TArgs>
    struct Constructor
    {
        using ArgsTuple = std::tuple<Unwrap_t<TArgs>...>;

        static int Function(lua_State* l)
        {
            ArgsTuple args;
            Constructor::GetArgs(l, args);
            UnpackArgs(UserData<C>::Allocate(l), args, std::index_sequence_for<TArgs...>{});
            return 1;
        }

    private:
        static constexpr size_t GetArgs(lua_State* l, ArgsTuple& args)
        {
            return FuncUtility::GetArgs<ArgsTuple, TArgs...>(l, args);
        }

        template <size_t ... Is>
        static constexpr void UnpackArgs(void* const object, ArgsTuple& args, const std::index_sequence<Is...>)
        {
            return Make(object, std::get<Is>(args)...);
        }

        static constexpr void Make(void* const object, Unwrap_t<TArgs>& ...args)
        {
            static_assert(std::is_constructible_v<C, Unwrap_t<TArgs>...>, "Object of class C can't be constructed with such arguments!");
            new(object) C(args...);
        }
    };

    template<typename ...TArgs>
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
                int n = lua_gettop(l);
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