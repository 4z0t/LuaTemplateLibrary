#pragma once


#include "Tests.h"

#include "LuaFunctions.hpp"
#include "LuaClass.hpp"




namespace Tests
{
	struct TestClass
	{
		static const char* className;


		static int Print(lua_State* l)
		{
			std::cout << "Hello!" << std::endl;

			return 0;
		}


		static int Create(lua_State* l)
		{
			size_t nbytes = sizeof(TestClass);
			TestClass* a = (TestClass*)lua_newuserdata(l, nbytes);

			luaL_getmetatable(l, TestClass::className);
			lua_setmetatable(l, -2);

			return 1;  /* new userdatum is already on the stack */
		}


		inline static const luaL_Reg meta[] = {
			{"print", TestClass::Print},
			{"aboba", TestClass::Print},
			{NULL, NULL}
		};

		inline static const luaL_Reg _class[] = {
			{"new", TestClass::Create},
			{NULL, NULL}
		};
	};

	const char* TestClass::className = typeid(TestClass).name();

}


class Callable
{
public:

	Callable(lua_State* l)
	{
		std::cout << "called with lua state in it" << std::endl;
	}

	int operator()(int a, int b)
	{
		return a * a + b;
	}

	int operator()(int a, int b, int c)
	{
		return a * a + b * c;
	}
};

template<typename T>
struct MakeArray
{
	std::vector<T> operator()(int n)
	{
		return std::vector<T>(n);
	}
};

void PrintClosureNumber2(int& a, float& b)
{
	a++;
	b += 0.1;
	std::cout << "Value is " << a << " " << b << std::endl;
}


void Say(const char* str)
{
	std::cout << "Value is " << str << std::endl;
}


void Test()
{
	using namespace std;
	lua_State* l = luaL_newstate();
	luaL_openlibs(l);
	using namespace Tests;
	Lua::ClassWrapper<TestClass>::Init(l, "TestClass", TestClass::_class);

	std::cout << TestClass::className << std::endl;

	Lua::RegisterFunction(l, "MakeArray", Lua::FunctionWrapper<MakeArray<int>, int>::Function);
	Lua::RegisterFunction(l, "DoubleInt", Lua::FunctionWrapper<Callable, int, int>::Function);
	Lua::RegisterFunction(l, "TripleInt", Lua::FunctionWrapper<Callable, int, int, int>::Function);
	Lua::RegisterClosure(l, "PrintInc", Lua::ClosureWrapper<PrintClosureNumber2>::Function<int, float>, 7, 3.2f);
	Lua::RegisterClosure(l, "SayHello", Lua::ClosureWrapper<Say>::Function<const char*>, "Hello!");
	Lua::RegisterClosure(l, "SayBye", Lua::ClosureWrapper<Say>::Function<const char*>, "Bye!");



	if (luaL_dofile(l, "main.lua"))
	{
		cout << "error:" << lua_tostring(l, -1) << std::endl;
	}

	Lua::CallFunction(l, "Main");

}