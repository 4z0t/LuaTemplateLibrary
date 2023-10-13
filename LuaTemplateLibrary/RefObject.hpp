#pragma once
#include "LuaAux.hpp"
#include "LuaState.hpp"

namespace Lua
{


	class RefObject
	{
	public:
		RefObject() {};
		RefObject(lua_State* l) : m_state(l)
		{

		};

		RefObject(const State& state) : RefObject(state.GetState()->Unwrap())
		{

		};

		RefObject& operator=(const RefObject& obj)
		{
			this->Unref();
			obj._Push();
			this->m_ref = luaL_ref(m_state, LUA_REGISTRYINDEX);
			this->m_state = obj.m_state;
			return *this;
		}

		~RefObject()
		{
			this->Unref();
		}


		static RefObject FromStack(const State& state, int index)
		{
			return FromStack(state.GetState()->Unwrap(), index);
		}

		static RefObject FromStack(lua_State* l, int index)
		{
			RefObject obj;
			lua_pushvalue(l, index);
			obj.m_ref = luaL_ref(l, LUA_REGISTRYINDEX);
			obj.m_state = l;
			return obj;
		}

		template<typename T>
		bool Is()const
		{
			this->_Push();
			bool res = TypeParser<T>::Check(m_state, -1);
			lua_pop(m_state, 1);
			return res;
		}

		bool IsNil()const
		{
			this->_Push();
			bool res = lua_isnil(m_state, -1);
			lua_pop(m_state, 1);
			return res;
		}

	private:

		void Unref()
		{
			if (m_state)
			{
				luaL_unref(m_state, LUA_REGISTRYINDEX, m_ref);
				m_state = nullptr;
				m_ref = LUA_NOREF;
			}
		}

		void _Push()const
		{
			lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_ref);
		}

		int m_ref = LUA_NOREF;
		lua_State* m_state = nullptr;
	};

}