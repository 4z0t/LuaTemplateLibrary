#pragma once
#include "LuaAux.hpp"
#include "LuaTypes.hpp"
#include "FuncArguments.hpp"
#include "UserData.hpp"


namespace LTL
{

    struct GetterBase {};
    struct SetterBase {};
    struct PropertyBase {};

    template<class C, typename T, T C::* Field>
    struct Getter :public GetterBase
    {
        using TClass = C;

        static int Function(lua_State* l)
        {
            C* ud = UserData<C>::ValidateUserData(l, 1);
            PushValue(l, ud->*Field);
            return 1;
        }

    };

    template<class C, typename T, T C::* Field>
    struct Setter :public SetterBase
    {
        using TClass = C;

        static int Function(lua_State* l)
        {
            C* ud = UserData<C>::ValidateUserData(l, 1);
            T value = StackType<T>::Get(l, 2);
            ud->*Field = std::move(value);
            return 0;
        }
    };

    template<class C, typename T, T C::* Field>
    struct Property : public PropertyBase
    {
        using TClass = C;

        static int Get(lua_State* l)
        {
            return Getter<C, T, Field>::Function(l);
        }

        static int Set(lua_State* l)
        {
            return Setter<C, T, Field>::Function(l);
        }
    };


    template<auto Field>
    struct FieldDeductor
    {
        template<typename C, typename T>
        struct _Deduced
        {
            using Class = C;
            using Type = T;
        };

        template<typename C, typename T>
        static _Deduced<C, T> Deduce(T C::* member) {}

        using Deduced = typename decltype(Deduce(Field));

        using Class = typename Deduced::Class;
        using Type = typename Deduced::Type;
    };



    template<auto Field>
    struct AGetter : Getter<typename FieldDeductor<Field>::Class, typename FieldDeductor<Field>::Type, Field> {};

    template<auto Field>
    struct ASetter : Setter<typename FieldDeductor<Field>::Class, typename FieldDeductor<Field>::Type, Field> {};

    template<auto Field>
    struct AProperty : Property<typename FieldDeductor<Field>::Class, typename FieldDeductor<Field>::Type, Field> {};

}