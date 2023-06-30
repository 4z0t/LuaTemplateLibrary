#pragma once
#pragma once
#include <memory>
#include <iostream>
#include <lua.hpp>
#include <tuple>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <stdint.h>


namespace Lua
{
	inline void RegisterFunction(lua_State* l, const char* name, lua_CFunction func)
	{
		lua_pushcfunction(l, func);
		lua_setglobal(l, name);
	}

	template<typename T>
	inline void _PushValue(lua_State* l, const T* arg);

	template<>
	inline void _PushValue(lua_State* l, const char* arg)
	{
		lua_pushstring(l, arg);
	}

	template<typename T>
	inline void _PushValue(lua_State* l, T arg);

	template<>
	inline void _PushValue(lua_State* l, lua_Integer arg)
	{
		lua_pushinteger(l, arg);
	}

	template<>
	inline void _PushValue(lua_State* l, int arg)
	{
		_PushValue<lua_Integer>(l, static_cast<lua_Integer>(arg));
	}

	template<>
	inline void _PushValue(lua_State* l, lua_Number arg)
	{
		lua_pushnumber(l, arg);
	}

	template<>
	inline void _PushValue(lua_State* l, float arg)
	{
		_PushValue<lua_Number>(l, static_cast<lua_Number> (arg));
	}

	template<>
	inline void _PushValue(lua_State* l, std::nullptr_t arg)
	{
		lua_pushnil(l);
	}

	template<size_t N>
	size_t _PushArgs(lua_State* l)
	{
		return N;
	}

	template<size_t N, typename T, typename ...Ts>
	size_t _PushArgs(lua_State* l, T&& arg, Ts&&... args)
	{
		_PushValue(l, std::forward<std::decay_t<T>>(arg));
		return _PushArgs<N + 1>(l, std::forward<Ts>(args)...);
	}

	template< typename ...Ts>
	inline size_t PushArgs(lua_State* l, Ts&& ...args)
	{
		return _PushArgs<0>(l, std::forward<Ts>(args)...);
	}

	template<typename ...Ts>
	inline size_t _PrepareCall(lua_State* l, const char* name, Ts&&... args)
	{
		lua_getglobal(l, name);
		size_t n = PushArgs(l, std::forward<Ts>(args)...);
		return n;
	}

	template<typename ...Ts>
	void CallFunction(lua_State* l, const char* name, Ts&&... args)
	{
		size_t n = _PrepareCall(l, name, std::forward<Ts>(args)...);
		lua_call(l, n, 0);
	}

	template<typename ...Ts>
	bool CallFunctionProtected(lua_State* l, const char* name, Ts&&... args)
	{
		size_t n = _PrepareCall(l, name, std::forward<Ts>(args)...);
		return lua_pcall(l, n, 0, 0) == LUA_OK;
	}


	template<typename ...Ts>
	void RegisterClosure(lua_State* l, const char* name, lua_CFunction func, Ts&&... args)
	{
		size_t n = PushArgs(l, std::forward<Ts>(args)...);
		lua_pushcclosure(l, func, n);
		lua_setglobal(l, name);
	}

	template<typename T>
	T GetArg(lua_State* l, size_t Index);

	template<>
	float GetArg(lua_State* l, size_t Index)
	{
		return luaL_checknumber(l, Index);
	}

	template<>
	int GetArg(lua_State* l, size_t Index)
	{
		return luaL_checkinteger(l, Index);
	}

	template<>
	uint8_t GetArg(lua_State* l, size_t Index)
	{
		return luaL_checkinteger(l, Index);
	}

	template<>
	double GetArg(lua_State* l, size_t Index)
	{
		return luaL_checknumber(l, Index);
	}

	template<>
	const char* GetArg(lua_State* l, size_t Index)
	{
		return luaL_checkstring(l, Index);
	}

	template<size_t N, typename TArgsTuple>
	constexpr size_t GetArgs(lua_State* l, TArgsTuple& args)
	{
		return N;
	}


	template<size_t N, typename TArgsTuple, typename TArg, typename ...TArgs>
	constexpr size_t GetArgs(lua_State* l, TArgsTuple& args)
	{
		std::get<N>(args) = GetArg<TArg>(l, N + 1);
		return GetArgs<N + 1, TArgsTuple, TArgs...>(l, args);
	}

	template<size_t N, typename TArgsTuple>
	constexpr size_t GetUpvalues(lua_State* l, TArgsTuple& args)
	{
		return N;
	}

	template<size_t N, typename TArgsTuple, typename TArg, typename ...TArgs>
	constexpr size_t GetUpvalues(lua_State* l, TArgsTuple& args)
	{
		std::get<N>(args) = GetArg<TArg>(l, lua_upvalueindex(N + 1));
		return GetUpvalues<N + 1, TArgsTuple, TArgs...>(l, args);
	}


	template<size_t Index, typename TResult>
	inline size_t _PushResult(lua_State* l, TResult& result)
	{
		return Index;
	}


	template<size_t Index, typename TResult, typename T, typename ...Ts>
	inline size_t _PushResult(lua_State* l, TResult& result)
	{
		_PushValue<T>(l, std::get<Index>(result));
		return _PushResult<Index + 1, TResult, Ts...>(l, result);
	}


	template<typename T>
	inline void _PushResult(lua_State* l, T result)
	{
		_PushValue<T>(l, result);
	}

	template<typename T>
	inline void _PushResult(lua_State* l, std::vector<T>& result)
	{
		lua_createtable(l, result.size(), 0);
		for (size_t i = 0; i < result.size(); i++) {
			_PushValue<T>(l, result[i]);
			lua_rawseti(l, -2, i + 1);
		}
	}

	template<typename T>
	inline size_t PushResult(lua_State* l, T result)
	{
		_PushResult(l, result);
		return 1;
	}


	template<typename ...Ts>
	inline size_t PushResult(lua_State* l, std::tuple<Ts...>& result)
	{
		return _PushResult<0, std::tuple<Ts...>, Ts...>(l, result);
	}


	template<size_t Index, typename TResult>
	constexpr size_t _ReplaceUpvalue(lua_State* l, TResult& upvalues)
	{
		return Index;
	}

	template<size_t Index, typename TResult, typename T, typename ...Ts>
	inline size_t _ReplaceUpvalue(lua_State* l, TResult& upvalues)
	{
		if constexpr (!std::is_pointer<T>::value)
		{
			_PushValue<T>(l, std::get<Index>(upvalues));
			lua_replace(l, lua_upvalueindex((int)Index + 1));
		}
		return _ReplaceUpvalue<Index + 1, TResult, Ts...>(l, upvalues);
	}

	template<typename ...CArgs>
	void ReplaceUpvalues(lua_State* l, std::tuple<CArgs...>& upvalues)
	{
		_ReplaceUpvalue<0, std::tuple<CArgs...>, CArgs...>(l, upvalues);
	}
}