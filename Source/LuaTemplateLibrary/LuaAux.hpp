#pragma once
#include "LuaTypes.hpp"

namespace Lua
{
    template<typename T>
    using const_decay_t = std::decay_t<const T>;

    template<size_t N>
    struct StackPopper
    {
        constexpr StackPopper(lua_State* l) : m_state(l) {}

        ~StackPopper()
        {
            lua_pop(m_state, N);
        }
    private:
        lua_State* m_state;
    };

    void RegisterFunction(lua_State* l, const char* name, lua_CFunction func)
    {
        lua_pushcfunction(l, func);
        lua_setglobal(l, name);
    }

    void RegisterFunction(lua_State* l, const std::string& name, lua_CFunction func)
    {
        return RegisterFunction(l, name.c_str(), func);
    }

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

    template<typename ...Ts>
    inline size_t _PrepareCall(lua_State* l, const char* name, Ts&&... args)
    {
        lua_getglobal(l, name);
        return PushArgs(l, std::forward<Ts>(args)...);
    }

    template<typename TReturn = void>
    TReturn CallStack(lua_State* l, const size_t  n_args)
    {
        if constexpr (std::is_void_v<TReturn>)
        {
            lua_call(l, static_cast<int>(n_args), 0);
        }
        else
        {
            StackPopper<1>{l};
            lua_call(l, static_cast<int>(n_args), 1);
            return GetValue<TReturn>(l, -1);
        }
    }

    template<typename TReturn = void, typename ...Ts>
    TReturn CallFunction(lua_State* l, const char* name, Ts&&... args)
    {
        size_t n = _PrepareCall(l, name, std::forward<Ts>(args)...);
        return CallStack<TReturn>(l, n);
    }

    template<typename ...Ts>
    bool CallFunctionProtected(lua_State* l, const char* name, Ts&&... args)
    {
        size_t n = _PrepareCall(l, name, std::forward<Ts>(args)...);
        return lua_pcall(l, n, 0, 0) == LUA_OK;
    }


    template<typename ...Ts>
    void RegisterClosure(lua_State* l, const char* name, lua_CFunction func, Ts&&... args)
    {
        size_t n = PushArgs(l, std::forward<Ts>(args)...);
        lua_pushcclosure(l, func, static_cast<int>(n));
        lua_setglobal(l, name);
    }

    template<size_t N, typename TArgsTuple>
    constexpr size_t GetArgs(lua_State* l, TArgsTuple& args)
    {
        return N;
    }

    template<size_t N, typename TArgsTuple, typename TArg, typename ...TArgs>
    constexpr size_t GetArgs(lua_State* l, TArgsTuple& args)
    {
        std::get<N>(args) = GetValue<TArg>(l, N + 1);
        return GetArgs<N + 1, TArgsTuple, TArgs...>(l, args);
    }

    template<size_t N, typename TArgsTuple>
    constexpr size_t GetUpvalues(lua_State* l, TArgsTuple& args)
    {
        return N;
    }

    template<size_t N, typename TArgsTuple, typename TArg, typename ...TArgs>
    constexpr size_t GetUpvalues(lua_State* l, TArgsTuple& args)
    {
        std::get<N>(args) = GetValue<TArg>(l, lua_upvalueindex((int)N + 1));
        return GetUpvalues<N + 1, TArgsTuple, TArgs...>(l, args);
    }


    template<size_t Index, typename TResult>
    inline size_t _PushResult(lua_State* l, TResult& result)
    {
        return Index;
    }


    template<size_t Index, typename TResult, typename T, typename ...Ts>
    inline size_t _PushResult(lua_State* l, TResult& result)
    {
        _PushValue<T>(l, std::get<Index>(result));
        return _PushResult<Index + 1, TResult, Ts...>(l, result);
    }


    template<typename T>
    inline void _PushResult(lua_State* l, const T& result)
    {
        PushValue<T>(l, result);
    }

    template<typename T>
    inline size_t PushResult(lua_State* l, T result)
    {
        _PushResult(l, result);
        return 1;
    }


    template<typename ...Ts>
    inline size_t PushResult(lua_State* l, std::tuple<Ts...>& result)
    {
        return _PushResult<0, std::tuple<Ts...>, Ts...>(l, result);
    }


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
}