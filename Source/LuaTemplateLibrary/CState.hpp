#pragma once
#include "LuaAux.hpp"
#include "Exception.hpp"

namespace LTL
{
    class CState final
    {
    public:

        inline static CState* Wrap(lua_State* l)
        {
            return (CState*)l;
        }

        inline static CState* Create()
        {
            return Wrap(luaL_newstate());
        }

        static CState* Create(lua_Alloc f, void* ud = nullptr)
        {
            return Wrap(lua_newstate(f, ud));
        }

        inline static lua_State* Unwrap(const CState* s)
        {
            return (lua_State*)s;
        }

        inline lua_State* Unwrap()const
        {
            return CState::Unwrap(this);
        }

        inline lua_State* Unwrap()
        {
            return CState::Unwrap(this);
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
            LTL::RegisterClosure(Unwrap(), name, func, std::forward<Ts>(args)...);
        }

        template<typename TReturn = void, typename ...Ts>
        TReturn Call(const char* name, Ts&&... args)
        {
            lua_getglobal(Unwrap(), name);
            const size_t n = PushArgs(Unwrap(), std::forward<Ts>(args)...);
            return CallStack<TReturn>(Unwrap(), n);
        }

        template<typename TReturn = void, typename ...Ts>
        PCallReturn<TReturn> PCall(const char* name, Ts&&... args)
        {
            lua_getglobal(Unwrap(), name);
            const size_t n = PushArgs(Unwrap(), std::forward<Ts>(args)...);
            return PCallStack<TReturn>(Unwrap(), n);
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
            lua_getglobal(Unwrap(), name);
            T v = Get<T>(-1);
            Pop(1);
            return v;
        }

        template<typename T>
        void SetGlobal(const char* name, const T& value)
        {
            Push(value);
            lua_setglobal(Unwrap(), name);
        }

    private:
        CState() = delete;
        ~CState() = delete;
    };

    inline CState* WrapState(lua_State* l)
    {
        return (CState*)l;
    }
}