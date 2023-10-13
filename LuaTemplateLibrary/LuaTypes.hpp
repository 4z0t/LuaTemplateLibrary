#pragma once
#include <lua.hpp>

namespace Lua
{
	template<typename T>
	struct TypeParser
	{
		static T Get(lua_State* l, int index);

		static bool Check(lua_State* l, int index);

		static void Push(lua_State* l, T&& value);
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

	/*template<>
	struct TypeParser<std::nullptr_t>
	{
		static bool Check(lua_State* l, int index)
		{
			return lua_isnil(l, index);
		}

		static void Push(lua_State* l, std::nullptr_t value)
		{
			lua_pushnil(l);
		}
	};*/

	template<>
	struct TypeParser<float>
	{
		static float Get(lua_State* l, int index)
		{
			return luaL_checknumber(l, index);
		}

		static bool Check(lua_State* l, int index)
		{
			return lua_isnumber(l, index);
		}

		static void Push(lua_State* l, float value)
		{
			lua_pushnumber(l, value);
		}
	};

	template<>
	struct TypeParser<double>
	{
		static double Get(lua_State* l, int index)
		{
			return luaL_checknumber(l, index);
		}

		static bool Check(lua_State* l, int index)
		{
			return lua_isnumber(l, index);
		}

		static void Push(lua_State* l, double value)
		{
			lua_pushnumber(l, value);
		}
	};

	template<>
	struct TypeParser<long double>
	{
		static long double Get(lua_State* l, int index)
		{
			return luaL_checknumber(l, index);
		}

		static bool Check(lua_State* l, int index)
		{
			return lua_isnumber(l, index);
		}

		static void Push(lua_State* l, long double value)
		{
			lua_pushnumber(l, value);
		}
	};

	template<>
	struct TypeParser<bool>
	{
		static bool Get(lua_State* l, int index)
		{
			return lua_toboolean(l, index);
		}

		static bool Check(lua_State* l, int index)
		{
			return lua_isboolean(l, index);
		}

		static void Push(lua_State* l, bool value)
		{
			lua_pushboolean(l, value);
		}
	};

	template<>
	struct TypeParser<const char*>
	{
		static const char* Get(lua_State* l, int index)
		{
			return luaL_checkstring(l, index);
		}

		static bool Check(lua_State* l, int index)
		{
			return lua_isstring(l, index);
		}

		static void Push(lua_State* l, const char* value)
		{
			lua_pushstring(l, value);
		}
	};

	template<>
	struct TypeParser<std::string> : public TypeParser<const char*>
	{
		static std::string Get(lua_State* l, int index)
		{
			return { TypeParser<const char*>::Get(l, index) };
		}

		static void Push(lua_State* l, const std::string& value)
		{
			TypeParser<const char*>::Push(l, value.c_str());
		}
	};

	template<typename T>
	struct TypeParser<std::vector<T>>
	{
		static std::vector<T> Get(lua_State* l, int index)
		{
			lua_pushvalue(l, index);
			auto size = lua_rawlen(l, -1);
			std::vector<T> result(size);
			for (size_t i = 0; i < size; i++) {
				lua_rawgeti(l, -1, i + 1);
				result[i] = TypeParser<T>::Get(l, -1);
				lua_pop(l, 1);
			}
			lua_pop(l, 1);
			return result;
		}

		static bool Check(lua_State* l, int index)
		{
			return lua_istable(l, index);
		}

		static void Push(lua_State* l, const std::vector<T>& value)
		{
			lua_createtable(l, value.size(), 0);
			for (size_t i = 0; i < value.size(); i++) {
				TypeParser<T>::Push(l, value[i]);
				lua_rawseti(l, -2, i + 1);
			}
		}
	};
}
