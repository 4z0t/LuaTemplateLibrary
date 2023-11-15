#pragma once

#include <type_traits>
#include <iostream>
#include <vector>
#include "LuaTypes.hpp"

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
        static_assert(!IsUpvalueType<T>::value, "Upvalue can't be an Upvalue of Upvalue type");
    };

    template<typename T>
    struct IsUpvalueType : std::false_type { using type = void; };

    template<typename T>
    struct IsUpvalueType<Upvalue<T>> : std::true_type { using type = typename T; };

    template<typename T>
    struct StackType<T, EnableIfOptionalArg<T>>
    {
        using TReturn = Unwrap_t<T>;

        static constexpr TReturn Get(lua_State* l, int index)
        {
            if (StackType<TReturn>::Check(l, index))
                return StackType<TReturn>::Get(l, index);
            return T::value;
        }

        static constexpr bool Check(lua_State* l, int index)
        {
            return StackType<TReturn>::Check(l, index) || lua_isnoneornil(l, index);
        }
    };
}

#define LuaOptionalArg(name_, type_, value_) \
struct name_ : Lua::OptionalBase<type_>\
{\
  static const type_ value; \
};\
const type_ name_::value = (value_)