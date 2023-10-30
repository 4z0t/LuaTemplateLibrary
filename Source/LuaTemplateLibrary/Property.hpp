#pragma once
#include "LuaAux.hpp"
#include "LuaTypes.hpp"
#include "FuncArguments.hpp"
#include "UserData.hpp"


namespace Lua
{

    template<class C, typename T, T C::* Field>
    struct Getter
    {
        static int Function(lua_State* l)
        {
            C* ud = UserData<C>::ValidateUserData(l, 1);
            PushValue(l, ud->*Field);
            return 1;
        }

    };

    template<class C, typename T, T C::* Field>
    struct Setter
    {
        static int Function(lua_State* l)
        {
            C* ud = UserData<C>::ValidateUserData(l, 1);
            T value = StackType<T>:Get(l, 2);
            ud->*Field = std::move(value);
            return 0;
        }
    };

    template<class C, typename T, T C::* Field>
    struct Property
    {
        static int Get(lua_State* l)
        {
            return Getter<C, T, Field>::Function(l);
        }

        static int Set(lua_State* l)
        {
            return Setter<C, T, Field>::Function(l);
        }
    };
}