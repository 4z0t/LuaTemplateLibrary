#pragma once
#include "LuaAux.hpp"
#include "LuaTypes.hpp"

namespace Lua
{
    class StateWrap final
    {
    public:

        inline static StateWrap* Wrap(lua_State* l)
        {
            return (StateWrap*)l;
        }

        inline static StateWrap* Create()
        {
            return Wrap(luaL_newstate());
        }

        inline static lua_State* Unwrap(const StateWrap* s)
        {
            return (lua_State*)s;
        }

        inline lua_State* Unwrap()const
        {
            return StateWrap::Unwrap(this);
        }

        inline lua_State* Unwrap()
        {
            return StateWrap::Unwrap(this);
        }

        void OpenLibs()
        {
            return luaL_openlibs(Unwrap(this));
        }

        bool DoFile(const char* name)
        {
            return luaL_dofile(Unwrap(this), name);
        }

        template<typename T>
        void Push(const T& value)
        {
            return TypeParser<const_decay_t<T>>::Push(Unwrap(this), value);
        }

        void Pop(size_t n)
        {
            return lua_pop(Unwrap(this), static_cast<int>(n));
        }

        void Close()
        {
            lua_close(Unwrap(this));
        }

        template<typename ...Ts>
        void RegisterClosure(const char* name, lua_CFunction func, Ts&&... args)
        {
            Lua::RegisterClosure(Unwrap(this), name, func, std::forward<Ts>(args)...);
        }

        template<typename RType = void, typename ...Ts>
        RType Call(const char* name, Ts&&... args)
        {
            return Lua::CallFunction<RType>(Unwrap(this), name, std::forward<Ts>(args)...);
        }

        template<typename T>
        T Get(int index)
        {
            return TypeParser<T>::Get(Unwrap(this), index);
        }

        int Run(const char* s)
        {
            return luaL_dostring(Unwrap(this), s);
        }

    private:
        StateWrap() = delete;
        ~StateWrap() = delete;

    };

    class State
    {
    public:
        State()
        {
            m_state = StateWrap::Create();
        }
        State(const State&) = delete;
        State& operator=(const State&) = delete;

        ~State()
        {
            if (m_state)
                m_state->Close();
            m_state = nullptr;
        }

        void OpenLibs()
        {
            return m_state->OpenLibs();
        }

        template<typename T>
        void Push(const T& value)
        {
            return m_state->Push<T>(value);
        }

        void Pop(size_t n)
        {
            return m_state->Pop(n);
        }

        State& AddFunction(const char* name, lua_CFunction func)
        {
            return this->AddClosure(name, func);
        }

        template<typename ...Ts>
        State& AddClosure(const char* name, lua_CFunction func, Ts&&... args)
        {
            m_state->RegisterClosure(name, func, std::forward<Ts>(args)...);
            return *this;
        }

        bool DoFile(const char* path)
        {
            return m_state->DoFile(path);
        }

        template<typename RType = void, typename ...Ts>
        RType Call(const char* name, Ts&&... args)
        {
            return m_state->Call<RType>(name, std::forward<Ts>(args)...);
        }

        template<typename T>
        T To(int index)
        {
            return m_state->Get<T>(index);
        }

        const StateWrap* GetState()const
        {
            return m_state;
        }

        int Run(const char* s)
        {
            return m_state->Run(s);
        }

    private:
        StateWrap* m_state = nullptr;

    };
}