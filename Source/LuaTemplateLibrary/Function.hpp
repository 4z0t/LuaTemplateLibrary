#pragma once
#include "FuncArguments.hpp"
#include "FuncUtils.hpp"
#include "LuaAux.hpp"

namespace LTL
{
    namespace Internal
    {
        struct CFunctionBase
        {
        };

        template <typename... TArgs>
        struct FunctionHelper : CFunctionBase
        {
            using ArgsTuple = std::tuple<Unwrap_t<TArgs>...>;
        private:
            template <size_t... Is>
            inline static ArgsTuple UnpackArgs(lua_State* l, const std::index_sequence<Is...>)
            {
                LTL::FuncUtility::Args<0, 0, TArgs...> args;
                return { args.Get<Is>(l)... };
            }

        public:
            constexpr static size_t min_arg_count = FuncUtility::MinArgumentCount<TArgs...>();
            constexpr static size_t max_arg_count = FuncUtility::MaxArgumentCount<TArgs...>();

            static constexpr inline ArgsTuple GetArgs(lua_State* l)
            {
                return UnpackArgs(l, std::index_sequence_for<TArgs...>{});
            }

            static constexpr inline size_t ReplaceUpvalues(lua_State* l, ArgsTuple& args)
            {
                return FuncUtility::ReplaceUpvalues<ArgsTuple, TArgs...>(l, args);
            }

            static constexpr inline void  CheckArgCount(lua_State* l)
            {
                const size_t n = static_cast<size_t>(lua_gettop(l));
                if constexpr (min_arg_count == max_arg_count)
                {
                    if (max_arg_count != n)
                    {
                        luaL_error(l, "Expected %d arguments, but got %d.", max_arg_count, n);
                    }
                }
                else
                {
                    if (n < min_arg_count && n > max_arg_count)
                    {
                        luaL_error(l, "Expected arguments between %d and %d, but got %d.", min_arg_count, max_arg_count, n);
                    }
                }
            }
        };

        template <auto fn, typename... TArgs>
        struct FunctionCaller
        {
            using FnType = decltype(fn);
            using ArgsTuple = std::tuple<Unwrap_t<TArgs>...>;
            static_assert(std::is_invocable_v<FnType, Unwrap_t<TArgs> &...>, "Given function can't be called with such arguments!");
            using TReturn = std::invoke_result_t<FnType, Unwrap_t<TArgs> &...>;

            template <typename R, typename D = R>
            struct CallHelper;

            template <typename R, typename... Ts>
            struct CallHelper<R(*)(Ts...)>
            {
                template <typename... Args>
                static TReturn Call(Args &...args)
                {
                    return fn(args...);
                }
            };

            template <typename R, typename... Ts>
            struct CallHelper<R(*)(Ts...) noexcept>
            {
                template <typename... Args>
                static TReturn Call(Args &...args)
                {
                    return fn(args...);
                }
            };

            template <typename R, class C, typename... Ts>
            struct CallHelper<R(C::*)(Ts...) const noexcept>
            {
                template <class Class, typename... Args>
                static TReturn Call(const Class& arg, Args &...args)
                {
                    if constexpr (std::is_base_of_v<UserDataValueBase, Class>)
                    {
                        return ((*arg).*fn)(args...);
                    }
                    else if constexpr (std::is_pointer_v<Class>)
                    {
                        return (arg->*fn)(args...);
                    }
                    else
                    {
                        return (arg.*fn)(args...);
                    }
                }
            };

            template <typename R, class C, typename... Ts>
            struct CallHelper<R(C::*)(Ts...) noexcept>
            {
                template <class Class, typename... Args>
                static TReturn Call(Class& arg, Args &...args)
                {
                    if constexpr (std::is_base_of_v<UserDataValueBase, Class>)
                    {
                        return ((*arg).*fn)(args...);
                    }
                    else if constexpr (std::is_pointer_v<Class>)
                    {
                        return (arg->*fn)(args...);
                    }
                    else
                    {
                        return (arg.*fn)(args...);
                    }
                }
            };

            template <typename R, class C, typename... Ts>
            struct CallHelper<R(C::*)(Ts...)>
            {
                template <class Class, typename... Args>
                static TReturn Call(Class& arg, Args &...args)
                {
                    if constexpr (std::is_base_of_v<UserDataValueBase, Class>)
                    {
                        return ((*arg).*fn)(args...);
                    }
                    else if constexpr (std::is_pointer_v<Class>)
                    {
                        return (arg->*fn)(args...);
                    }
                    else
                    {
                        return (arg.*fn)(args...);
                    }
                }
            };

            template <typename R, class C, typename... Ts>
            struct CallHelper<R(C::*)(Ts...) const>
            {
                template <class Class, typename... Args>
                static TReturn Call(const Class& arg, Args &...args)
                {
                    if constexpr (std::is_base_of_v<UserDataValueBase, Class>)
                    {
                        return ((*arg).*fn)(args...);
                    }
                    else if constexpr (std::is_pointer_v<Class>)
                    {
                        return (arg->*fn)(args...);
                    }
                    else
                    {
                        return (arg.*fn)(args...);
                    }
                }
            };

