#pragma once

#include <type_traits>
#include <iostream>
#include <vector>
#include "LuaTypes.hpp"

namespace LTL
{

    struct UnwrapTypeBase {};

    template<typename T>
    struct TypeBase :UnwrapTypeBase
    {
        using type = T;
    };

    struct OptionalArg {};

    template<typename T>
    struct OptionalBase :OptionalArg, TypeBase<T> {};

    template<typename T, typename DerivedFromSuper = void>
    struct Unwrap
    {
        using type = T;
    };

    template<typename T>
    using EnableIfUnwrappable = std::enable_if_t<std::is_base_of_v<UnwrapTypeBase, T>>;

    template<typename T>
    using EnableIfOptionalArg = std::enable_if_t<std::is_base_of_v<OptionalArg, T>>;

    template<typename T>
    struct Unwrap<T, EnableIfUnwrappable<T>>
    {
        using type = typename T::type;
    };

    template<typename T, typename DerivedFromSuper = void>
    using Unwrap_t = typename Unwrap<T, DerivedFromSuper>::type;

    template<typename T>
    struct  Default : OptionalBase<T>
    {
        static  T value;
    };

    template<typename T>
    T Default<T>::value{};

    template<typename T>
    struct IsUpvalueType;

    template<typename T>
    struct Upvalue final :TypeBase<T>
    {
        static_assert(!IsUpvalueType<T>::value, "T can't be an Upvalue");
    };

    template<typename T>
    struct IsUpvalueType : std::false_type {};

    template<typename T>
    struct IsUpvalueType<Upvalue<T>> : std::true_type {};

    template<typename T>
    struct CheckOptional
    {
        static constexpr bool Check(lua_State* l, int index)
        {
            return lua_isnoneornil(l, index) || StackType<T>::Check(l, index);
        }
    };

    template<typename TOpt>
    struct StackType<TOpt, EnableIfOptionalArg<TOpt>> : CheckOptional<Unwrap_t<TOpt>>
    {
        using T = Unwrap_t<TOpt>;

        static constexpr T Get(lua_State* l, int index)
        {
            if (lua_isnoneornil(l, index))
                return TOpt::value;
            return StackType<T>::Get(l, index);
        }
    };

    struct MultReturnBase {};

    template<typename ...Ts>
    struct MultReturn : std::tuple<Ts...>, MultReturnBase
    {
        using std::tuple<Ts...>::tuple;

        template <size_t N>
        decltype(auto) Get()
        {
            return std::get<N>(*this);
        }

        template <size_t N>
        decltype(auto) Get()const
        {
            return std::get<N>(*this);
        }
    };

    struct StackResult
    {
        const size_t n_results = 1;

        StackResult(size_t n) : n_results(n) { }
    };
}

#define LuaOptionalArg(name_, type_, value_) \
struct name_ : LTL::OptionalBase<type_>\
{\
  static const type_ value; \
};\
const type_ name_::value = (value_)