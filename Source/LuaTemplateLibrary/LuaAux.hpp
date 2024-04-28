#pragma once
#include "LuaTypes.hpp"
#include "FuncArguments.hpp"
#include <optional>

namespace LTL
{
    /**
     * @brief Класс для доступа к объектам по ссылке
     * внутри ВМ Lua.
     */
    struct RefGlobalAccess
    {
        static int GetRef(lua_State* l)
        {
            return luaL_ref(l, LUA_REGISTRYINDEX);
        }

        static void Unref(lua_State* l, int ref)
        {
            luaL_unref(l, LUA_REGISTRYINDEX, ref);
        }

        static void PushRef(lua_State* l, int ref)
        {
            lua_rawgeti(l, LUA_REGISTRYINDEX, ref);
        }
    };

    template<typename T>
    using const_decay_t = std::decay_t<const T>;

    // Considered harmful due to destructor throw issue
   /* struct StackPopper final
    {
        StackPopper(lua_State* l, int n) : m_state(l), m_n(n) {}

        ~StackPopper()
        {
            lua_pop(m_state, m_n);
        }

        StackPopper(const StackPopper&) = delete;
        StackPopper(StackPopper&&) = delete;
        StackPopper& operator=(const StackPopper&) = delete;
        StackPopper& operator=(StackPopper&&) = delete;
    private:
        lua_State* const m_state;
        const int m_n;
    };*/

    /**
     * @brief Класс для восстановления стека после выхода из области видимости
     *
     */
    struct StackRestorer final
    {
        StackRestorer(lua_State* l) : m_state(l), m_top(lua_gettop(l)) {}

        ~StackRestorer()
        {
            lua_settop(m_state, m_top);
        }

        StackRestorer(const StackRestorer&) = delete;
        StackRestorer(StackRestorer&&) = delete;
        StackRestorer& operator=(const StackRestorer&) = delete;
        StackRestorer& operator=(StackRestorer&&) = delete;
    private:
        lua_State* const m_state;
        const int m_top;
    };

    template<int Index>
    struct StackIndex {};

    template<int Index>
    struct StackType<StackIndex<Index>>
    {
        static bool Check(lua_State* l, int index)
        {
            return !lua_isnone(l, index);
        }

        static void Push(lua_State* l, const StackIndex<Index>&)
        {
            lua_pushvalue(l, Index);
        }
    };

    template<typename T>
    T GetValue(lua_State* l, int index)
    {
        return StackType<T>::Get(l, index);
    }

    template<typename T>
    inline void PushValue(lua_State* l, const T& arg)
    {
        StackType<const_decay_t<T>>::Push(l, arg);
    }

#pragma region Push Args

    template<size_t N>
    size_t _PushArgs(lua_State* l)
    {
        return N;
    }

    template<size_t N, typename T, typename ...Ts>
    size_t _PushArgs(lua_State* l, T&& arg, Ts&&... args)
    {
        PushValue(l, std::forward<T>(arg));
        return _PushArgs<N + 1>(l, std::forward<Ts>(args)...);
    }

    template< typename ...Ts>
    inline size_t PushArgs(lua_State* l, Ts&& ...args)
    {
        return _PushArgs<0>(l, std::forward<Ts>(args)...);
    }

#pragma endregion

#pragma region Function and Closure register
    inline void RegisterFunction(lua_State* l, const char* name, lua_CFunction func)
    {
        lua_pushcfunction(l, func);
        lua_setglobal(l, name);
    }

    inline  void RegisterFunction(lua_State* l, const std::string& name, lua_CFunction func)
    {
        return RegisterFunction(l, name.c_str(), func);
    }

    template<typename ...Ts>
    inline void RegisterClosure(lua_State* l, const char* name, lua_CFunction func, Ts&&... args)
    {
        size_t n = PushArgs(l, std::forward<Ts>(args)...);
        lua_pushcclosure(l, func, static_cast<int>(n));
        lua_setglobal(l, name);
    }
#pragma endregion

#pragma region Tuple From Stack

    template<int Index, size_t N, typename Tuple>
    constexpr void _GetTupleFromStack(lua_State* l, Tuple& tpl)
    {
    }

    template<int Index, size_t N, typename Tuple, typename TArg, typename ...TArgs>
    constexpr void _GetTupleFromStack(lua_State* l, Tuple& tpl)
    {
        std::get<N>(tpl) = GetValue<TArg>(l, Index);
        return _GetTupleFromStack<Index + 1, N + 1, Tuple, TArgs...>(l, tpl);
    }

    template<int Index, typename ...TArgs>
    constexpr decltype(auto) GetTupleFromStack(lua_State* l)
    {
        using Tuple = std::tuple<TArgs...>;
        Tuple tpl{};
        _GetTupleFromStack<Index, 0, Tuple, TArgs... >(l, tpl);
        return tpl;
    }


#pragma endregion

#pragma region Get Result

    template<typename T>
    struct ResulltNum : std::integral_constant<int, 1> {};

    template<typename ...Ts>
    struct ResulltNum< MultReturn<Ts...>> : std::integral_constant<int, sizeof...(Ts)> {};

    template<typename T>
    struct StackResultGetter
    {
        static constexpr T Get(lua_State* l)
        {
            return GetValue<T>(l, -1);
        }
    };

    template<typename ...Ts>
    struct StackResultGetter<MultReturn<Ts...>>
    {
        using T = MultReturn<Ts...>;

