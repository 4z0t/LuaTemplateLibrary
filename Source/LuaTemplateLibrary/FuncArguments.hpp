#pragma once

#include <type_traits>
#include <iostream>
#include <vector>

namespace Lua
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
    struct Unwrap<T, std::enable_if_t<std::is_base_of_v<UnwrapTypeBase, T>>>
    {
        using type = typename T::type;
    };

    template<typename T, typename DerivedFromSuper = void>
    using Unwrap_t = typename Unwrap<T, DerivedFromSuper>::type;

    template<typename T>
    struct  Default : TypeBase<T>
    {
        static  T value;
    };

    template<typename T>
    T Default<T>::value{};

    template<typename T>
    struct Upvalue :TypeBase<T> {};
}

#define LuaOptionalArg(name_, type_, value_) \
struct name_ : Lua::OptionalBase<type_>\
{\
  static const type_ value; \
};\
const type_ name_::value = (value_)