            template <size_t... Is>
            inline static TReturn Call(ArgsTuple& args)
            {
                return UnpackArgs(args, std::index_sequence_for<TArgs...>{});
            }

            template <size_t... Is>
            inline static TReturn UnpackArgs(ArgsTuple& args, const std::index_sequence<Is...>)
            {
                return CallHelper<FnType>::Call(std::get<Is>(args)...);
            }
        };
    }

    /**
     * @brief Класс для преобразования функции в универсальную функцию Lua.
     *
     * @tparam fn Функция.
     * @tparam TArgs Типы аргументов.
     * @see Default<T>
     * @see Optional<T>
     * @see Upvalue<T>
     */
    template <auto fn, typename... TArgs>
    struct CFunction : Internal::FunctionHelper<TArgs...>, private Internal::FunctionCaller<fn, TArgs...>
    {
        using _FunctionHelper = Internal::FunctionHelper<TArgs...>;
        using _FunctionCaller = Internal::FunctionCaller<fn, TArgs...>;
        using TReturn = typename _FunctionCaller::TReturn;
        using ArgsTuple = typename _FunctionHelper::ArgsTuple;

    public:
        CFunction() = default;

        /**
         * @brief Проверяет типы на соответствие Upvalue типам аргументов
         *
         * @tparam TUpvalues
         */
        template <typename... TUpvalues>
        struct ValidUpvalues : FuncUtility::MatchUpvalues<TUpvalues...>::template Matches<TArgs...>
        {
        };

        static int Function(lua_State* l)
        {
            _FunctionHelper::CheckArgCount(l);
            if constexpr (!FuncUtility::CanThrow<decltype(fn)>::value)
            {
                return _Caller(l);
            }
            else
            {
                try
                {
                    return _Caller(l);
                }
                catch (std::exception& ex)
                {
                    luaL_error(l, "%s", ex.what());
                }
                catch (LTL::Exception&)
                {
                    throw;
                }
                catch (...)
                {
                    luaL_error(l, "%s", "unknown error");
                }
            }
            return 0;
        }

        inline static int _Caller(lua_State* l)
        {

            ArgsTuple args = _FunctionHelper::GetArgs(l);
            if constexpr (std::is_void_v<TReturn>)
            {
                _FunctionCaller::Call(args);
                _FunctionHelper::ReplaceUpvalues(l, args);
                return 0;
            }
            else
            {
                auto result = _FunctionCaller::Call(args);
                _FunctionHelper::ReplaceUpvalues(l, args);
                size_t n_results = PushResult(l, result);
                return static_cast<int>(n_results);
            }
        }
    };

    template <auto fn, typename TRet, typename... TArgs>
    struct CFunction<fn, TRet(TArgs...)> : Internal::FunctionHelper<TArgs...>, private Internal::FunctionCaller<fn, TArgs...>
    {
        using _FunctionHelper = Internal::FunctionHelper<TArgs...>;
        using _FunctionCaller = Internal::FunctionCaller<fn, TArgs...>;
        using TUnwrappedReturn = typename _FunctionCaller::TReturn;
        static_assert(std::is_void_v<TUnwrappedReturn> == std::is_void_v<TRet>, "If function returns void you cannot return anything else but void!");

        using ArgsTuple = typename _FunctionHelper::ArgsTuple;

    public:
        template <typename... TUpvalues>
        struct ValidUpvalues : FuncUtility::MatchUpvalues<TUpvalues...>::template Matches<TArgs...>{};

        constexpr static size_t min_arg_count = FuncUtility::MinArgumentCount<TArgs...>();
        constexpr static size_t max_arg_count = FuncUtility::MaxArgumentCount<TArgs...>();

        static int Function(lua_State* l)
        {
            _FunctionHelper::CheckArgCount(l);
            if constexpr (!FuncUtility::CanThrow<decltype(fn)>::value)
            {
                return _Caller(l);
            }
            else
            {
                try
                {
                    return _Caller(l);
                }
                catch (std::exception& ex)
                {
                    luaL_error(l, "%s", ex.what());
                }
                catch (LTL::Exception&)
                {
                    throw;
                }
                catch (...)
                {
                    luaL_error(l, "%s", "unknown error");
                }
            }
            return 0;
        }

        inline static int _Caller(lua_State* l)
        {
            ArgsTuple args = _FunctionHelper::GetArgs(l);
            if constexpr (std::is_void_v<TUnwrappedReturn>)
            {
                _FunctionCaller::Call(args);
                _FunctionHelper::ReplaceUpvalues(l, args);
                return 0;
            }
            else
            {
                TUnwrappedReturn result = _FunctionCaller::Call(args);
                _FunctionHelper::ReplaceUpvalues(l, args);
                return PushResult<TUnwrappedReturn, TRet>(l, result);
            }
        }
    };
}