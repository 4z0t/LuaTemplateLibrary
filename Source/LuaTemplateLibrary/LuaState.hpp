#pragma once
#include "LuaAux.hpp"
#include "LuaTypes.hpp"
#include "Exception.hpp"

namespace Lua
{
    StateWrap* WrapState(lua_State* l)
    {
        return (StateWrap*)l;
    }

    class StateWrap final
    {
    public:

        inline static StateWrap* Wrap(lua_State* l)
        {
            return WrapState(l);
        }

        inline static StateWrap* Create()
        {
            return Wrap(luaL_newstate());
        }

        static StateWrap* Create(lua_Alloc f, void* ud = nullptr)
        {
            auto s = Wrap(lua_newstate(f, ud));
            s->SetAllocFunction(f);
            return s;
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
            return luaL_openlibs(Unwrap());
        }

        bool DoFile(const char* name)
        {
            return luaL_dofile(Unwrap(), name);
        }

        template<typename T>
        void Push(const T& value)
        {
            return PushValue(Unwrap(), value);
        }

        void Pop(size_t n = 1)
        {
            return lua_pop(Unwrap(), static_cast<int>(n));
        }

        void Close()
        {
            lua_close(Unwrap());
        }

        template<typename ...Ts>
        void RegisterClosure(const char* name, lua_CFunction func, Ts&&... args)
        {
            Lua::RegisterClosure(Unwrap(), name, func, std::forward<Ts>(args)...);
        }

        template<typename TReturn = void, typename ...Ts>
        TReturn Call(const char* name, Ts&&... args)
        {
            return Lua::CallFunction<TReturn>(Unwrap(), name, std::forward<Ts>(args)...);
        }

        template<typename T>
        T Get(int index)
        {
            return GetValue<T>(Unwrap(), index);
        }

        void Run(const char* const s) throw(Exception)
        {
            if (luaL_dostring(Unwrap(), s))
            {
                lua_error(Unwrap());
            }
        }

        lua_CFunction SetAtPanicFuntion(lua_CFunction func)
        {
            return lua_atpanic(Unwrap(), func);
        }

        void SetAllocFunction(lua_Alloc func, void* ud = nullptr)
        {
            return lua_setallocf(Unwrap(), func, ud);
        }

        void Remove(int index)
        {
            return lua_remove(Unwrap(), index);
        }

        void Rotate(int index, int n)
        {
            return lua_rotate(Unwrap(), index, n);
        }

        void Duplicate(int index)
        {
            return lua_pushvalue(Unwrap(), index);
        }

        void SetTable(int index)
        {
            return lua_settable(Unwrap(), index);
        }

        int GetTable(int index)
        {
            return lua_gettable(Unwrap(), index);
        }

        template<typename ...Ts>
        const char* PushFormatString(const char* fmt, const Ts& ...args)
        {
            return lua_pushfstring(Unwrap(), fmt, args...);
        }

        template<typename T>
        T GetGlobal(const char* name)
        {
            StackPopper pop{ Unwrap(),1 };
            lua_getglobal(Unwrap(), name);
            return Get<T>(-1);
        }

    private:
        StateWrap() = delete;
        ~StateWrap() = delete;

    };

    struct OpNewAllocator
    {
        static void* Function(void* ud, void* ptr, size_t osize, size_t nsize)
        {
            if (nsize == 0)
            {
                Delete(ptr);
                return nullptr;
            }
            return NewMem(ptr, osize, nsize);
        }
    protected:
        static void Delete(void* ptr)
        {
            delete[] static_cast<char*>(ptr);
        }

        static void* NewMem(void* ptr, size_t osize, size_t nsize)
        {
            if (ptr == nullptr)
            {
                ptr = static_cast<void*>(new char[nsize]);
                return ptr;
            }
            size_t min_size = std::min(osize, nsize);
            void* new_ptr = static_cast<void*>(new char[nsize]);
            std::memcpy(new_ptr, ptr, min_size);
            Delete(ptr);
            return new_ptr;
        }
    };

    template<typename Allocator = void>
    class State
    {
    public:
        State(void* obj = nullptr)
        {
            if constexpr (!std::is_void_v<Allocator>)
            {
                m_state = StateWrap::Create(Allocator::Function, obj);
            }
            else
            {
                m_state = StateWrap::Create();
            }
        }

        State(const State&) = delete;
        State(State&&) = delete;
        State& operator=(const State&) = delete;
        State& operator=(State&&) = delete;

        ~State()
        {
            if (m_state)
                m_state->Close();
            m_state = nullptr;
        }

        void ThrowExceptions()
        {
            m_state->SetAtPanicFuntion(Exception::PanicFunc);
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

        State& AddFunction(const char* const name, lua_CFunction func)
        {
            return this->AddClosure(name, func);
        }

        template<typename ...Ts>
        State& AddClosure(const char* const name, lua_CFunction func, Ts&&... args)
        {
            m_state->RegisterClosure(name, func, std::forward<Ts>(args)...);
            return *this;
        }

        bool DoFile(const char* const path)
        {
            return m_state->DoFile(path);
        }

        template<typename TReturn = void, typename ...Ts>
        TReturn Call(const char* name, Ts&&... args)
        {
            return m_state->Call<TReturn>(name, std::forward<Ts>(args)...);
        }

        template<typename T>
        T To(int index)
        {
            return m_state->Get<T>(index);
        }

        StateWrap* const GetState()const
        {
            return m_state;
        }

        void Run(const char* const s) throw(Exception)
        {
            return m_state->Run(s);
        }

        void Run(const std::string& s)
        {
            return Run(s.c_str());
        }

        template<typename T>
        State& SetGlobal(const char* name, const T& value)
        {
            PushValue(m_state->Unwrap(), value);
            lua_setglobal(m_state->Unwrap(), name);

            return *this;
        }

    private:
        StateWrap* m_state = nullptr;

    };
}