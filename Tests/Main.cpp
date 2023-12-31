#pragma once
#define LUA_VERSION 504
#include <iostream>
#include <cmath>
#include <chrono>
#include "Lua/LuaLibrary.h"
#include "LuaTemplateLibrary/LTL.hpp"

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


struct Vector3f
{
    float x, y, z;

    Vector3f(float x, float y, float z) :x(x), y(y), z(z)
    {

    }
    Vector3f() :Vector3f(0, 0, 0) {}

    static float Length(const Vector3f& v)
    {
        return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    }

    static Vector3f Sum(const Vector3f& v1, const Vector3f& v2)
    {
        return { v1.x + v2.x , v1.y + v2.y , v1.z + v2.z };
    }

    std::string ToString(Lua::CState* state)const
    {
        std::string  s{ state->PushFormatString("Vector { %f, %f, %f }", x, y, z) };
        state->Pop();
        return s;
    }

    /* Vector3f operator+(const Vector3f& v)const
     {
         return Sum(*this, v);
     }*/

    Vector3f operator+(const Vector3f& v)const
    {
        return Sum(*this, v);
    }
};

template<>
struct Lua::StackType<Vector3f>
{
    using TReturn = Vector3f;

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
struct Lua::StackType<Vector3f*>
{
    using TReturn = Vector3f*;

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
        new (lua_newuserdata(l, sizeof(Vector3f))) Vector3f(*vec);
    }
};

double GetSystemTime() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() / 1000.0;
}

LuaOptionalArg(OptStringValue, std::string, "My string");
LuaOptionalArg(OptionalDoubleHalf, double, 0.5);
LuaOptionalArg(OptionalDouble1, double, 1);

void CoolFunction(Lua::GRefObject& obj)
{
    using namespace std;
    cout << obj["Lua"].ToString() << endl;
    obj["Lua"] = "Yes";

}

void Test()
{
    using namespace std;
    using namespace Lua;
    Lua::State lua_state{};
    lua_state.OpenLibs();
    {

        Vector3f v{ 1,2,3 };
        lua_state
            /*
            .AddFunction("MakeArray", Lua::ClassFunction<MakeArray<int>>::Function<int>)
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
            .AddFunction("MyFunc", Lua::CFunction<myfunc, float, float>::Function)
            .AddFunction("Def", Lua::CFunction<TestDefault, double, Default<double>>::Function)
            .AddClosure("Upval", Lua::CFunction<TestDefault, double, Upvalue<double>>::Function, 1.f)
            .AddClosure("Opt", Lua::CFunction<TestDefault, double, OptionalDoubleHalf>::Function)
            .AddClosure("PrintInc", Lua::CFunction<PrintClosureNumber2, Upvalue<int>, Upvalue<float>>::Function, 7, 3.2f)
            .AddClosure("SayBye", Lua::CFunction<Say, OptStringValue>::Function)
            .AddFunction("GetSystemTime", Lua::CFunction<GetSystemTime>::Function<>)
            */
            //.AddFunction("VecSum2", Lua::CFunction<&Vector3f::operator+, Vector3f, Vector3f>::Function)
            //.AddClosure("VecPtr", Lua::CFunction<&Vector3f::operator+, Upvalue<Vector3f*>, Vector3f>::Function, &v)
            //.AddClosure("CoolFunction", Lua::CFunction<CoolFunction, GRefObject>::Function)
            ;
    }


    if (lua_state.DoFile("main.lua"))
    {
        Lua::GRefObject obj = Lua::GRefObject::FromStack(lua_state, -1);

        cout << obj.Is<const char*>() << std::endl;
        cout << "error:" << lua_state.To<const char*>(-1) << std::endl;
        return;
    }

    Lua::GRefObject obj2(lua_state);
    cout << obj2.IsNil() << endl;

    GRefObject obj3 = GRefObject::MakeTable(lua_state);
    cout << obj3.IsTable() << endl;
    cout << obj3.TypeName() << endl;

    GRefObject obj4{ lua_state };
    obj4 = "Hello world";
    cout << obj4.ToString() << endl;
    obj4 = 4;
    cout << obj4.ToString() << endl;
    obj4 = 4.5f;
    cout << obj4.ToString() << endl;
    obj4 = 4.3;
    cout << obj4.ToString() << endl;
    obj4 = obj3;
    cout << obj4.ToString() << endl;
    cout << obj4.TypeName() << endl;

    obj4["Hi"] = "Bruh";
    cout << obj4["Hi"].ToString() << endl;
    GRefObject obj5{ lua_state };
    obj5 = "Bro";
    obj4["Hi"] = obj5;
    cout << obj4["Hi"].ToString() << endl;
    obj4["Hi"] = obj4;
    obj4["Hi"]["Hi"] = "No";

    obj4["hi"] = obj4["Hi"];
    cout << obj4["hi"].TypeName() << endl;
    cout << obj4["hi"].ToString() << endl;

    obj4[1.1] = 2.5;
    obj4[obj4[1.1]] = 1.1;
    obj4[obj4] = "TABLE";
    cout << obj4[1.1].ToString() << endl;
    cout << obj4[2.5].ToString() << endl;
    cout << obj4[obj4[1.1]].ToString() << endl;
    cout << obj4[obj4[2.5]].ToString() << endl;
    cout << obj4[obj4].ToString() << endl;
    obj4["func"] = CFunction<CoolFunction, GRefObject>::Function;

    cout << lua_state.Call<GRefObject>("Main", obj4) << endl;

    lua_state.Run("s = { a = 4 }");
    GRefObject s = GRefObject::Global(lua_state, "s");
    GRefObject global = GRefObject::Global(lua_state);
    cout << s.TypeName() << endl;
    cout << s.ToString() << endl;
    cout << s["a"].TypeName() << endl;
    cout << s["a"] << endl;
    cout << s["a"].To<int>() << endl;
    cout << (int)s["a"] << endl;

    cout << (s == s) << endl;
    cout << (global["s"] == s) << endl;
    cout << (s["a"] == s) << endl;
    cout << (s["a"] == 4) << endl;
    cout << (s == 4) << endl;
    lua_state.Run("t = { a = 4, b = 5, c = {1,2,3,4,5} , 1,2,3}");
    GRefObject v = GRefObject::Global(lua_state, "t");
    for (auto& [key, value] : v)
    {
        cout << key << ":" << value << endl;
    }
    for (auto& [key, value] : v["c"])
    {
        cout << key << ":" << value << endl;
    }
    cout << (s["a"] == s["a"]) << endl;

    global["bool"] = true;
    cout << global["bool"] << endl;
    cout << global["RefFunc"](1) << endl;
    global["RefFunc"].Call(1);


    lua_state.Push(3);
    lua_state.Push(2);
    lua_state.Push(1);
    cout << lua_gettop(lua_state.GetState()->Unwrap()) << endl;

}

