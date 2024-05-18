#pragma once
#include "LuaAux.hpp"
#include "Exception.hpp"

namespace LTL
{
    /**
     * @brief Класс-обертка, дублирующий функционал lua_State*.
     * 
     */
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

        inline void OpenLibs()
        {
            return luaL_openlibs(Unwrap());
        }

        inline void Require(const char* modname, lua_CFunction openf, bool global = true)
        {
            return luaL_requiref(Unwrap(), modname, openf, global);
        }

        inline PCallResult LoadFile(const char* name, const char* mode = nullptr)
        {
            return static_cast<PCallResult>(luaL_loadfilex(Unwrap(), name, mode));
        }

        inline void Call(int n_args, int n_results, lua_KContext ctx = 0, lua_KFunction k = nullptr)
        {
            lua_callk(Unwrap(), n_args, n_results, ctx, k);
        }

        inline PCallResult PCall(int n_args, int n_results, int i_errfunc = 0, lua_KContext ctx = 0, lua_KFunction k = nullptr)
        {
            return static_cast<PCallResult>(lua_pcallk(Unwrap(), n_args, n_results, i_errfunc, ctx, k));
        }

        inline PCallResult DoFile(const char* name)
        {
            auto res = LoadFile(name);
            if (res != PCallResult::Ok)
                return res;
            return PCall(0, LUA_MULTRET);
        }

        inline PCallResult LoadString(const char* s)
        {
            return static_cast<PCallResult>(luaL_loadstring(Unwrap(), s));
        }

        inline PCallResult DoString(const char* s)
        {
            auto res = LoadString(s);
            if (res != PCallResult::Ok)
                return res;
            return PCall(0, LUA_MULTRET);
        }

        inline void Error()
        {
            lua_error(Unwrap());
        }

        inline int GetTop()
        {
            return lua_gettop(Unwrap());
        }

        inline void SetTop(int top)
        {
            return lua_settop(Unwrap(), top);
        }

        template<typename T>
        void Push(const T& value)
        {
            return PushValue(Unwrap(), value);
        }

        template<typename T>
        T Get(int index)
        {
            return GetValue<T>(Unwrap(), index);
        }

        inline void Pop(size_t n = 1)
        {
            return lua_pop(Unwrap(), static_cast<int>(n));
        }

        inline void Close()
        {
            lua_close(Unwrap());
        }

        template<typename ...TArgs>
        void RegisterClosure(const char* name, lua_CFunction func, TArgs&&... args)
        {
            LTL::RegisterClosure(Unwrap(), name, func, std::forward<TArgs>(args)...);
        }

        template<typename TReturn = void, typename ...TArgs>
        TReturn Call(const char* name, TArgs&&... args)
        {
            return CallFunction<TReturn>(Unwrap(), GlobalValue{ name }, std::forward<TArgs>(args)...);
        }

        template<typename TReturn = void, typename ...TArgs>
        PCallReturn<TReturn> PCall(const char* name, TArgs&&... args)
        {
            return PCallFunction<TReturn>(Unwrap(), GlobalValue{ name }, std::forward<TArgs>(args)...);
        }

        void Run(const char* const s) noexcept(false)
        {
            if (DoString(s) != PCallResult::Ok)
            {
                Error();
            }
        }

        inline lua_CFunction SetAtPanicFuntion(lua_CFunction func)
        {
            return lua_atpanic(Unwrap(), func);
        }

        inline void SetAllocFunction(lua_Alloc func, void* ud = nullptr)
        {
            return lua_setallocf(Unwrap(), func, ud);
        }

        inline void Remove(int index)
        {
            return lua_remove(Unwrap(), index);
        }

        inline void Rotate(int index, int n)
        {
            return lua_rotate(Unwrap(), index, n);
        }

        inline void Duplicate(int index)
        {
            return lua_pushvalue(Unwrap(), index);
        }

        inline void SetTable(int index)
        {
            return lua_settable(Unwrap(), index);
        }

        inline Type GetTable(int index)
        {
            return static_cast<Type>(lua_gettable(Unwrap(), index));
        }

        template<typename ...Ts>
        inline const char* PushFormatString(const char* fmt, const Ts& ...args)
        {
            return lua_pushfstring(Unwrap(), fmt, args...);
        }

        inline Type GetGlobal(const char* name)
        {
            return static_cast<LTL::Type>(lua_getglobal(Unwrap(), name));
        }

        inline void SetGlobal(const char* name)
        {
            return lua_setglobal(Unwrap(), name);
        }

        template<typename T>
        T GetGlobal(const char* name)
        {
            GetGlobal(name);
            T v = Get<T>(-1);
            Pop(1);
            return v;
        }

        template<typename T>
        void SetGlobal(const char* name, const T& value)
        {
            Push(value);
            SetGlobal(name);
        }

    private:
        CState() = delete;
        ~CState() = delete;
    };

    template<>
    struct StackType<CState*> : AlwaysValid
    {
        static CState* Get(lua_State* l, int index)
        {
            return CState::Wrap(l);
        }
    };
}