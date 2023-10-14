#pragma once
#include "LuaAux.hpp"
#include "LuaTypes.hpp"
#include "FuncArguments.hpp"

namespace Lua
{
    namespace FuncUtility
    {
        template<typename T, T Value>
        using ValueContainer = std::integral_constant<T, Value>;

        template<typename T>
        struct IncrementArgIndex : ValueContainer<size_t, 1> {};

        template<typename T>
        struct IncrementArgIndex<Upvalue<T>> : ValueContainer<size_t, 0> {};

        template<typename T>
        struct IncrementUpvalueIndex : ValueContainer<size_t, 0> {};

        template<typename T>
        struct IncrementUpvalueIndex<Upvalue<T>> : ValueContainer<size_t, 1> {};

        template<typename T>
        struct IsUpvalueType : std::bool_constant<IncrementUpvalueIndex<T>::value == 1> {};

        template<typename T, typename Optional = void>
        struct ArgExtractor
        {
            template<size_t ArgI, size_t UpvalueI>
            static 	constexpr T Get(lua_State* l)
            {
                return TypeParser<T>::Get(l, ArgI + 1);
            }
        };

        template<typename T>
        struct ArgExtractor<Default<T>>
        {
            template<size_t ArgI, size_t UpvalueI>
            static 	constexpr T Get(lua_State* l)
            {
                if (TypeParser<T>::Check(l, ArgI + 1))
                    return TypeParser<T>::Get(l, ArgI + 1);
                return Default<T>::value;
            }
        };

        template<typename T>
        struct ArgExtractor<Upvalue<T>>
        {
            template<size_t ArgI, size_t UpvalueI>
            static constexpr T Get(lua_State* l)
            {
                return TypeParser<T>::Get(l, lua_upvalueindex((int)UpvalueI + 1));
            }
        };


        template<typename T>
        struct ArgExtractor<T, std::enable_if_t<std::is_base_of_v<OptionalArg, T>>>
        {
            using ReturnT = typename T::type;
            template<size_t ArgI, size_t UpvalueI>
            static constexpr ReturnT Get(lua_State* l)
            {
                if (TypeParser<ReturnT>::Check(l, ArgI + 1))
                    return TypeParser<ReturnT>::Get(l, ArgI + 1);
                return T::value;
            }
        };

        template<typename T>
        struct UpvalueReplacer
        {
            template<size_t UpvalueI>
            static  constexpr void Replace(lua_State* l, const T& value)
            {
                if constexpr (!std::is_pointer_v<T>)
                {
                    TypeParser<T>::Push(l, value);
                    lua_replace(l, lua_upvalueindex((int)UpvalueI + 1));
                }
            }
        };
        template<size_t ArgIndex, size_t UpvalueIndex, typename TArgsTuple>
        constexpr size_t GetArgs(lua_State* l, TArgsTuple& args)
        {
            return ArgIndex + UpvalueIndex;
        }

        template<size_t ArgIndex, size_t UpvalueIndex, typename TArgsTuple, typename TArg, typename ...TArgs>
        constexpr size_t GetArgs(lua_State* l, TArgsTuple& args)
        {
            std::get<ArgIndex + UpvalueIndex>(args) = ArgExtractor<TArg>::Get<ArgIndex, UpvalueIndex>(l);
            return GetArgs<
                ArgIndex + IncrementArgIndex<TArg>::value,
                UpvalueIndex + IncrementUpvalueIndex<TArg>::value,
                TArgsTuple, TArgs...>(l, args);
        }

        template<typename TArgsTuple, typename ...TArgs>
        constexpr size_t GetArgs(lua_State* l, TArgsTuple& args)
        {
            return GetArgs<0, 0, TArgsTuple, TArgs...>(l, args);
        }

        template<size_t ArgIndex, size_t UpvalueIndex, typename TArgsTuple>
        constexpr size_t ReplaceUpvalues(lua_State* l, TArgsTuple& args)
        {
            return ArgIndex + UpvalueIndex;
        }

        template<size_t ArgIndex, size_t UpvalueIndex, typename TArgsTuple, typename T, typename ...Ts>
        constexpr size_t ReplaceUpvalues(lua_State* l, TArgsTuple& args)
        {
            if constexpr (IsUpvalueType<T>::value)
                UpvalueReplacer<typename T::type>::Replace<UpvalueIndex>(l, std::get<ArgIndex + UpvalueIndex>(args));
            return ReplaceUpvalues<
                ArgIndex + IncrementArgIndex<T>::value,
                UpvalueIndex + IncrementUpvalueIndex<T>::value,
                TArgsTuple, Ts...>(l, args);
        }

        template<typename TArgsTuple, typename ...Ts>
        constexpr size_t ReplaceUpvalues(lua_State* l, TArgsTuple& args)
        {
            return ReplaceUpvalues<0, 0, TArgsTuple, Ts...>(l, args);
        }

    }
}