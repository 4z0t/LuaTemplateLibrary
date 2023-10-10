#pragma once
#include "LuaAux.hpp"

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



	private:
		StateWrap* m_state = nullptr;

	};
}