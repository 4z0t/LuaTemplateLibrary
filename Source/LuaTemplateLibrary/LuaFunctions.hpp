#pragma once
#include "FuncArguments.hpp"
#include "FuncUtils.hpp"
#include "LuaAux.hpp"

namespace LTL
{
    /*
    template<auto fn, typename ...TUpvalues>
    struct CClosure
    {
        using FnType = decltype(fn);
        template<typename ...TArgs>
        using TReturn = typename std::invoke_result<FnType, TUpvalues&..., TArgs...>::type;

        template<typename ...TArgs>
        using ArgsTuple = std::tuple<TArgs...>;
        using Upvalues = std::tuple<TUpvalues...>;


    public:
        template<typename ...TArgs>
        static int Function(lua_State* l)
        {
            using namespace std;

            using Indexes = index_sequence_for<TArgs...>;
            static_assert(is_invocable<FnType, TUpvalues&..., TArgs...>::value, "Given function can't be called with such arguments!");
            using ArgsTupleT = ArgsTuple<TArgs...>;

            Upvalues upvalues;
            GetUpvalues<0, Upvalues, TUpvalues...>(l, upvalues);
            ArgsTupleT args;
            GetArgs<0, ArgsTupleT, TArgs ...>(l, args);
            if constexpr (is_void<TReturn<TArgs...>>::value)
            {
                CClosure::CallHelper(upvalues, args, index_sequence_for<TUpvalues...>{}, Indexes{});
                ReplaceUpvalues(l, upvalues);
                return 0;
            }
            else
            {
                TReturn<TArgs...> result = CClosure::CallHelper(upvalues, args, index_sequence_for<TUpvalues...>{}, Indexes{});
                ReplaceUpvalues(l, upvalues);
                size_t n_results = PushResult(l, result);
                return static_cast<int>(n_results);
            }
        }
    private:
        template <size_t ... UpIs, typename ...TArgs, size_t ... Is>
        inline static TReturn<TArgs...> CallHelper(Upvalues& upvalues, ArgsTuple<TArgs...>& args, std::index_sequence<UpIs...> const, std::index_sequence<Is...> const)
        {
            return fn(std::get<UpIs>(upvalues)..., std::get<Is>(args)...);
        }
    };

    template<auto fn>
    struct CFunction : public CClosure<fn> {
    };


    template<typename FnClass, typename ...TUpvalues>
    struct ClassClosure
    {
        template<typename ...TArgs>
        using TReturn = typename std::invoke_result<FnClass, TUpvalues&..., TArgs...>::type;

        template<typename ...TArgs>
        using ArgsTuple = std::tuple<TArgs...>;
        using Upvalues = std::tuple<TUpvalues...>;


    public:
        template<typename ...TArgs>
        static int Function(lua_State* l)
        {
            using namespace std;

            using Indexes = index_sequence_for<TArgs...>;
            static_assert(std::is_invocable<FnClass, TUpvalues&..., TArgs...>::value, "Given class doesnt provide callable or can't be called with such arguments!");
            using ArgsTupleT = ArgsTuple<TArgs...>;

            Upvalues upvalues;
            GetUpvalues<0, Upvalues, TUpvalues...>(l, upvalues);
            ArgsTupleT args;
            GetArgs<0, ArgsTupleT, TArgs ...>(l, args);
            if constexpr (is_void<TReturn<TArgs...>>::value)
            {
                ClassClosure::CallHelper(l, upvalues, args, index_sequence_for<TUpvalues...>{}, Indexes{});
                ReplaceUpvalues(l, upvalues);
                return 0;
            }
            else
            {
                TReturn<TArgs...> result = ClassClosure::CallHelper(l, upvalues, args, index_sequence_for<TUpvalues...>{}, Indexes{});
                ReplaceUpvalues(l, upvalues);
                size_t n_results = PushResult(l, result);
                return static_cast<int>(n_results);
            }
        }
    private:
        template <size_t ... UpIs, typename ...TArgs, size_t ... Is>
        inline static TReturn<TArgs...> CallHelper(lua_State* l, Upvalues& upvalues, ArgsTuple<TArgs...>& args, std::index_sequence<UpIs...> const, std::index_sequence<Is...> const)
        {

            if constexpr (std::is_constructible<FnClass, lua_State*>::value)
            {
                return FnClass{ l }(std::get<UpIs>(upvalues)..., std::get<Is>(args)...);
            }
            else
            {
                return FnClass{}(std::get<UpIs>(upvalues)..., std::get<Is>(args)...);
            }
        }
    };

    template<typename FnClass>
    struct ClassFunction : public ClassClosure<FnClass> {
    };
    */

    namespace Internal
    {
        struct CFunctionBase
        {
        };

        template <typename... TArgs>
        struct FunctionHelper : CFunctionBase
        {
            using ArgsTuple = std::tuple<Unwrap_t<TArgs>...>;
            static constexpr size_t GetArgs(lua_State* l, ArgsTuple& args)
            {
                return FuncUtility::GetArgs<ArgsTuple, TArgs...>(l, args);
            }

            static constexpr size_t ReplaceUpvalues(lua_State* l, ArgsTuple& args)
            {
                return FuncUtility::ReplaceUpvalues<ArgsTuple, TArgs...>(l, args);
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
        using TReturn = typename FunctionCaller::TReturn;
        using ArgsTuple = typename FunctionHelper::ArgsTuple;

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

        constexpr static size_t min_arg_count = FuncUtility::MinArgumentCount<TArgs...>();
        constexpr static size_t max_arg_count = FuncUtility::MaxArgumentCount<TArgs...>();

        static int Function(lua_State* l)
        {
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

        static int _Caller(lua_State* l)
        {
            ArgsTuple args;
            FunctionHelper::GetArgs(l, args);
            if constexpr (std::is_void_v<TReturn>)
            {
                FunctionCaller::Call(args);
                FunctionHelper::ReplaceUpvalues(l, args);
                return 0;
            }
            else
            {
                auto result = FunctionCaller::Call(args);
                FunctionHelper::ReplaceUpvalues(l, args);
                size_t n_results = PushResult(l, result);
                return static_cast<int>(n_results);
            }
        }
    };

    template <auto fn, typename TRet, typename... TArgs>
    struct CFunction<fn, TRet(TArgs...)> : Internal::FunctionHelper<TArgs...>, private Internal::FunctionCaller<fn, TArgs...>
    {
        using TUnwrappedReturn = typename FunctionCaller::TReturn;
        static_assert(std::is_void_v<TUnwrappedReturn> == std::is_void_v<TRet>, "If function returns void you cannot return anything else but void!");

        using ArgsTuple = typename FunctionHelper::ArgsTuple;

    public:
        template <typename... TUpvalues>
        struct ValidUpvalues : FuncUtility::MatchUpvalues<TUpvalues...>::template Matches<TArgs...>{};

        constexpr static size_t min_arg_count = FuncUtility::MinArgumentCount<TArgs...>();
        constexpr static size_t max_arg_count = FuncUtility::MaxArgumentCount<TArgs...>();

        static int Function(lua_State* l)
        {
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

        static int _Caller(lua_State* l)
        {
            ArgsTuple args;
            FunctionHelper::GetArgs(l, args);
            if constexpr (std::is_void_v<TUnwrappedReturn>)
            {
                FunctionCaller::Call(args);
                FunctionHelper::ReplaceUpvalues(l, args);
                return 0;
            }
            else
            {
                TUnwrappedReturn result = FunctionCaller::Call(args);
                FunctionHelper::ReplaceUpvalues(l, args);
                StackType<TRet>::Push(l, std::move(result));
                return 1;
            }
        }
    };
}