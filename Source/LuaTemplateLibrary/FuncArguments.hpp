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
            return StackType<T>::Check(l, index) || lua_isnoneornil(l, index);
        }
    };

    template<typename OptType, typename T>
    struct GetOptional
    {
        static constexpr T Get(lua_State* l, int index)
        {
            if (lua_isnoneornil(l, index))
                return OptType::value;
            return StackType<T>::Get(l, index);
        }
    };

    template<typename OptType>
    struct GetOptional<OptType, int> :Internal::OptionalInteger<OptType, int> {};
    template<typename OptType>
    struct GetOptional<OptType, unsigned int> :Internal::OptionalInteger<OptType, unsigned int> {};
    template<typename OptType>
    struct GetOptional<OptType, char> :Internal::OptionalInteger<OptType, char> {};
    template<typename OptType>
    struct GetOptional<OptType, unsigned char> :Internal::OptionalInteger<OptType, unsigned char> {};
    template<typename OptType>
    struct GetOptional<OptType, short> :Internal::OptionalInteger<OptType, short> {};
    template<typename OptType>
    struct GetOptional<OptType, unsigned short> :Internal::OptionalInteger<OptType, unsigned short> {};
    template<typename OptType>
    struct GetOptional<OptType, long long> :Internal::OptionalInteger<OptType, long long> {};
    template<typename OptType>
    struct GetOptional<OptType, unsigned long long> :Internal::OptionalInteger<OptType, unsigned long long> {};

    template<typename OptType>
    struct GetOptional<OptType, float> :Internal::OptionalFloat<OptType, float> {};
    template<typename OptType>
    struct GetOptional<OptType, double> :Internal::OptionalFloat<OptType, double> {};
    template<typename OptType>
    struct GetOptional<OptType, long double> :Internal::OptionalFloat<OptType, long double> {};

    template<typename T>
    struct StackType<T, EnableIfOptionalArg<T>> : CheckOptional<Unwrap_t<T>>, GetOptional<T, Unwrap_t<T>> {};
}

#define LuaOptionalArg(name_, type_, value_) \
struct name_ : Lua::OptionalBase<type_>\
{\
  static const type_ value; \
};\
const type_ name_::value = (value_)