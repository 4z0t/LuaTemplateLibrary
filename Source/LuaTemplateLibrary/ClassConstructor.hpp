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
            void* obj = lua_newuserdata(l, sizeof(C));
            UnpackArgs(obj, args, std::index_sequence_for<TArgs...>{});
            UserData<C>::SetClassMetaTable(l);
            return 1;
        }

    private:
        static constexpr size_t GetArgs(lua_State* l, ArgsTuple& args)
        {
            return FuncUtility::GetArgs<0, 0, ArgsTuple, TArgs...>(l, args);
        }

        template <size_t ... Is>
        static constexpr void UnpackArgs(void* const object, ArgsTuple& args, const std::index_sequence<Is...>)
        {
            return Make(object, std::get<Is>(args)...);
        }

        static constexpr void Make(void* const object, TArgs&& ...args)
        {
            static_assert(std::is_constructible_v<C, TArgs...>, "Object of class C can't be constructed with such arguments!");
            new(object) C(std::forward<TArgs>(args)...);
        }
    };
}