struct MyUData
{
    MyUData() :MyUData(1, 2, 3)
    {

    }

    MyUData(int a, int b, int c) :a(a), b(b), c(c)
    {

    }

    //MyUData(const MyUData&) = delete;

    int GetA()const
    {
        return a;
    }

    void SetA(int a)
    {
        this->a = a;
    }

    void Hello(const char* s)const
    {
        std::cout << "Hello" << a << b << c << s << std::endl;
    }

    void Hello2(const char* s, const MyUData* other)const
    {
        std::cout << "Hello" << a << b << c << s << other->a << std::endl;
    }

    ~MyUData()
    {
        //std::cout << "GG" << std::endl;
    }

    // todo such methods
    MyUData Double()const
    {
        return MyUData{ a * 2,b * 2,c * 2 };
    }


    int a, b, c;
};

void Print(const MyUData* data)
{
    using namespace std;
    cout << "MyUData: " << data->a << ' ' << data->b << ' ' << data->c << endl;
}

float Dot(const Vector3f& v1, const Vector3f& v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

void ClassTest()
{
    using namespace Lua;
    using namespace std;
    try
    {
        struct MyAlloc :OpNewAllocator
        {
            static void* Function(void* ud, void* ptr, size_t osize, size_t nsize)
            {
                if (nsize == 0)
                {
                    cout << "Clearing mem at " << ptr << endl;
                    Delete(ptr);
                    return nullptr;
                }
                else
                {
                    cout << "Allocation mem at " << ptr << " with new size " << nsize << endl;
                    return NewMem(ptr, osize, nsize);
                }
            }
        };

        State<OpNewAllocator> lua_state{};
        lua_state.OpenLibs();
        lua_state.ThrowExceptions();
        Class<MyUData>(lua_state, "MyClass")
            .AddConstructor<Default<int>, Default<int>, Default<int>>()
            .Add("GetA", Method<MyUData, &MyUData::GetA>{})
            .Add("SetA", Method<MyUData, &MyUData::SetA, int>{})
            .Add("Hello", Method<MyUData, &MyUData::Hello, const char*>{})
            .Add("Hello2", Method<MyUData, &MyUData::Hello2, const char*, MyUData>{})
            .Add("Hello3", Method<MyUData, &MyUData::Hello2, void(const char*, MyUData)>{})
            .Add("Double", Method<MyUData, &MyUData::Double, MyUData()>{})
            .Add("a", Getter<MyUData, int, &MyUData::a>{})
            .Add("b", Setter<MyUData, int, &MyUData::b>{})
            .Add("Print", Method<MyUData, Print >{})
            ;

        Class<Vector3f>(lua_state, "Vector")
            .AddConstructor<Default<float>, Default<float>, Default<float>>()
            .Add("x", Property<Vector3f, float, &Vector3f::x>{})
            .Add("y", Property<Vector3f, float, &Vector3f::y>{})
            .Add("z", Property<Vector3f, float, &Vector3f::z>{})
            .Add("__add", Method<Vector3f, &Vector3f::operator+, Vector3f(Vector3f)>{})
            .Add("__tostring", Method<Vector3f, &Vector3f::ToString, string(CState*)>{})
            .Add("Dot", Method<Vector3f, Dot, Vector3f>{})
            ;

        cout << typeid(&Vector3f::ToString).name() << endl;
        cout << typeid(string(Vector3f::*)(CState*)const).name() << endl;


        //cout << typeid(UserDataValueClassWrapper<Vector3f>::AddUserDataValue<Vector3f>::type).name() << endl;
        lua_state.Run(
            "local v = Vector(1,2,3)       "
            "print(v.x)         "
            "print(v.y)         "
            "print(v.z)         "
            "print(v.w)         "
            "v.x = 5            "
            //"v.w = 1          "   
            "print(v.x)         "
            "local ud = MyClass(2,3,4)  "
            "print(ud.a)        "
            "print(ud.a)        "
            "ud.b = 5           "
            "print(ud.b)        "
            "ud:Double():Print() "
            "local v1 = Vector(1,2,3) "
            "local v2 = Vector(4,5,6) "
            "print(v1+v2) "
            //"v1.w = 6 "
            "print(v1+v2) "
            "print(v1:Dot(v2)) "
            "print({}<{}) "
        );




    }
    catch (Exception& ex)
    {
        std::cerr << ex.what() << std::endl;
    }
}

template<typename T>
double Measure()
{
    using namespace std;
    Lua::State<T> s;
    s.OpenLibs();
    double start = GetSystemTime();

    s.Run(
        "local insert = table.insert  "
        "local remove = table.remove "
        "for i = 1, 10 do "
        "local t = {}   "
        "for j = 1, 1000000 do "
        " insert(t, j)  "
        "end "
        "for j = 1, 1000000 do "
        " remove(t)  "
        "end "
        "end "
    );

    double end = GetSystemTime();
    return end - start;
}

void PerfAllocTest()
{
    using namespace Lua;
    using namespace std;
    auto t = 0.0;
    const auto n = 10;
    for (int i = 0; i < n; i++)
    {
        t += Measure<void>();
    }
    cout << t / n << endl;
    t = 0.0;
    for (int i = 0; i < n; i++)
    {
        t += Measure<OpNewAllocator>();
    }
    cout << t / n << endl;
}



void StackObjectTest()
{
    using namespace Lua;
    using namespace std;

    State s;

    StackObject s1 = StackObject::FromValue(s.GetState()->Unwrap(), 1);
    StackObject s2 = StackObject::FromValue(s.GetState()->Unwrap(), 2);
    StackObjectView s3{ s.GetState()->Unwrap() };

    cout << s1.RawEqual(s2) << endl;
    cout << s1.RawEqual(s3) << endl;
    cout << s3.RawEqual(s1) << endl;


}


void TypeMatching()
{
    using namespace Lua;
    using namespace std;
    State s;

    s.AddClosure("Match", MatchArgumentTypes<int, int, Default<int>>::Function);
    s.Run("a = Match(1,2)");
    cout << GRefObject::Global(s, "a").To<bool>() << endl;

}


int main()
{
    //ClassTest();
    //Test();
    //PerfAllocTest();

    //StackObjectTest();
    TypeMatching();
}