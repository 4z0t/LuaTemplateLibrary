#include <lua.hpp>

namespace Lua
{
	template<typename T>
	struct TypeParser
	{
		static T Get(lua_State* l, int index);

		static bool Check(lua_State* l, int index);

		static void Push(lua_State* l, T && value);
	};

	template<>
	struct TypeParser<int>
	{
		static int Get(lua_State* l, int index)
		{
			return luaL_checkinteger(l, index);
		}

		static bool Check(lua_State* l, int index)
		{
			return lua_isinteger(l, index);
		}

		static void Push(lua_State* l, int value)
		{
			lua_pushinteger(l, value);
		}
	};

}
