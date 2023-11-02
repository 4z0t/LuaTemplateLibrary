#pragma once
#include "LuaAux.hpp"
#include "FuncArguments.hpp"
#include "FuncUtils.hpp"

namespace Lua
{
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

    template<auto fn, typename ...TArgs>
    struct Closure
    {
        using FnType = decltype(fn);

        static_assert(std::is_invocable_v<FnType, Unwrap_t<TArgs>&...>, "Given function can't be called with such arguments!");
        using TReturn = std::invoke_result_t<FnType, Unwrap_t<TArgs>&...>;

        using ArgsTuple = std::tuple<Unwrap_t<TArgs>...>;
        using Indexes = std::index_sequence_for<TArgs...>;

    public:
        static int Function(lua_State* l)
        {
            ArgsTuple args;
            Closure::GetArgs(l, args);
            if constexpr (std::is_void_v<TReturn>)
            {
                Closure::Call(args, Indexes{});
                Closure::ReplaceUpvalues(l, args);
                return 0;
            }
            else
            {
                TReturn result = Closure::Call(args, Indexes{});
                Closure::ReplaceUpvalues(l, args);
                size_t n_results = PushResult(l, result);
                return static_cast<int>(n_results);
            }
        }
    private:
        static constexpr size_t GetArgs(lua_State* l, ArgsTuple& args)
        {
            return FuncUtility::GetArgs<0, 0, ArgsTuple, TArgs...>(l, args);
        }

        static constexpr size_t ReplaceUpvalues(lua_State* l, ArgsTuple& args)
        {
            return FuncUtility::ReplaceUpvalues<0, 0, ArgsTuple, TArgs...>(l, args);
        }

        template<typename R, typename D = R>
        struct CallHelper;

        template<typename R, typename ...Ts>
        struct CallHelper<R(*)(Ts...)>
        {
            template<typename ...Args>
            static TReturn Call(Args&... args)
            {
                return fn(args...);
            }
        };

        template<typename R, class C, typename ...Ts>
        struct CallHelper<R(C::*)(Ts...)>
        {
            template<class Class, typename ...Args>
            static TReturn Call(Class& arg, Args&... args)
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

        template<typename R, class C, typename ...Ts>
        struct CallHelper<R(C::*)(Ts...)const>
        {
            template<class Class, typename ...Args>
            static TReturn Call(const Class& arg, Args&... args)
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

        template <size_t ... Is>
        inline static TReturn Call(ArgsTuple& args, const std::index_sequence<Is...>)
        {
            return CallHelper<FnType>::Call(std::get<Is>(args)...);
        }
    };


    template<auto fn, typename TReturn, typename ...TArgs>
    struct Closure<fn, TReturn(TArgs...)>
    {
        using FnType = decltype(fn);

        static_assert(std::is_invocable_v<FnType, Unwrap_t<TArgs>&...>, "Given function can't be called with such arguments!");
        using UnwrapperReturn = std::invoke_result_t<FnType, Unwrap_t<TArgs>&...>;

        static_assert(std::is_same_v<UnwrapperReturn, void> == std::is_same_v<TReturn, void>, "If function returns void you cannot return anything else but void!");

        using ArgsTuple = std::tuple<Unwrap_t<TArgs>...>;
        using Indexes = std::index_sequence_for<TArgs...>;

    public:
        static int Function(lua_State* l)
        {
            ArgsTuple args;
            Closure::GetArgs(l, args);
            if constexpr (std::is_void_v<UnwrapperReturn>)
            {
                Closure::Call(args, Indexes{});
                Closure::ReplaceUpvalues(l, args);
                return 0;
            }
            else
            {
                UnwrapperReturn result = Closure::Call(args, Indexes{});
                Closure::ReplaceUpvalues(l, args);
                StackType<TReturn>::Push(l, std::move(result));
                return 1;
            }
        }
    private:
        static constexpr size_t GetArgs(lua_State* l, ArgsTuple& args)
        {
            return FuncUtility::GetArgs<0, 0, ArgsTuple, TArgs...>(l, args);
        }

        static constexpr size_t ReplaceUpvalues(lua_State* l, ArgsTuple& args)
        {
            return FuncUtility::ReplaceUpvalues<0, 0, ArgsTuple, TArgs...>(l, args);
        }

        template<typename R, typename D = R>
        struct CallHelper;

        template<typename R, typename ...Ts>
        struct CallHelper<R(*)(Ts...)>
        {
            template<typename ...Args>
            static UnwrapperReturn Call(Args&... args)
            {
                return fn(args...);
            }
        };

        template<typename R, class C, typename ...Ts>
        struct CallHelper<R(C::*)(Ts...)>
        {
            template<class Class, typename ...Args>
            static UnwrapperReturn Call(Class& arg, Args&... args)
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

        template<typename R, class C, typename ...Ts>
        struct CallHelper<R(C::*)(Ts...)const>
        {
            template<class Class, typename ...Args>
            static UnwrapperReturn Call(const Class& arg, Args&... args)
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

        template <size_t ... Is>
        inline static UnwrapperReturn Call(ArgsTuple& args, const std::index_sequence<Is...>)
        {
            return CallHelper<FnType>::Call(std::get<Is>(args)...);
        }
    };

}