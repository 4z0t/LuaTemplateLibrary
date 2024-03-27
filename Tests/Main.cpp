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

    Vector3f(float x, float y, float z) :x(x), y(y), z(z) {}

    Vector3f() :Vector3f(0, 0, 0) {}

    static float Length(const Vector3f& v)
    {
        return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    }

    static Vector3f Sum(const Vector3f& v1, const Vector3f& v2)
    {
        return { v1.x + v2.x , v1.y + v2.y , v1.z + v2.z };
    }

    std::string ToString(LTL::CState* state)const
    {
        std::string  s{ state->PushFormatString("Vector { %f, %f, %f }", x, y, z) };
        state->Pop();
        return s;
    }

    Vector3f operator+(const Vector3f& v)const
    {
        return Sum(*this, v);
    }
};

template<>
struct LTL::StackType<Vector3f>
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
        LTL::StackObjectView tbl{ l };
        tbl.RawSetI(1, vec.x);
        tbl.RawSetI(2, vec.y);
        tbl.RawSetI(3, vec.z);
    }
};

double GetSystemTime() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() / 1000.0;
}

LuaOptionalArg(OptStringValue, std::string, "My string");
LuaOptionalArg(OptionalDoubleHalf, double, 0.5);
LuaOptionalArg(OptionalDouble1, double, 1);

void CoolFunction(LTL::GRefObject& obj)
{
    using namespace std;
    cout << obj["Lua"].ToString() << endl;
    obj["Lua"] = "Yes";

}

