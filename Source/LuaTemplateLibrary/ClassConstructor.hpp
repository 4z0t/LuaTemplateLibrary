#pragma once
#include "LuaAux.hpp"
#include "LuaTypes.hpp"
#include "FuncArguments.hpp"
#include "UserData.hpp"


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
}