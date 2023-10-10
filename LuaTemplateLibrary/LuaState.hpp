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

		inline static lua_State* Unwrap(StateWrap* s)
		{
			return (lua_State*)s;
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
			return TypeParser<T>::Push(Unwrap(this), value);
		}

		void Pop(size_t n)
		{
			return lua_pop(Unwrap(this), static_cast<int>(n));
		}

		void Close()
		{
			lua_close(Unwrap(this));
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

	private:
		StateWrap* m_state = nullptr;

	};
}