void Test()
{
    using namespace std;
    using namespace LTL;
    LTL::State lua_state{};
    lua_state.OpenLibs();
    {

        Vector3f v{ 1,2,3 };
        lua_state
            /*
            .AddFunction("MakeArray", LTL::ClassFunction<MakeArray<int>>::Function<int>)
            .AddFunction("DoubleArray", LTL::CFunction<DoubleArray<float>>::Function<std::vector<float>>)
            .AddFunction("DoubleInt", LTL::ClassFunction<Callable>::Function<int, int>)
            .AddFunction("TripleInt", LTL::ClassFunction<Callable>::Function<int, int, int>)
            .AddClosure("SayHello", LTL::CClosure<Say, std::string>::Function<>, "Hello!")
            .AddFunction("VectorLen", LTL::CFunction <Vector3f::Length>::Function<Vector3f>)
            .AddFunction("VectorSum", LTL::CFunction <Vector3f::Sum>::Function<Vector3f, Vector3f>)
            .AddClosure("SayFoo", LTL::CClosure<Say, const char*>::Function, "Foo")
            .AddFunction("Say", LTL::CFunction<Say>::Function<const char*>)
            .AddFunction("Gamma", LTL::CFunction<Gamma>::Function<double>)
            .AddFunction("Hypot", LTL::CFunction<Hypot>::Function<float, float>)
            .AddFunction("MyFunc", LTL::CFunction<myfunc, float, float>::Function)
            .AddFunction("Def", LTL::CFunction<TestDefault, double, Default<double>>::Function)
            .AddClosure("Upval", LTL::CFunction<TestDefault, double, Upvalue<double>>::Function, 1.f)
            .AddClosure("Opt", LTL::CFunction<TestDefault, double, OptionalDoubleHalf>::Function)
            .AddClosure("PrintInc", LTL::CFunction<PrintClosureNumber2, Upvalue<int>, Upvalue<float>>::Function, 7, 3.2f)
            .AddClosure("SayBye", LTL::CFunction<Say, OptStringValue>::Function)
            .AddFunction("GetSystemTime", LTL::CFunction<GetSystemTime>::Function<>)
            */
            //.AddFunction("VecSum2", LTL::CFunction<&Vector3f::operator+, Vector3f, Vector3f>::Function)
            //.AddClosure("VecPtr", LTL::CFunction<&Vector3f::operator+, Upvalue<Vector3f*>, Vector3f>::Function, &v)
            //.AddClosure("CoolFunction", LTL::CFunction<CoolFunction, GRefObject>::Function)
            ;
    }


    if (lua_state.DoFile("main.lua"))
    {
        LTL::GRefObject obj = LTL::GRefObject::FromStack(lua_state, -1);

        cout << obj.Is<const char*>() << std::endl;
        cout << "error:" << lua_state.To<const char*>(-1) << std::endl;
        return;
    }

    LTL::GRefObject obj2(lua_state);
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
    using namespace LTL;
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
            .Add("a", AGetter<&MyUData::a>{})
            .Add("b", ASetter<&MyUData::b>{})
            .Add("Print", Method<MyUData, Print >{})
            ;

        Class<Vector3f>(lua_state, "Vector")
            .AddConstructor<Default<float>, Default<float>, Default<float>>()
            .Add("x", AProperty<&Vector3f::x>{})
            .Add("y", AProperty<&Vector3f::y>{})
            .Add("z", AProperty<&Vector3f::z>{})
            .Add("__add", Method<Vector3f, &Vector3f::operator+, Vector3f(Vector3f)>{})
            .Add("__tostring", Method<Vector3f, &Vector3f::ToString, string(CState*)>{})
            .Add("Dot", Method<Vector3f, Dot, Vector3f>{})
            ;

        /*
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
            "local ud2 = ud.Double(v1) "
            "print(ud2.a) "
            /*"print(getmetatable(ud)) "
            "print(getmetatable(ud).__gc) "
            "getmetatable(ud).__gc = nil "
            "print(getmetatable(ud).__gc) "
        );
        */

        constexpr auto myFunc = +[](const Vector3f& vec, float n)->float
            {
                return vec.x + vec.y + vec.z + n;
            };

        lua_state.AddFunction("MyFunction", CFunction<myFunc, UserData<Vector3f>, OptionalDouble1>::Function);

        auto vector = UserData<Vector3f>::Make(lua_state, 1, 2, 3);
        int r = lua_state.Call<int>("MyFunction", vector);

        cout << r << endl;

        lua_state.DoFile("example.lua");

        GRefObject my_ref = GRefObject::Global(lua_state, "MyFunction");
        cout << (my_ref == nullptr) << endl;




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
    LTL::State<T> s;
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
    using namespace LTL;
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


void TypeMatching()
{
    using namespace LTL;
    using namespace std;
    State s;

    s.AddClosure("Match", MatchArgumentTypes<int, int, Default<int>>::Function);
    s.Run("a = Match(1,2)");
    cout << GRefObject::Global(s, "a").To<bool>() << endl;

}

void MyVectorStackType()
{
    using namespace LTL;
    using namespace std;
    State s;
    s.OpenLibs();
    s.Run(
        "function PrintVector(v) "
        "   print(string.format('{ %.2f, %.2f, %.2f }',v[1],v[2],v[3])) "
        "end "
    );
    Vector3f v{ 1,2,3 };
    s.Call("PrintVector", v);

}

void RetOptional()
{
    using namespace LTL;
    using namespace std;
    State s;
    s.OpenLibs();
    s.Run(
        "function RetOpt(v) "
        "   if v < 10 and v>0 then  "
        "       return v "
        "   end "
        "   return nil "
        "end "
    );
    auto r = s.Call<optional<int>>("RetOpt", 1);
    if (r.has_value())
        cout << r.value() << endl;
    else
        cout << "null" << endl;
    r = s.Call<optional<int>>("RetOpt", 10);
    if (r.has_value())
        cout << r.value() << endl;
    else
        cout << "null" << endl;
}


void OnlyMethods()
{

    using namespace LTL;
    using namespace std;
    State s;
    s.OpenLibs();
    Class<Vector3f>(s, "Vector")
        .AddConstructor<Default<float>, Default<float>, Default<float>>()
        .Add("__add", Method<Vector3f, &Vector3f::operator+, Vector3f(Vector3f)>{})
        .Add("__tostring", Method<Vector3f, &Vector3f::ToString, string(CState*)>{})
        .Add("Dot", Method<Vector3f, Dot, Vector3f>{})
        ;
    s.Run(
        "local v = Vector(1,2,3) "
        "print(v) "
        "local v2 = Vector(4,5,6) "
        "print(v+v2) "
    );


}



void ClassTestStack()
{
    using namespace LTL;
    using namespace std;

    struct MyClass
    {
        MyClass(const string& name) :m_name(name)
        {

        }

        MyClass(const MyClass& other) :MyClass(other.m_name)
        {
        }

        string GetName()const
        {
            return m_name;
        }


    private:
        string m_name;
    };

    constexpr auto copyf = +[](const MyClass& other)-> MyClass
        {
            return { other };
        };

    State s;
    s.OpenLibs();
    Class<MyClass>(s, "MyClass")
        //.AddConstructor<string>()
        .AddGetter("Name", CFunction<&MyClass::GetName, UserData<MyClass>>::Function)
        .AddMetaMethod("Duplicate", Constructor<MyClass, UserData<MyClass>>::Function)
        ;
    MyClass instance("smart name");

    StackType<UserData<MyClass>>::Push(s.GetState()->Unwrap(), instance); // ugly
    GRefObject udata = GRefObject::FromTop(s);


    s.Run(
        "function MyClasPrintName(ud) "
        " print(ud.Name) " 
        "end "
        "function MyClasPrintNameDup(ud) "
        " print(ud.Name) "
        " print(ud:Duplicate().Name) "
        "end "
        
    );
    s.Call("MyClasPrintName", udata);
    s.Call("MyClasPrintNameDup", udata);


}

int main()
{
    //ClassTest();
    //Test();
    //PerfAllocTest();

    //StackObjectTest();
    //TypeMatching();
    //MyVectorStackType();
    //RetOptional();
    //OnlyMethods();

    ClassTestStack();


}