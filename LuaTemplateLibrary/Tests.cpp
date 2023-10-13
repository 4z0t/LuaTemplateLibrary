#pragma once


#include "Tests.h"
#include <cmath>

#include "LuaFunctions.hpp"
#include "LuaClass.hpp"
#include "LuaState.hpp"
#include <chrono>
#include "FuncTraits.hpp"
#include "RefObject.hpp"

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

	Vector3f operator+(const Vector3f& v)const
	{
		return Sum(*this, v);
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

template<>
struct Lua::TypeParser<Vector3f*>
{
	static bool Check(lua_State* l, int index)
	{
		return lua_isuserdata(l, index);
	}

	static Vector3f* Get(lua_State* l, int index)
	{
		void* ud = lua_touserdata(l, index);

		return (Vector3f*)ud;
	}

	static void Push(lua_State* l, const Vector3f* vec)
	{
		Vector3f* ud = (Vector3f*)lua_newuserdata(l, sizeof(Vector3f));
		ud->operator=(*vec);
	}
};

double GetSystemTime() {
	using namespace std::chrono;
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() / 1000.0;
}

LuaOptionalArg(OptStringValue, std::string, "My string");
LuaOptionalArg(OptionalDoubleHalf, double, 0.5);
LuaOptionalArg(OptionalDouble1, double, 1);


void Test()
{
	using namespace std;
	using namespace Lua;
	Lua::State lua_state{};
	lua_state.OpenLibs();
	/*lua_State* l = luaL_newstate();
	luaL_openlibs(l);
	using namespace Tests;
	Lua::ClassWrapper<TestClass>::Init(l, "TestClass", TestClass::_class);*/

	//std::cout << TestClass::className << std::endl;

	Vector3f v{ 1,2,3 };
	lua_state.AddFunction("MakeArray", Lua::ClassFunction<MakeArray<int>>::Function<int>)
		.AddFunction("DoubleArray", Lua::CFunction<DoubleArray<float>>::Function<std::vector<float>>)
		.AddFunction("DoubleInt", Lua::ClassFunction<Callable>::Function<int, int>)
		.AddFunction("TripleInt", Lua::ClassFunction<Callable>::Function<int, int, int>)
		.AddClosure("SayHello", Lua::CClosure<Say, std::string>::Function<>, "Hello!")
		.AddFunction("VectorLen", Lua::CFunction <Vector3f::Length>::Function<Vector3f>)
		.AddFunction("VectorSum", Lua::CFunction <Vector3f::Sum>::Function<Vector3f, Vector3f>)
		.AddClosure("SayFoo", Lua::CClosure<Say, const char*>::Function, "Foo")
		.AddFunction("Say", Lua::CFunction<Say>::Function<const char*>)
		.AddFunction("Gamma", Lua::CFunction<Gamma>::Function<double>)
		.AddFunction("Hypot", Lua::CFunction<Hypot>::Function<float, float>)
		.AddFunction("MyFunc", Lua::Closure<myfunc, float, float>::Function)
		.AddFunction("Def", Lua::Closure<TestDefault, double, Default<double>>::Function)
		.AddClosure("Upval", Lua::Closure<TestDefault, double, Upvalue<double>>::Function, 1.f)
		.AddClosure("Opt", Lua::Closure<TestDefault, double, OptionalDoubleHalf>::Function)
		.AddClosure("PrintInc", Lua::Closure<PrintClosureNumber2, Upvalue<int>, Upvalue<float>>::Function, 7, 3.2f)
		.AddClosure("SayBye", Lua::Closure<Say, OptStringValue>::Function)
		.AddFunction("GetSystemTime", Lua::CFunction<GetSystemTime>::Function<>)
		.AddFunction("VecSum2", Lua::Closure<&Vector3f::operator+, Vector3f, Vector3f>::Function)
		.AddClosure("VecPtr", Lua::Closure<&Vector3f::operator+, Upvalue<Vector3f*>, Vector3f>::Function, &v);
	;




	if (lua_state.DoFile("main.lua"))
	{
		Lua::RefObject obj = Lua::RefObject::FromStack(lua_state, -1);

		cout << obj.Is<const char*>() << std::endl;
		cout << "error:" << lua_state.To<const char*>(-1) << std::endl;
		return;
	}

	lua_state.Call("Main");

}