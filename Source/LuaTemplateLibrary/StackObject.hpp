#pragma once
#include "LuaAux.hpp"
#include "LuaState.hpp"
#include "LuaTypes.hpp"

namespace Lua
{
    class StackObject
    {
    public:
        StackObject(lua_State* l, int index) : m_state(l), m_index(PushIndex(l, index)) {}

        StackObject() = delete;
        StackObject(const StackObject&) = delete;
        StackObject(StackObject&&) = delete;

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

        ~StackObject()
        {
            lua_remove(m_state, m_index);
            /*
            * lua_pushnil(m_state);
            * lua_replace(m_state, m_index);
            */
        }

    private:

        StackObject(lua_State* l) : m_state(l), m_index(lua_absindex(l, -1)) {}

        static int PushIndex(lua_State* l, int index)
        {
            lua_pushvalue(l, index);
            return lua_absindex(l, -1);
        }

        lua_State* const m_state;
        const int m_index;
    };
}