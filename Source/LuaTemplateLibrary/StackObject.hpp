#pragma once
#include "LuaAux.hpp"
#include "LuaState.hpp"
#include "LuaTypes.hpp"
#include "RefObject.hpp"

namespace Lua
{
    class StackObject
    {
    public:
        StackObject(lua_State* l, int index) : m_state(l), m_index(PushIndex(l, index)) {}

        template<typename T>
        StackObject(const RefObject<T>& obj) : m_state(obj.GetState()), m_index(PushRefObject(obj)) {}

        StackObject() = delete;
        StackObject(const StackObject&) = delete;
        StackObject(StackObject&&) = delete;

        StackObject& operator=(const StackObject&) = delete;
        StackObject& operator=(StackObject&&) = delete;


        template<typename T>
        T To()const
        {
            return StackType<T>::Get(m_state, m_index);
        }

        template<typename T>
        bool Is()const
        {
            return StackType<T>::Check(m_state, m_index);
        }

        template<typename T>
        static StackObject FromValue(lua_State* l, const T& value)
        {
            PushValue(l, value);
            return StackObject(l);
        }

        static StackObject Global(lua_State* l, const char* name)
        {
            lua_getglobal(l, name);
            return StackObject(l);
        }

        void Push()const
        {
            lua_pushvalue(m_state, m_index);
        }

        template<typename T>
        bool operator==(const T& value)const
        {
            StackPopper pop{ m_state,1 };
            PushValue(m_state, value);
            return lua_compare(m_state, m_index, -1, LUA_OPEQ);
        }

        template<>
        bool operator==(const StackObject& value)const
        {
            return m_state == value.m_state && lua_compare(m_state, m_index, value.m_index, LUA_OPEQ);
        }

        template<typename T>
        bool operator!=(const T& value)const
        {
            return !(*this == value);
        }

        template<typename R, typename T>
        R Get(const T& key)const
        {
            StackPopper pop{ m_state, 1 };
            PushValue(m_state, key);
            lua_gettable(m_state, m_index);
            return GetValue<R>(m_state, -1);
        }

        template<typename T>
        StackObject Get(const T& key)const
        {
            PushValue(m_state, key);
            lua_gettable(m_state, m_index);
            return { m_state };
        }

        template<typename K, typename V>
        void Set(const K& key, const V& value)
        {
            PushValue(m_state, key);
            PushValue(m_state, value);
            lua_settable(m_state, m_index);
        }

        template<typename R, typename T>
        R RawGet(const T& key)const
        {
            StackPopper pop{ m_state, 1 };
            PushValue(m_state, key);
            lua_rawget(m_state, m_index);
            return GetValue<R>(m_state, -1);
        }

        template<typename T>
        StackObject RawGet(const T& key)const
        {
            PushValue(m_state, key);
            lua_rawget(m_state, m_index);
            return { m_state };
        }

        template<typename K, typename V>
        void RawSet(const K& key, const V& value)
        {
            PushValue(m_state, key);
            PushValue(m_state, value);
            lua_rawset(m_state, m_index);
        }

        template<typename R>
        R Len()const
        {
            StackPopper pop{ m_state, 1 };
            lua_len(m_state, m_index);
            return GetValue<R>(m_state, -1);
        }

        StackObject Len()const
        {
            lua_len(m_state, m_index);
            return { m_state };
        }

        size_t RawLen()const
        {
            return lua_rawlen(m_state, m_index);
        }

        ~StackObject()
        {
            if (lua_gettop(m_state) == m_index)
            {
                lua_pop(m_state, 1);
            }
            else
            {
                lua_remove(m_state, m_index);
            }
            /*
            * lua_pushnil(m_state);
            * lua_replace(m_state, m_index);
            */
        }

        lua_State* const GetState()const
        {
            return m_state;
        }

    protected:

        StackObject(lua_State* l) : m_state(l), m_index(lua_absindex(l, -1)) {}

        static int PushIndex(lua_State* l, int index)
        {
            lua_pushvalue(l, index);
            return lua_absindex(l, -1);
        }

        template<typename T>
        static int PushRefObject(const RefObject<T>& obj)
        {
            obj.Push();
            return lua_absindex(obj.GetState(), -1);
        }

        lua_State* const m_state;
        const int m_index;
    };



    template<>
    struct StackType<StackObject>
    {
        static StackObject Get(lua_State* l, int index)
        {
            return { l, index };
        }

        static bool Check(lua_State* l, int index)
        {
            return !lua_isnone(l, index);
        }

        static void Push(lua_State* l, const StackObject& value)
        {
            assert(value.GetState() == l);
            value.Push();
        }
    };


}