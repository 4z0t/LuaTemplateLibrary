#pragma once


#include "Tests.h"
#include <cmath>

#include "LuaFunctions.hpp"
#include "LuaClass.hpp"
#include "LuaState.hpp"
#include <chrono>
#include "FuncTraits.hpp"

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
		std::cout << "2 args\n";
		return a * a + b;
	}

	int operator()(int a, int b, int c)
	{
		std::cout << "3 args\n";
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

template<typename T>
std::vector<T> DoubleArray(const std::vector<T>& arr)
{
	std::vector<T> result(arr.size() * 2);
	for (size_t i = 0; i < result.size(); i++)
	{
		result[i] = arr[i % arr.size()];
	}
	return result;
}


void PrintClosureNumber2(int& a, float& b)
{
	a++;
	b += 0.1;
	std::cout << "Value is " << a << " " << b << std::endl;
}


void Say(const std::string& str)
{
	std::cout << "Value is " << str << std::endl;
}


float myfunc(float a, float b)
{
	return a * b - a / b;
}


float TestDefault(float a, float b)
{
	return a * b;
}

inline double Gamma(double a)
{
	return tgamma(a);
}

inline float Hypot(float a, float b)
{
	return hypotf(a, b);
}

#include "LuaTypes.hpp"

struct Vector3f
{
	float x, y, z;

	static float Length(const Vector3f& v)
	{
		return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	}

	static Vector3f Sum(const Vector3f& v1, const Vector3f& v2)
	{
		return { v1.x + v2.x , v1.y + v2.y , v1.z + v2.z };
	}
};

template<>
struct Lua::TypeParser<Vector3f>
{
	static bool Check(lua_State* l, int index)
	{
		return lua_istable(l, index);
	}

	static Vector3f Get(lua_State* l, int index)
	{
		if (!lua_istable(l, index))
		{
			return { 0,0,0 };
		}
		Vector3f res;
		lua_pushvalue(l, index);
		lua_rawgeti(l, -1, 1);
		res.x = lua_tonumber(l, -1);
		lua_rawgeti(l, -2, 2);
		res.y = lua_tonumber(l, -1);
		lua_rawgeti(l, -3, 3);
		res.z = lua_tonumber(l, -1);
		lua_pop(l, 4);
		return res;
	}

	static void Push(lua_State* l, const Vector3f& vec)
	{
		lua_createtable(l, 3, 0);
		lua_pushnumber(l, vec.x);
		lua_rawseti(l, -2, 1);
		lua_pushnumber(l, vec.y);
		lua_rawseti(l, -2, 2);
		lua_pushnumber(l, vec.z);
		lua_rawseti(l, -2, 3);
	}
};

double GetSystemTime() {
	using namespace std::chrono;
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() / 1000.0;
}


struct OptionalDouble1 :OptionalBase<double>
{
	static constexpr double value = 1;
};

struct OptStringValue :OptionalBase<std::string>
{
	static const std::string value;

};
const std::string OptStringValue::value = "My string";

void Test()
{
	using namespace std;
	Lua::State s{};
	s.OpenLibs();
	lua_State* l = luaL_newstate();
	luaL_openlibs(l);
	using namespace Tests;
	Lua::ClassWrapper<TestClass>::Init(l, "TestClass", TestClass::_class);

	std::cout << TestClass::className << std::endl;

	Lua::RegisterFunction(l, "MakeArray", Lua::ClassFunction<MakeArray<int>>::Function<int>);
	Lua::RegisterFunction(l, "DoubleArray", Lua::CFunction<DoubleArray<float>>::Function<std::vector<float>>);
	Lua::RegisterFunction(l, "DoubleInt", Lua::ClassFunction<Callable>::Function<int, int>);
	Lua::RegisterFunction(l, "TripleInt", Lua::ClassFunction<Callable>::Function<int, int, int>);
	//Lua::RegisterClosure(l, "PrintInc", Lua::CClosure<PrintClosureNumber2, int, float>::Function<>, 7, 3.2f);
	Lua::RegisterClosure(l, "SayHello", Lua::CClosure<Say, std::string>::Function<>, "Hello!");
	//Lua::RegisterClosure(l, "SayBye", Lua::CClosure<Say, std::string>::Function<>, "Bye!");

	Lua::RegisterFunction(l, "VectorLen", Lua::CFunction <Vector3f::Length>::Function<Vector3f>);
	Lua::RegisterFunction(l, "VectorSum", Lua::CFunction <Vector3f::Sum>::Function<Vector3f, Vector3f>);

	Lua::RegisterClosure(l, "SayFoo", Lua::CClosure<Say, const char*>::Function, "Foo");
	Lua::RegisterFunction(l, "Say", Lua::CFunction<Say>::Function<const char*>);
	Lua::RegisterFunction(l, "Gamma", Lua::CFunction<Gamma>::Function<double>);
	Lua::RegisterFunction(l, "Hypot", Lua::CFunction<Hypot>::Function<float, float>);
	Lua::RegisterFunction(l, "MyFunc", Lua::CFunction<myfunc>::Function<double, double>);

	Lua::RegisterFunction(l, "Def", Lua::Closure<TestDefault, double, Default<double>>::Function);
	Lua::RegisterClosure(l, "Upval", Lua::Closure<TestDefault, double, Upvalue<double>>::Function, 1.f);
	Lua::RegisterClosure(l, "Opt", Lua::Closure<TestDefault, double, OptionalDouble1>::Function);
	Lua::RegisterClosure(l, "PrintInc", Lua::Closure<PrintClosureNumber2, Upvalue<int>, Upvalue<float>>::Function, 7, 3.2f);
	Lua::RegisterClosure(l, "SayBye", Lua::Closure<Say, OptStringValue>::Function);

	Lua::RegisterFunction(l, "GetSystemTime", Lua::CFunction<GetSystemTime>::Function<>);



	if (luaL_dofile(l, "main.lua"))
	{
		cout << "error:" << lua_tostring(l, -1) << std::endl;
	}

	Lua::CallFunction(l, "Main");

}