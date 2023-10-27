#pragma once
#include "LuaAux.hpp"
#include "LuaTypes.hpp"
#include "FuncArguments.hpp"

namespace Lua
{
    namespace FuncUtility
    {
        template<typename T>
        struct IsUpvalueType : std::false_type {};

        template<typename T>
        struct IsUpvalueType<Upvalue<T>> : std::true_type {};

        template<size_t Value>
        using ValueContainer = std::integral_constant<size_t, Value>;

        template<bool Increment, size_t Value>
        struct Incrementer;

        template<size_t Value>
        struct Incrementer<true, Value> : ValueContainer<Value + 1> {};

        template<size_t Value>
        struct Incrementer<false, Value> : ValueContainer<Value> {};

        template<typename T, size_t Index>
        struct IncrementArgIndex : Incrementer<!IsUpvalueType<T>::value, Index> {};

        template<typename T, size_t Index>
        struct IncrementUpvalueIndex : Incrementer<IsUpvalueType<T>::value, Index> {};

        template<typename T, typename Optional = void>
        struct ArgExtractor
        {
            template<size_t ArgI, size_t UpvalueI>
            static constexpr typename StackType<T>::TReturn Get(lua_State* l)
            {
                return StackType<T>::Get(l, ArgI + 1);
            }
        };

        template<typename T>
        struct ArgExtractor<Default<T>>
        {
            template<size_t ArgI, size_t UpvalueI>
            static 	constexpr typename StackType<T>::TReturn Get(lua_State* l)
            {
                if (StackType<T>::Check(l, ArgI + 1))
                    return StackType<T>::Get(l, ArgI + 1);
                return Default<T>::value;
            }
        };

        template<typename T>
        struct ArgExtractor<Upvalue<T>>
        {
            template<size_t ArgI, size_t UpvalueI>
            static constexpr typename StackType<T>::TReturn Get(lua_State* l)
            {
                return StackType<T>::Get(l, lua_upvalueindex(static_cast<int>(UpvalueI) + 1));
            }
        };


        template<typename T>
        struct ArgExtractor<T, std::enable_if_t<std::is_base_of_v<OptionalArg, T>>>
        {
            using ReturnT = typename T::type;
            template<size_t ArgI, size_t UpvalueI>
            static constexpr ReturnT Get(lua_State* l)
            {
                if (StackType<ReturnT>::Check(l, ArgI + 1))
                    return StackType<ReturnT>::Get(l, ArgI + 1);
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
                    StackType<T>::Push(l, value);
                    lua_replace(l, lua_upvalueindex(static_cast<int>(UpvalueI) + 1));
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
                IncrementArgIndex<TArg, ArgIndex>::value,
                IncrementUpvalueIndex<TArg, UpvalueIndex>::value,
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
                IncrementArgIndex<T, ArgIndex>::value,
                IncrementUpvalueIndex<T, UpvalueIndex>::value,
                TArgsTuple, Ts...>(l, args);
        }

        template<typename TArgsTuple, typename ...Ts>
        constexpr size_t ReplaceUpvalues(lua_State* l, TArgsTuple& args)
        {
            return ReplaceUpvalues<0, 0, TArgsTuple, Ts...>(l, args);
        }

    }
}