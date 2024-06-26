#pragma once
#include "FuncArguments.hpp"
#include "LuaAux.hpp"
#include "Types.hpp"

namespace LTL
{
    namespace FuncUtility
    {

        template <typename T>
        struct NoIncrement : std::false_type
        {
        };

        template <>
        struct NoIncrement<CState*> : std::true_type
        {
        };

        template <>
        struct NoIncrement<lua_State*> : std::true_type
        {
        };

        template <size_t Value>
        using ValueContainer = std::integral_constant<size_t, Value>;

        template <bool Increment, size_t Value>
        struct Incrementer;

        template <size_t Value>
        struct Incrementer<true, Value> : ValueContainer<Value + 1>
        {
        };

        template <size_t Value>
        struct Incrementer<false, Value> : ValueContainer<Value>
        {
        };

        template <typename T, size_t Index>
        struct IncrementArgIndex : Incrementer<!IsUpvalueType<T>::value && !NoIncrement<T>::value, Index>
        {
        };

        template <typename T, size_t Index>
        struct IncrementUpvalueIndex : Incrementer<IsUpvalueType<T>::value && !NoIncrement<T>::value, Index>
        {
        };

        template <typename T, typename IsOptional = void>
        struct IsOptionalArgumentType : std::false_type
        {
        };

        template <typename T>
        struct IsOptionalArgumentType<T, EnableIfOptionalArg<T>> : std::true_type
        {
        };

        template <typename T>
        struct ArgExtractor
        {
            template <size_t ArgI, size_t UpvalueI>
            static constexpr Unwrap_t<T> Get(lua_State* l)
            {
                return StackType<T>::Get(l, ArgI + 1);
            }

            template <size_t ArgI, size_t UpvalueI>
            static constexpr bool Check(lua_State* l)
            {
                return StackType<T>::Check(l, ArgI + 1);
            }
        };

        template <typename T>
        struct ArgExtractor<Upvalue<T>>
        {
            template <size_t ArgI, size_t UpvalueI>
            static constexpr Unwrap_t<T> Get(lua_State* l)
            {
                return StackType<T>::Get(l, lua_upvalueindex(static_cast<int>(UpvalueI) + 1));
            }
        };

#pragma region GetArgs logic
        template <size_t TupleIndex, size_t ArgIndex, size_t UpvalueIndex, typename TArgsTuple>
        constexpr size_t GetArgs(lua_State* l, TArgsTuple& args)
        {
            return TupleIndex;
        }

        template <size_t TupleIndex, size_t ArgIndex, size_t UpvalueIndex, typename TArgsTuple, typename TArg, typename... TArgs>
        constexpr size_t GetArgs(lua_State* l, TArgsTuple& args)
        {
            std::get<TupleIndex>(args) = ArgExtractor<TArg>::Get<ArgIndex, UpvalueIndex>(l);
            return GetArgs<
                TupleIndex + 1,
                IncrementArgIndex<TArg, ArgIndex>::value,
                IncrementUpvalueIndex<TArg, UpvalueIndex>::value,
                TArgsTuple, TArgs...>(l, args);
        }

        template <typename TArgsTuple, typename... TArgs>
        constexpr size_t GetArgs(lua_State* l, TArgsTuple& args)
        {
            return GetArgs<0, 0, 0, TArgsTuple, TArgs...>(l, args);
        }
#pragma endregion

#pragma region ReplaceUpvalues logic
        template <size_t TupleIndex, size_t ArgIndex, size_t UpvalueIndex, typename TArgsTuple>
        constexpr size_t ReplaceUpvalues(lua_State* l, TArgsTuple& args)
        {
            return TupleIndex;
        }

        template <size_t TupleIndex, size_t ArgIndex, size_t UpvalueIndex, typename TArgsTuple, typename T, typename... Ts>
        constexpr size_t ReplaceUpvalues(lua_State* l, TArgsTuple& args)
        {
            if constexpr (IsUpvalueType<T>::value)
            {
                if constexpr (!std::is_pointer_v<T>)
                {
                    PushValue(l, std::get<TupleIndex>(args));
                    lua_replace(l, lua_upvalueindex(static_cast<int>(UpvalueIndex) + 1));
                }
            }
            return ReplaceUpvalues<
                TupleIndex + 1,
                IncrementArgIndex<T, ArgIndex>::value,
                IncrementUpvalueIndex<T, UpvalueIndex>::value,
                TArgsTuple, Ts...>(l, args);
        }

        template <typename TArgsTuple, typename... Ts>
        constexpr size_t ReplaceUpvalues(lua_State* l, TArgsTuple& args)
        {
            return ReplaceUpvalues<0, 0, 0, TArgsTuple, Ts...>(l, args);
        }
#pragma endregion

#pragma region CanThrow types

        template <typename... TArgs>
        struct CanThrow;

        template <typename R, typename... Ts>
        struct CanThrow<R(*)(Ts...)> : std::true_type
        {
        };

        template <typename R, typename... Ts>
        struct CanThrow<R(*)(Ts...) noexcept> : std::false_type
        {
        };

        template <typename R, class C, typename... Ts>
        struct CanThrow<R(C::*)(Ts...) const> : std::true_type
        {
        };

        template <typename R, class C, typename... Ts>
        struct CanThrow<R(C::*)(Ts...) const noexcept> : std::false_type
        {
        };

