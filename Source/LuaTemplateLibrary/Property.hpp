#pragma once
#include "LuaAux.hpp"
#include "LuaTypes.hpp"
#include "FuncArguments.hpp"
#include "UserData.hpp"


namespace Lua
{

    struct PropertyBase {};

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
            T value = StackType<T>::Get(l, 2);
            ud->*Field = std::move(value);
            return 0;
        }
    };

    template<class C, typename T, T C::* Field>
    struct Property: public PropertyBase
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

    /*template<auto Field>
    struct AProperty
    {
        using TField = decltype(Field);

        static int Get(lua_State* l)
        {
            return __Get<TField>::Call(l);
        }

        static int Set(lua_State* l)
        {
            return __Set<TField>::Call(l);
        }
    private:
        template<class C, typename T>
        struct __Get;

        template<class C, typename T>
        struct __Set;

        template<class C, typename T>
        struct __Get<T C::* >
        {
            static int Call(lua_State* l)
            {
                return Getter<C, T, Field>::Function(l);
            }
        };

        template<class C, typename T>
        struct __Set<T C::*>
        {
            static int Call(lua_State* l)
            {
                return Getter<C, T, Field>::Function(l);
            }
        };
    };*/

}