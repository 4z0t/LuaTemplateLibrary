#pragma once
#include "LuaAux.hpp"
#include "LuaState.hpp"
#include "LuaTypes.hpp"

namespace Lua
{
    class RefObject
    {
    public:
        RefObject() = delete;
        RefObject(lua_State* l) noexcept : m_state(l) { assert(m_state, "Expected not null lua_State"); };
        RefObject(const State& state) noexcept : RefObject(state.GetState()->Unwrap()) {};

        RefObject(const RefObject& obj) noexcept : RefObject(obj.m_state)
        {
            obj._Push();
            m_ref = luaL_ref(m_state, LUA_REGISTRYINDEX);
        }

        RefObject(RefObject&& obj) noexcept : RefObject(obj.m_state)
        {
            m_ref = obj.m_ref;
            obj._Clear();
        }

        RefObject& operator=(const RefObject& obj)
        {
            this->_Unref();
            obj._Push();
            m_state = obj.m_state;
            m_ref = luaL_ref(m_state, LUA_REGISTRYINDEX);
            return *this;
        }

        RefObject& operator=(RefObject&& obj) noexcept
        {
            this->_Unref();
            this->m_ref = obj.m_ref;
            this->m_state = obj.m_state;
            obj._Clear();
            return *this;
        }

        ~RefObject()
        {
            _Unref();
        }

        static RefObject MakeTable(lua_State* l, int narr = 0, int nhash = 0)
        {
            RefObject obj{ l };
            lua_createtable(l, narr, nhash);
            obj.m_ref = luaL_ref(l, LUA_REGISTRYINDEX);
            return obj;
        }

        static RefObject MakeTable(const State& state, int narr = 0, int nhash = 0)
        {
            return MakeTable(state.GetState()->Unwrap(), narr, nhash);
        }

        static RefObject FromStack(lua_State* l, int index)
        {
            RefObject obj{ l };
            lua_pushvalue(l, index);
            obj.m_ref = luaL_ref(l, LUA_REGISTRYINDEX);
            return obj;
        }

        static RefObject FromStack(const State& state, int index)
        {
            return FromStack(state.GetState()->Unwrap(), index);
        }

        template<typename T>
        bool Is()const
        {
            AutoPop pop(this);
            bool res = TypeParser<T>::Check(m_state, -1);
            return res;
        }

        bool IsNil()const
        {
            AutoPop pop(this);
            bool res = lua_isnil(m_state, -1);
            return res;
        }

        bool IsTable()const
        {
            AutoPop pop(this);
            bool res = lua_istable(m_state, -1);
            return res;
        }

        Type Type()const
        {
            AutoPop pop(this);
            Lua::Type res = static_cast<Lua::Type>(lua_type(m_state, -1));
            return res;
        }

        const char* TypeName()const
        {
            AutoPop pop(this);
            const char* res = lua_typename(m_state, lua_type(m_state, -1));
            return res;
        }

    private:
        class AutoPop
        {
        public:
            AutoPop(const RefObject* obj) : m_obj(obj)
            {
                m_obj->_Push();
            }
            ~AutoPop()
            {
                m_obj->_Pop();
            }
            const RefObject* m_obj;
        };
        friend class AutoPop;

        void _Unref()
        {
            if (m_state)
            {
                luaL_unref(m_state, LUA_REGISTRYINDEX, m_ref);
                _Clear();
            }
        }

        void _Clear()
        {
            m_state = nullptr;
            m_ref = LUA_NOREF;
        }

        void _Push()const
        {
            lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_ref);
        }

        void _Pop()const
        {
            lua_pop(m_state, 1);
        }

        lua_State* m_state = nullptr;
        int m_ref = LUA_NOREF;
    };

}