        template <typename R, class C, typename... Ts>
        struct CanThrow<R(C::*)(Ts...)> : std::true_type
        {
        };

        template <typename R, class C, typename... Ts>
        struct CanThrow<R(C::*)(Ts...) noexcept> : std::false_type
        {
        };
#pragma endregion

#pragma region Deduce class from method
        template<typename T>
        struct _Class {
            using type = typename T;
        };

        template <typename... TArgs>
        struct DeduceClass;

        template <typename R, class C, typename... Ts>
        struct DeduceClass<R(*)(C, Ts...)> : _Class<std::decay_t<std::remove_pointer_t<C>>>
        {
        };

        template <typename R, class C, typename... Ts>
        struct DeduceClass<R(*)(C, Ts...) noexcept> : _Class<std::decay_t<std::remove_pointer_t<C>>>
        {
        };

        template <typename R, class C, typename... Ts>
        struct DeduceClass<R(C::*)(Ts...) const> : _Class<C>
        {
        };

        template <typename R, class C, typename... Ts>
        struct DeduceClass<R(C::*)(Ts...) const noexcept> : _Class<C>
        {
        };

        template <typename R, class C, typename... Ts>
        struct DeduceClass<R(C::*)(Ts...)> : _Class<C>
        {
        };

        template <typename R, class C, typename... Ts>
        struct DeduceClass<R(C::*)(Ts...) noexcept> : _Class<C>
        {
        };
#pragma endregion


#pragma region ArgumentTypeMatching
        template <typename... Ts>
        struct MatchUpvalues;

        template <typename TUpvalue, typename... TUpvalues>
        struct MatchUpvalues<TUpvalue, TUpvalues...>
        {
            template <typename... TArgs>
            struct Matches;

            template <typename TArg, typename... TArgs>
            struct Matches<TArg, TArgs...>
            {
                constexpr static bool value = IsUpvalueType<TArg>::value ? (std::is_same_v<Unwrap_t<TArg>, TUpvalue> &&
                    MatchUpvalues<TUpvalues...>::template Matches<TArgs...>::value)
                    : MatchUpvalues<TUpvalue, TUpvalues...>::template Matches<TArgs...>::value;
            };

            template <>
            struct Matches<>
            {
                constexpr static bool value = false;
            };
        };

        template <>
        struct MatchUpvalues<>
        {
            template <typename... TArgs>
            struct Matches;

            template <typename TArg, typename... TArgs>
            struct Matches<TArg, TArgs...>
            {
                constexpr static bool value = IsUpvalueType<TArg>::value ? false : MatchUpvalues<>::Matches<TArgs...>::value;
            };

            template <>
            struct Matches<>
            {
                constexpr static bool value = true;
            };
        };

        template <typename... TArgs>
        struct HasUpvalues;

        template <>
        struct HasUpvalues<> : std::false_type
        {
        };

        template <typename TArg, typename... TArgs>
        struct HasUpvalues<TArg, TArgs...> : std::bool_constant<IsUpvalueType<TArg>::value || HasUpvalues<TArgs...>::value>
        {
        };

        template <size_t ArgIndex, size_t UpvalueIndex>
        constexpr bool MatchesTypes(lua_State* l)
        {
            return true;
        }

        template <size_t ArgIndex, size_t UpvalueIndex, typename T, typename... Ts>
        constexpr bool MatchesTypes(lua_State* l)
        {
            if constexpr (!IsUpvalueType<T>::value)
            {
                if (!ArgExtractor<T>::Check<ArgIndex, UpvalueIndex>(l))
                {
                    return false;
                }
            }
            return MatchesTypes<
                IncrementArgIndex<T, ArgIndex>::value,
                IncrementUpvalueIndex<T, UpvalueIndex>::value,
                Ts...>(l);
        }

        template <typename... Ts>
        constexpr bool MatchesTypes(lua_State* l)
        {
            return MatchesTypes<0, 0, Ts...>(l);
        }

        struct MinArgumentCountResult
        {
            size_t n;
            bool ret;
        };

        template <size_t ArgIndex>
        constexpr MinArgumentCountResult MinArgumentCount()
        {
            return { ArgIndex, false };
        }

        template <size_t ArgIndex, typename T, typename... Ts>
        constexpr MinArgumentCountResult MinArgumentCount()
        {
            constexpr auto r = MinArgumentCount<IncrementArgIndex<T, ArgIndex>::value, Ts...>();
            if constexpr (r.ret)
            {
                return r;
            }
            else if constexpr (IsUpvalueType<T>::value || NoIncrement<T>::value || IsOptionalArgumentType<T>::value)
            {
                return { ArgIndex, false };
            }
            else
            {
                return { r.n, true };
            }
        }


        template <size_t ArgIndex>
        constexpr size_t MaxArgumentCount()
        {
            return ArgIndex;
        }

        template <size_t ArgIndex, typename T, typename... Ts>
        constexpr size_t MaxArgumentCount()
        {
            return MaxArgumentCount<IncrementArgIndex<T, ArgIndex>::value, Ts...>();
        }

        template <typename... Ts>
        constexpr size_t MinArgumentCount()
        {
            return MinArgumentCount<0, Ts...>().n;
        }
        template <typename... Ts>
        constexpr size_t MaxArgumentCount()
        {
            return MaxArgumentCount<0, Ts...>();
        }
#pragma endregion

    }
}