        static constexpr T Get(lua_State* l)
        {
            return GetTupleFromStack<-ResulltNum<T>::value, Ts...>(l);
        }
    };


#pragma endregion

#pragma region Call

    template<typename TReturn = void>
    inline  TReturn CallStack(lua_State* l, const size_t  n_args)
    {
        lua_call(l, static_cast<int>(n_args), ResulltNum<TReturn>::value);
        TReturn r = StackResultGetter<TReturn>::Get(l);
        lua_pop(l, ResulltNum<TReturn>::value);
        return r;
    }

    template<>
    inline void CallStack(lua_State* l, const size_t  n_args)
    {
        lua_call(l, static_cast<int>(n_args), 0);
    }

#pragma endregion

#pragma region PCall

    enum class PCallResult : int
    {
        Ok = LUA_OK,
        Yield = LUA_YIELD,
        ERRRUN = LUA_ERRRUN,
        ERRSYNTAX = LUA_ERRSYNTAX,
        ERRMEM = LUA_ERRMEM,
        ERRERR = LUA_ERRERR,
    };

    struct PCallReturnBase
    {
        PCallResult status = PCallResult::Ok;

        PCallReturnBase(PCallResult status) : status{ status } {}

        bool IsOk()const noexcept
        {
            return status == PCallResult::Ok;
        }

        bool operator==(PCallResult status)const noexcept
        {
            return this->status == status;
        }

        operator bool()const
        {
            return IsOk();
        }
    };

    template<typename T>
    struct PCallReturn : PCallReturnBase
    {
        std::optional<T> result = std::nullopt;

        PCallReturn(const T& result, PCallResult status) : PCallReturnBase(status), result{ result } {}

        PCallReturn(PCallResult status) : PCallReturnBase(status), result{ std::nullopt } {}
    };

    template<>
    struct PCallReturn<void> : PCallReturnBase
    {
        using PCallReturnBase::PCallReturnBase;
    };

    template<typename TReturn = void>
    inline PCallReturn<TReturn> PCallStack(lua_State* l, const size_t  n_args)
    {
        PCallResult status = static_cast<PCallResult>(lua_pcall(l, static_cast<int>(n_args), ResulltNum<TReturn>::value, 0));
        if (status == PCallResult::Ok)
        {
            TReturn result = StackResultGetter<TReturn>::Get(l);
            lua_pop(l, ResulltNum<TReturn>::value);
            return { result, status };
        }
        else
        {
            return { status };
        }
    }

    template<>
    inline PCallReturn<void> PCallStack(lua_State* l, const size_t  n_args)
    {
        return static_cast<PCallResult>(lua_pcall(l, static_cast<int>(n_args), 0, 0));
    }

#pragma endregion

#pragma region Generic call functions

    template<typename TReturn = void, typename F, typename ...TArgs>
    inline TReturn CallFunction(lua_State* l, const F& func, TArgs&& ...args)
    {
        PushValue(l, func);
        const size_t n = PushArgs(l, std::forward<TArgs>(args)...);
        return CallStack<TReturn>(l, n);
    }

    template<typename TReturn = void, typename F, typename ...TArgs>
    inline PCallReturn<TReturn> PCallFunction(lua_State* l, const F& func, TArgs&& ...args)
    {
        PushValue(l, func);
        const size_t n = PushArgs(l, std::forward<TArgs>(args)...);
        return PCallStack<TReturn>(l, n);
    }

#pragma endregion

#pragma region Push Result

    template<size_t Index, typename TResult>
    inline size_t _PushResult(lua_State* l, TResult& result)
    {
        return Index;
    }

    template<size_t Index, typename TResult, typename T, typename ...Ts>
    inline size_t _PushResult(lua_State* l, TResult& result)
    {
        PushValue<T>(l, std::get<Index>(result));
        return _PushResult<Index + 1, TResult, Ts...>(l, result);
    }

    template<typename T>
    inline void _PushResult(lua_State* l, const T& result)
    {
        PushValue<T>(l, result);
    }

    template<typename UnwrappedT, typename T = UnwrappedT>
    inline size_t PushResult(lua_State* l, UnwrappedT& result)
    {
        StackType<T>::Push(l, result);
        return 1;
    }

    template<typename ...Ts>
    inline size_t PushResult(lua_State* l, MultReturn<Ts...>& result)
    {
        return _PushResult<0, MultReturn<Ts...>, Ts...>(l, result);
    }

    template<>
    inline size_t PushResult(lua_State* l, StackResult& result)
    {
        return result.n_results;
    }

#pragma endregion

#pragma region Upvalues Replace

    template<size_t Index, typename TResult>
    constexpr size_t _ReplaceUpvalue(lua_State* l, TResult& upvalues)
    {
        return Index;
    }

    template<size_t Index, typename TResult, typename T, typename ...Ts>
    inline size_t _ReplaceUpvalue(lua_State* l, TResult& upvalues)
    {
        if constexpr (!std::is_pointer<T>::value)
        {
            PushValue<T>(l, std::get<Index>(upvalues));
            lua_replace(l, lua_upvalueindex((int)Index + 1));
        }
        return _ReplaceUpvalue<Index + 1, TResult, Ts...>(l, upvalues);
    }

    template<typename ...CArgs>
    void ReplaceUpvalues(lua_State* l, std::tuple<CArgs...>& upvalues)
    {
        _ReplaceUpvalue<0, std::tuple<CArgs...>, CArgs...>(l, upvalues);
    }

#pragma endregion
}