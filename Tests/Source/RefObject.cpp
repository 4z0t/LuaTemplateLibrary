#include "TestBase.hpp"

#pragma region RefObjectTests
struct RefObjectTests : TestBase
{

};
TEST_F(RefObjectTests, Basic)
{
    using namespace LTL;
    GRefObject obj{ l };

    ASSERT_EQ(obj.GetState(), l);
    ASSERT_TRUE(obj.IsNil());


}


TEST_F(RefObjectTests, ValueAccess)
{
    using namespace LTL;
    Run("result = true");
    ASSERT_TRUE(Result().Is<bool>());
    ASSERT_TRUE(Result().To<bool>());
    ASSERT_TRUE((bool)Result());

    Run("result = 4");
    ASSERT_TRUE(Result().Is<int>());
    ASSERT_EQ(Result().To<int>(), 4);
    ASSERT_EQ(Result().To<unsigned int>(), 4u);
    ASSERT_EQ((int)Result(), 4);
    ASSERT_FLOAT_EQ(Result().To<float>(), 4.f);
    ASSERT_DOUBLE_EQ(Result().To<double>(), 4.0);
    ASSERT_DOUBLE_EQ(Result().To<long double>(), 4.0);


    Run("result = {a = 4}");
    ASSERT_TRUE(Result().IsTable());
    ASSERT_TRUE(Result()["a"].Is<int>());

    GRefObject key{ l, "a" };
    ASSERT_TRUE(Result()[key].Is<int>());
}

TEST_F(RefObjectTests, TestImplicitConvertion)
{
    using namespace LTL;
    using namespace std;
    Run("result = 4");
    {
        bool res = Result();
        ASSERT_TRUE(res);
    }
    {
        int res = Result();
        ASSERT_EQ(res, 4);
    }
    {
        float res = Result();
        ASSERT_FLOAT_EQ(res, 4.f);
    }
    {
        double res = Result();
        ASSERT_DOUBLE_EQ(res, 4.0);
    }

    Run("result = {}");
    {
        bool res = Result();
        ASSERT_TRUE(res) << "Table value is not true";
    }
    ASSERT_THROW(int res = Result(), Exception);
    ASSERT_THROW(float res = Result(), Exception);
    ASSERT_THROW(double res = Result(), Exception);
    ASSERT_THROW(const char* res = Result(), Exception);
    ASSERT_THROW(string res = Result(), Exception);
    ASSERT_THROW(string_view res = Result(), Exception);

    Run("result = nil");
    {
        bool res = Result();
        ASSERT_FALSE(res) << "nil value is not false";
    }
    ASSERT_THROW(int res = Result(), Exception);
    ASSERT_THROW(float res = Result(), Exception);
    ASSERT_THROW(double res = Result(), Exception);
    ASSERT_THROW(const char* res = Result(), Exception);
    ASSERT_THROW(string res = Result(), Exception);
    ASSERT_THROW(string_view res = Result(), Exception);
}

TEST_F(RefObjectTests, IteratorTest)
{
    {
        Run("result = {1,3,4,6,7,10}");
        auto result = Result();

        int arr[]{ 1,3,4,6,7,10 };
        int index = 0;

        for (const auto& [key, value] : result)
        {

            ASSERT_TRUE(key.Is<int>());
            ASSERT_TRUE(key == index + 1);
            ASSERT_TRUE(value.Is<int>());
            ASSERT_TRUE(value == arr[index]);
            index++;
        }
    }
    {
        Run("result = {1,3,4,6,7,10}");
        auto result = Result();

        int arr[]{ 1,3,4,6,7,10 };
        int index = 0;

        for (auto [key, value] : result)
        {

            ASSERT_TRUE(key.Is<int>());
            ASSERT_TRUE(key == index + 1);
            ASSERT_TRUE(value.Is<int>());
            ASSERT_TRUE(value == arr[index]);
            index++;
            key = 0;
            ASSERT_TRUE(key.Is<int>());
            ASSERT_TRUE(key == 0);
            value = nullptr;
            ASSERT_TRUE(value.Is<void>());
            ASSERT_TRUE(value == nullptr);
        }
    }
    //    Run("result = {1,3,4,6,7, a = 5, b = 'abc'}");

}

TEST_F(RefObjectTests, CompareTest)
{
    using namespace LTL;
    {

        Run("a, b = 2, 3");
        GRefObject a = GRefObject::Global(l, "a");
        GRefObject b = GRefObject::Global(l, "b");

        ASSERT_FALSE(a == b);
        ASSERT_TRUE(a != b);
        ASSERT_TRUE(a < b);
        ASSERT_FALSE(a > b);
        ASSERT_FALSE(a >= b);
        ASSERT_TRUE(a <= b);
    }
    {
        Run("a = {}"
            " b = a "
        );
        GRefObject a = GRefObject::Global(l, "a");
        GRefObject b = GRefObject::Global(l, "b");

        ASSERT_TRUE(a == b);
        ASSERT_FALSE(a != b);
        ASSERT_THROW(a > b, Exception);
        ASSERT_THROW(a < b, Exception);
        ASSERT_THROW(a >= b, Exception);
        ASSERT_THROW(a <= b, Exception);
    }

}


// Access classes tests

struct MyAccess
{
    static int GetRef(lua_State* l)
    {
        return luaL_ref(l, LUA_REGISTRYINDEX);
    }

    static void Unref(lua_State* l, int ref)
    {
        luaL_unref(l, LUA_REGISTRYINDEX, ref);
    }

    static void PushRef(lua_State* l, int ref)
    {
        lua_rawgeti(l, LUA_REGISTRYINDEX, ref);
    }
};

struct MyAccess2
{
    static int GetRef(lua_State* l)
    {
        GetTable(l);
        lua_pushvalue(l, -2);
        int ref = luaL_ref(l, -2);
        lua_pop(l, 2);
        return ref;
    }

    static void Unref(lua_State* l, int ref)
    {
        GetTable(l);
        luaL_unref(l, -1, ref);
        lua_pop(l, 1);
    }

    static void PushRef(lua_State* l, int ref)
    {
        GetTable(l);
        lua_rawgeti(l, -1, ref);
        lua_remove(l, -2);
    }
private:

    static void GetTable(lua_State* l)
    {
        if (!assigned)
            AssignTable(l);
        lua_getregp(l, GetIndex());
        assert(lua_istable(l, -1));
    }

    static void AssignTable(lua_State* l)
    {
        lua_newtable(l);
        lua_setregp(l, GetIndex());
        assigned = true;
    }

    static constexpr const void* GetIndex()
    {
        return &index;
    }
    static const char index = 0;
    static bool assigned;
};
bool MyAccess2::assigned = false;

TEST_F(RefObjectTests, AccessClasses)
{
    using namespace LTL;

    {
        RefObject<MyAccess> obj{ l };
        GRefObject gobj{ l };
        obj = "Hello";
        gobj = obj;
        ASSERT_TRUE(obj == gobj);
    }
    {
        RefObject<MyAccess2> obj{ l };
        GRefObject gobj{ l };
        obj = "Hello";
        gobj = obj;
        ASSERT_TRUE(obj == gobj);
        ASSERT_TRUE(obj.Is<const char*>());
        ASSERT_STREQ(obj.To<const char*>(), "Hello");
    }
    {
        RefObject<MyAccess2> obj{ l };
        GRefObject gobj{ l };
        obj = RefObject<MyAccess2>::MakeTable(l);
        obj["key"] = "Hi";
        gobj = obj["key"];
        ASSERT_TRUE(gobj == obj["key"]);
        ASSERT_TRUE(gobj.Is<const char*>());
        ASSERT_STREQ(gobj.To<const char*>(), "Hi");
    }
}


TEST_F(RefObjectTests, SelfCallTest)
{

    using namespace LTL;
    Run("result = {                             "
        "   func = function(self, s, n)         "
        "      local s_ = self.custom_string    "
        "      for i = 1,n do                   "
        "          s_ = s_ .. s                 "
        "      end                              "
        "      return s_                        "
        "   end,                                "
        "   custom_string = 'My string',        "
        "}                                      "
    );
    {
        auto s = Result().SelfCall<GRefObject>("func", "1", 5);
        ASSERT_TRUE(s.Is<const char*>());
        ASSERT_STREQ(s.To<const char*>(), "My string11111");
    }
    {
        ASSERT_THROW(Result()["func"].Call<GRefObject>("1", 5), Exception);
    }
    {
        auto s = Result()["func"].Call<GRefObject>(Result(), "1", 5);
        ASSERT_TRUE(s.Is<const char*>());
        ASSERT_STREQ(s.To<const char*>(), "My string11111");
    }

}
#pragma endregion

#pragma region ExceptionTests
struct ExceptionTests : TestBase
{

};

TEST_F(ExceptionTests, ThrowTests)
{
    using namespace LTL;
    Run("result = true");
    ASSERT_THROW(Result().Call(), Exception);
    ASSERT_THROW(int _ = Result().Call<int>(), Exception);
    ASSERT_THROW(Result()["Key"].To<int>(), Exception);

}
#pragma endregion

#pragma region UserDataTests
struct UserDataTests : TestBase
{

};

TEST_F(UserDataTests, MoveCtorTest)
{
    using namespace LTL;
    struct CantCopy
    {

        CantCopy(int a) :a(a) {}
        CantCopy(const CantCopy&) = delete;
        CantCopy(CantCopy&& other)noexcept
        {
            a = std::move(other.a);
        }

        int Print()
        {
            return a;
        }

        CantCopy Dup()
        {
            return CantCopy(a * 2);
        }

        int a = 4;
    };

    Class<CantCopy>(l, "CantCopy")
        .AddConstructor<Default<int>>()
        .Add("Duplicate", Method<&CantCopy::Dup, CantCopy()>{})
        .Add("Print", Method<&CantCopy::Print>{})
        ;
    Run("local a = CantCopy() "
        "result = a:Print()"
    );


    ASSERT_TRUE(Result().Is<int>());
    ASSERT_EQ(Result().To<int>(), 0);

    Run("local a = CantCopy(4) "
        "result = a:Duplicate():Print()"
    );

    ASSERT_TRUE(Result().Is<int>());
    ASSERT_EQ(Result().To<int>(), 8);
}


TEST_F(UserDataTests, PropertyTests)
{
    using namespace LTL;
    struct Vector3f
    {
        float x, y, z;

        Vector3f(float x, float y, float z) :x(x), y(y), z(z)
        {

        }
        Vector3f() :Vector3f(0, 0, 0) {}

        float Length()const
        {
            return sqrtf(x * x + y * y + z * z);
        }
    };

    Class<Vector3f>(l, "Vector")
        .AddConstructor<Default<float>, Default<float>, Default<float>>()
        .Add("x", Property<Vector3f, float, &Vector3f::x>{})
        .Add("y", Property<Vector3f, float, &Vector3f::y>{})
        .Add("z", Property<Vector3f, float, &Vector3f::z>{})
        .Add("Length", Method<&Vector3f::Length>{})
        .AddGetter("length", CFunction<&Vector3f::Length, UserData<Vector3f>>{})
        ;
    {

        Run("v = Vector(1,2,3)  "
            "result = v.x   "
        );
        ASSERT_TRUE(Result().Is<float>());
        ASSERT_FLOAT_EQ(Result().To<float>(), 1.f);
        Run(
            "result = v.y   "
        );
        ASSERT_TRUE(Result().Is<float>());
        ASSERT_FLOAT_EQ(Result().To<float>(), 2.f);
        Run(
            "result = v.z   "
        );
        ASSERT_TRUE(Result().Is<float>());
        ASSERT_FLOAT_EQ(Result().To<float>(), 3.f);
        {
            Run("v.x = 2        "
                "result = v.y   "
            );
            ASSERT_TRUE(Result().Is<float>());
            ASSERT_FLOAT_EQ(Result().To<float>(), 2.f);
            Run("v.y = 4        "
                "result = v.y   "
            );
            ASSERT_TRUE(Result().Is<float>());
            ASSERT_FLOAT_EQ(Result().To<float>(), 4.f);
            Run("v.z = 8        "
                "result = v.z   "
            );
            ASSERT_TRUE(Result().Is<float>());
            ASSERT_FLOAT_EQ(Result().To<float>(), 8.f);
        }
        {
            ASSERT_THROW(Run("v.w = 2"), Exception);
            Run("result = v.w   "
            );
            ASSERT_TRUE(Result().Is<void>());
        }
        {
            Run("v = Vector(1,2,3)  "
                "result = v:Length()  "
            );
            ASSERT_TRUE(Result().Is<float>());
            ASSERT_FLOAT_EQ(Result().To<float>(), Vector3f(1, 2, 3).Length());
        }
        {
            Run("v = Vector(1,2,3)  "
                "result = v.length  "
            );
            ASSERT_TRUE(Result().Is<float>());
            ASSERT_FLOAT_EQ(Result().To<float>(), Vector3f(1, 2, 3).Length());
        }
    }
    {
        using UDVector3f = UserData<Vector3f>;
        auto vec = UDVector3f::Make(l);

        ASSERT_TRUE(vec.IsUserData());
        ASSERT_TRUE(vec.Is<UDVector3f>());
        ASSERT_TRUE(vec["x"].Is<float>());
        ASSERT_FLOAT_EQ(vec["x"].To<float>(), 0);
        ASSERT_TRUE(vec["y"].Is<float>());
        ASSERT_FLOAT_EQ(vec["y"].To<float>(), 0);
        ASSERT_TRUE(vec["z"].Is<float>());
        ASSERT_FLOAT_EQ(vec["z"].To<float>(), 0);
    }

    {
        using UDVector3f = UserData<Vector3f>;
        auto vec = UDVector3f::Make(l, 1, 2, 3);

        ASSERT_TRUE(vec.IsUserData());
        ASSERT_TRUE(vec.Is<UDVector3f>());
        ASSERT_TRUE(vec["x"].Is<float>());
        ASSERT_FLOAT_EQ(vec["x"].To<float>(), 1);
        ASSERT_TRUE(vec["y"].Is<float>());
        ASSERT_FLOAT_EQ(vec["y"].To<float>(), 2);
        ASSERT_TRUE(vec["z"].Is<float>());
        ASSERT_FLOAT_EQ(vec["z"].To<float>(), 3);
        ASSERT_TRUE(vec["w"].Is<void>());
        {
            Vector3f* v = vec.To<UDVector3f>();
            ASSERT_FLOAT_EQ(v->x, 1);
            ASSERT_FLOAT_EQ(v->y, 2);
            ASSERT_FLOAT_EQ(v->z, 3);
        }
        {
            Vector3f v = vec.To<UDVector3f>();
            ASSERT_FLOAT_EQ(v.x, 1);
            ASSERT_FLOAT_EQ(v.y, 2);
            ASSERT_FLOAT_EQ(v.z, 3);
        }
        {
            auto v = vec.To<UDVector3f>();
            ASSERT_FLOAT_EQ(v->x, 1);
            ASSERT_FLOAT_EQ(v->y, 2);
            ASSERT_FLOAT_EQ(v->z, 3);
        }
    }
}

struct Vector3f
{
    float x, y, z;

    Vector3f(float x, float y, float z) :x(x), y(y), z(z)
    {

    }
    Vector3f() :Vector3f(0, 0, 0) {}

    float Length()const
    {
        return sqrtf(x * x + y * y + z * z);
    }
};

TEST_F(UserDataTests, APropertyTests)
{
    using namespace LTL;


    Class<Vector3f>(l, "Vector")
        .AddConstructor<Default<float>, Default<float>, Default<float>>()
        .Add("x", AProperty<&Vector3f::x>{})
        .Add("y", AProperty<&Vector3f::y>{})
        .Add("z", AProperty<&Vector3f::z>{})
        .Add("Length", Method<&Vector3f::Length>{})
        .AddGetter("length", CFunction<&Vector3f::Length, UserData<Vector3f>>{})
        ;
    {

        Run("v = Vector(1,2,3)  "
            "result = v.x   "
        );
        ASSERT_TRUE(Result().Is<float>());
        ASSERT_FLOAT_EQ(Result().To<float>(), 1.f);
        Run(
            "result = v.y   "
        );
        ASSERT_TRUE(Result().Is<float>());
        ASSERT_FLOAT_EQ(Result().To<float>(), 2.f);
        Run(
            "result = v.z   "
        );
        ASSERT_TRUE(Result().Is<float>());
        ASSERT_FLOAT_EQ(Result().To<float>(), 3.f);
        {
            Run("v.x = 2        "
                "result = v.y   "
            );
            ASSERT_TRUE(Result().Is<float>());
            ASSERT_FLOAT_EQ(Result().To<float>(), 2.f);
            Run("v.y = 4        "
                "result = v.y   "
            );
            ASSERT_TRUE(Result().Is<float>());
            ASSERT_FLOAT_EQ(Result().To<float>(), 4.f);
            Run("v.z = 8        "
                "result = v.z   "
            );
            ASSERT_TRUE(Result().Is<float>());
            ASSERT_FLOAT_EQ(Result().To<float>(), 8.f);
        }
        {
            ASSERT_THROW(Run("v.w = 2"), Exception);
            Run("result = v.w   "
            );
            ASSERT_TRUE(Result().Is<void>());
        }
        {
            Run("v = Vector(1,2,3)  "
                "result = v:Length()  "
            );
            ASSERT_TRUE(Result().Is<float>());
            ASSERT_FLOAT_EQ(Result().To<float>(), Vector3f(1, 2, 3).Length());
        }
        {
            Run("v = Vector(1,2,3)  "
                "result = v.length  "
            );
            ASSERT_TRUE(Result().Is<float>());
            ASSERT_FLOAT_EQ(Result().To<float>(), Vector3f(1, 2, 3).Length());
        }
    }
    {
        using UDVector3f = UserData<Vector3f>;
        auto vec = UDVector3f::Make(l);

        ASSERT_TRUE(vec.IsUserData());
        ASSERT_TRUE(vec.Is<UDVector3f>());
        ASSERT_TRUE(vec["x"].Is<float>());
        ASSERT_FLOAT_EQ(vec["x"].To<float>(), 0);
        ASSERT_TRUE(vec["y"].Is<float>());
        ASSERT_FLOAT_EQ(vec["y"].To<float>(), 0);
        ASSERT_TRUE(vec["z"].Is<float>());
        ASSERT_FLOAT_EQ(vec["z"].To<float>(), 0);
    }

    {
        using UDVector3f = UserData<Vector3f>;
        auto vec = UDVector3f::Make(l, 1, 2, 3);

        ASSERT_TRUE(vec.IsUserData());
        ASSERT_TRUE(vec.Is<UDVector3f>());
        ASSERT_TRUE(vec["x"].Is<float>());
        ASSERT_FLOAT_EQ(vec["x"].To<float>(), 1);
        ASSERT_TRUE(vec["y"].Is<float>());
        ASSERT_FLOAT_EQ(vec["y"].To<float>(), 2);
        ASSERT_TRUE(vec["z"].Is<float>());
        ASSERT_FLOAT_EQ(vec["z"].To<float>(), 3);
        ASSERT_TRUE(vec["w"].Is<void>());
        {
            Vector3f* v = vec.To<UDVector3f>();
            ASSERT_FLOAT_EQ(v->x, 1);
            ASSERT_FLOAT_EQ(v->y, 2);
            ASSERT_FLOAT_EQ(v->z, 3);
        }
        {
            Vector3f v = vec.To<UDVector3f>();
            ASSERT_FLOAT_EQ(v.x, 1);
            ASSERT_FLOAT_EQ(v.y, 2);
            ASSERT_FLOAT_EQ(v.z, 3);
        }
        {
            auto v = vec.To<UDVector3f>();
            ASSERT_FLOAT_EQ(v->x, 1);
            ASSERT_FLOAT_EQ(v->y, 2);
            ASSERT_FLOAT_EQ(v->z, 3);
        }
    }
}
#pragma endregion

#pragma region  LuaStateTests
struct StateTests : TestBase
{

};


TEST_F(StateTests, UpvalueTest)
{
    using namespace LTL;

    constexpr auto func = +[](int a, CState* s, int b)->int
        {
            int c = s->GetGlobal<int>("globalValue");
            return a + b + c;
        };

    Run("globalValue = 4 ");
    RegisterClosure(l, "Func", CFunction<func, Upvalue<int>, CState*, Upvalue<int>>::Function, 1, 2);
    Run("result = Func()");

    ASSERT_EQ(1 + 2 + 4, Result().To<int>());
}


TEST_F(StateTests, AllocTest)
{
    using namespace LTL;

    State<OpNewAllocator> s;
    s.Run("result = 4");
    ASSERT_TRUE(GRefObject::Global(s, "result").Is<int>());
    ASSERT_EQ(GRefObject::Global(s, "result").To<int>(), 4);

    ASSERT_TRUE(s.GetGlobal("result").Is<int>());
    ASSERT_EQ(s.GetGlobal("result").To<int>(), 4);

}
#pragma endregion

#pragma region StackObjectViewTests
struct StackObjectViewTest : TestBase
{

};


TEST_F(StackObjectViewTest, Tests)
{

    const auto AssertEmptyStack = [&]()
        {
            ASSERT_TRUE(lua_gettop(l) == 0);
        };

    using namespace LTL;
    using namespace std;
    {
        const StackRestorer rst{ l };
        lua_pushnumber(l, 1);
        StackObjectView so{ l };
        ASSERT_TRUE(so.Is<float>());
        ASSERT_TRUE(so.Is<Type::Number>());
    }
    AssertEmptyStack();
    {
        const StackRestorer rst{ l };
        PushValue(l, 1);
        StackObjectView so{ l };
        ASSERT_TRUE(so.Is<int>());
        ASSERT_TRUE(so.Is<Type::Number>());
        ASSERT_EQ(so.To<int>(), 1);
    }
    AssertEmptyStack();
    {
        const StackRestorer rst{ l };
        PushValue(l, "Hello");
        StackObjectView so{ l };
        ASSERT_TRUE(so.Is<const char*>());
        ASSERT_TRUE(so.Is<Type::String>());
        ASSERT_STREQ(so.To<const char*>(), "Hello");
    }
    AssertEmptyStack();

    {
        const StackRestorer rst{ l };
        PushValue(l, string_view{ "Hello" });
        StackObjectView so{ l };
        ASSERT_TRUE(so.Is<const char*>());
        ASSERT_TRUE(so.Is<Type::String>());
        ASSERT_TRUE(so.Is<string_view>());
        ASSERT_TRUE(so.Is<string>());
        ASSERT_STREQ(so.To<const char*>(), "Hello");
        ASSERT_EQ(so.To<string_view>(), string_view{ "Hello" });
        ASSERT_EQ(so.To<string>(), string{ "Hello" });
    }
    AssertEmptyStack();
    {
        const StackRestorer rst{ l };
        PushValue(l, false);
        StackObjectView so{ l };
        ASSERT_TRUE(so.Is<bool>());
        ASSERT_TRUE(so.Is<Type::Boolean>());
        ASSERT_EQ(so.To<bool>(), false);
    }
    AssertEmptyStack();

    {
        const StackRestorer rst{ l };
        PushValue(l, 1);
        StackObjectView s1{ l };
        PushValue(l, 2);
        StackObjectView s2{ l };
        ASSERT_FALSE(s1 == s2);
        ASSERT_TRUE(s1 != s2);
        ASSERT_TRUE(s1 == 1);
        ASSERT_TRUE(s2 == 2);


    }
    AssertEmptyStack();
    {
        const StackRestorer rst{ l };
        Run("a = {b = 3}");
        lua_getglobal(l, "a");
        StackObjectView a{ l };
        auto b = a.Get("b");
        ASSERT_TRUE(b.Is<int>());
        ASSERT_TRUE(b.Is<Type::Number>());
        ASSERT_EQ(b.To<int>(), 3);
    }
    AssertEmptyStack();

    {
        const StackRestorer rst{ l };
        Run("a = {b = 3}");
        lua_getglobal(l, "a");
        StackObjectView a{ l };
        PushValue(l, "b");
        StackObjectView key{ l };
        auto b = a.Get(key);
        ASSERT_TRUE(b.Is<int>());
        ASSERT_EQ(b.To<int>(), 3);
    }
    AssertEmptyStack();

    {
        const StackRestorer rst{ l };
        Run("a = {}");
        lua_getglobal(l, "a");
        StackObjectView a{ l };
        PushValue(l, "b");
        StackObjectView key{ l };
        auto b1 = a.Get(key);
        ASSERT_TRUE(b1.Is<void>());
        ASSERT_TRUE(b1.Is<Type::Nil>());
        a.Set("b", 3);
        auto b2 = a.Get(key);
        ASSERT_TRUE(b2.Is<int>());
        ASSERT_EQ(b2.To<int>(), 3);
    }
    AssertEmptyStack();

    {
        const StackRestorer rst{ l };
        Run("a = {}");
        lua_getglobal(l, "a");
        StackObjectView a{ l };
        PushValue(l, "b");
        StackObjectView key{ l };
        auto b1 = a.Get(key);
        ASSERT_TRUE(b1.Is<void>());
        a.Set("b", 3);
        auto b2 = a.Get<int>(key);
        ASSERT_EQ(b2, 3);
    }
    AssertEmptyStack();
}



TEST_F(StackObjectViewTest, TestStack)
{
    using namespace LTL;
    using namespace std;
#define CheckStackChange(i, block)          \
    {                                       \
        int top = lua_gettop(l);            \
        block;                              \
        int new_top = lua_gettop(l);        \
        ASSERT_EQ(new_top, top + (i))  << "Stack has changed by " << (new_top - top) << " but expected " << i; \
        lua_settop(l, top);                 \
    }

    { // Basic stack ops
        const StackRestorer rst{ l };
        PushValue(l, 1);

        CheckStackChange(0, {
                StackObjectView s{ l };

                ASSERT_EQ(s.GetState(), l);
                ASSERT_EQ(s.GetIndex(), lua_gettop(l));
            });
        CheckStackChange(1, {
                StackObjectView s{ l };
                s.Push();
                ASSERT_TRUE(lua_isinteger(l, -1));
                ASSERT_EQ(lua_tonumber(l, -1), 1);
            });
        CheckStackChange(0, {
                StackObjectView s{ l };
                ASSERT_TRUE(s.Is<int>());
                ASSERT_TRUE(s.Is<Type::Number>());
                ASSERT_EQ(s.To<int>(), 1);
            });
        CheckStackChange(0, {
                StackObjectView s{ l };
                ASSERT_EQ(s, 1);
                ASSERT_NE(s, 2);
                ASSERT_NE(s, "a");
                ASSERT_NE(s, 1.5);
                ASSERT_NE(s, false);
                ASSERT_NE(s, true);
                ASSERT_NE(s, nullptr);
            });
        CheckStackChange(1, {
                StackObjectView s1{ l };
                s1.Push();
                StackObjectView s2{ l };
                ASSERT_EQ(s1, s2);
                ASSERT_EQ(s2, 1);
            });
        CheckStackChange(1, {
                StackObjectView s1{ l };
                s1.Push();
                StackObjectView s2{ l };
                ASSERT_TRUE(s1.RawEqual(s2));
                ASSERT_TRUE(s1.RawEqual(1));
                ASSERT_TRUE(s2.RawEqual(1));

                ASSERT_FALSE(s2.RawEqual(2));
                ASSERT_FALSE(s2.RawEqual("1"));
                ASSERT_FALSE(s2.RawEqual(1.5));
                ASSERT_FALSE(s2.RawEqual(false));
                ASSERT_FALSE(s2.RawEqual(true));
                ASSERT_FALSE(s2.RawEqual(nullptr));
            });
        CheckStackChange(1, {
                StackObjectView s1{ l };
                s1.Push();
                StackObjectView s2{ l };
                ASSERT_TRUE(s1 >= s2);
                ASSERT_TRUE(s1 <= s2);
                ASSERT_FALSE(s1 > s2);
                ASSERT_FALSE(s1 < s2);

                ASSERT_TRUE(s1 >= 1);
                ASSERT_TRUE(s1 >= 0);
                ASSERT_FALSE(s1 >= 2);

                ASSERT_TRUE(s1 < 2);
                ASSERT_FALSE(s1 < 1);

                ASSERT_TRUE(s1 <= 1);
                ASSERT_TRUE(s1 <= 2);
                ASSERT_FALSE(s1 <= 0);

                ASSERT_TRUE(s1 > 0);
                ASSERT_FALSE(s1 > 2);
            });
        CheckStackChange(1, {
                StackObjectView s1{ l };
                auto s2 = s1.GetMetaTable();
                ASSERT_TRUE(s2.Is<void>());
            });
    }
    {// table ops
        const StackRestorer rst{ l };
        Run(R"===(
                local t = {
                    __eq = function(self, other)
                        return self.n == other
                    end,
                    __lt = function(self, other)
                        return self.n < other
                    end,
                    __le = function(self, other)
                        return self.n <= other
                    end,
                    __len = function(self) return self.n end,
                }
                t.__index = t
                result = setmetatable({ n = 4 }, t)
                )===");
        lua_getglobal(l, "result");
        CheckStackChange(0, {
                StackObjectView s1{ l };
                ASSERT_TRUE(s1.Is<Type::Table>());
            });
        CheckStackChange(1, {
                StackObjectView s1{ l };
                s1.Push();
                StackObjectView s2{ l };
                ASSERT_TRUE(s1 == s2);
                ASSERT_FALSE(s1 != s2);
                ASSERT_FALSE(s1 == nullptr);
                ASSERT_TRUE(s1 != nullptr);
                ASSERT_TRUE(s1.RawEqual(s2));
            });
        CheckStackChange(0, {
                StackObjectView s1{ l };
                ASSERT_TRUE(s1 > 3);
                ASSERT_TRUE(s1 < 5);
                ASSERT_TRUE(s1 >= 4);
                ASSERT_TRUE(s1 <= 4);
                ASSERT_FALSE(s1 < 4);
                ASSERT_FALSE(s1 > 4);
                ASSERT_FALSE(s1 > 5);
                ASSERT_FALSE(s1 < 3);
            });
        CheckStackChange(0, {
                StackObjectView s1{ l };
                ASSERT_EQ(s1.RawLen(), 0);
                ASSERT_EQ(s1.Len<int>(), 4);
            });
        CheckStackChange(1, {
                StackObjectView s1{ l };
                auto s2 = s1.GetMetaTable();
                ASSERT_TRUE(s2.Is<Type::Table>());
            });
        CheckStackChange(2, {
                StackObjectView s1{ l };
                auto s3 = s1.Get("__index");
                ASSERT_FALSE(s3 == nullptr);
                auto s4 = s1.RawGet("__index");
                ASSERT_TRUE(s4 == nullptr);
            });
        CheckStackChange(3, {
                StackObjectView s1{ l };
                auto s2 = s1.GetMetaTable();
                auto s3 = s2.Get("__index");
                ASSERT_FALSE(s3 == nullptr);
                auto s4 = s2.RawGet("__index");
                ASSERT_FALSE(s4 == nullptr);
                ASSERT_TRUE(s4 == s3);
            });
    }
    { // get and set
        const StackRestorer rst{ l };
        lua_createtable(l, 0, 0);
        CheckStackChange(0, {
                StackObjectView s1{ l };
                s1.RawSetI(1, 10);
            });
        CheckStackChange(1, {
                StackObjectView s1{ l };
                auto s2 = s1.RawGetI(1);
                ASSERT_TRUE(s2.Is<int>());
                ASSERT_EQ(s2, 10);
            });
        CheckStackChange(0, {
                StackObjectView s1{ l };
                s1.SetI(1, 20);
            });
        CheckStackChange(1, {
                StackObjectView s1{ l };
                auto s2 = s1.GetI(1);
                ASSERT_TRUE(s2.Is<int>());
                ASSERT_EQ(s2, 20);
            });
        CheckStackChange(0, {
                StackObjectView s1{ l };
                s1.RawSet("a", 30);
            });
        CheckStackChange(1, {
                StackObjectView s1{ l };
                auto s2 = s1.RawGet("a");
                ASSERT_TRUE(s2.Is<int>());
                ASSERT_EQ(s2, 30);
            });
        CheckStackChange(0, {
                StackObjectView s1{ l };
                s1.Set("b", 40);
            });
        CheckStackChange(1, {
                StackObjectView s1{ l };
                auto s2 = s1.Get("b");
                ASSERT_TRUE(s2.Is<int>());
                ASSERT_EQ(s2, 40);
            });

    }

#undef CheckStackChange
}


TEST_F(StackObjectViewTest, TestAsArg)
{
    const auto AssertEmptyStack = [&]()
        {
            ASSERT_TRUE(lua_gettop(l) == 0);
        };
    using namespace LTL;
    using namespace std;

    constexpr auto stack_f = +[](StackObjectView v) {
        v.Set("hello", "world");
        return v;
        };


    {
        RegisterFunction(l, "Test", CFunction<stack_f, StackObjectView>::Function);
        Run("result = Test({})");
        ASSERT_TRUE(Result().IsTable());
        ASSERT_TRUE(Result()["hello"].Is<string>());
        ASSERT_EQ(Result()["hello"].To<string>(), "world");
    }
}

#pragma endregion

#pragma region TypeMatchingTests
struct TypeMatchingTests : TestBase
{

};

TEST_F(TypeMatchingTests, MatchArgumentTypesTests)

{
    using namespace LTL;
#define ASSERT_MATCHES(s, res)\
    Run("result = Match(" s ")"); \
    ASSERT_TRUE(Result().Is<bool>()); \
    ASSERT_EQ(Result().To<bool>(), res)

    {
        using Match = typename MatchArgumentTypes<int, int, int>;
        ASSERT_EQ(Match::max_arg_count, 3);
        ASSERT_EQ(Match::min_arg_count, 3);
        RegisterFunction(l, "Match", Match::Function);

        ASSERT_MATCHES("1,2,3", true);
        ASSERT_MATCHES("1,0,3", true);
        ASSERT_MATCHES("1.2,2,3", false);
        ASSERT_MATCHES("true,2,3", false);
        ASSERT_MATCHES("1,false,3", false);
        ASSERT_MATCHES("1,'hello',3", false);
        ASSERT_MATCHES("1,2,3,4,5", false);
        ASSERT_MATCHES("1,2,3,4", false);
        ASSERT_MATCHES("1", false);
        ASSERT_MATCHES("1,2", false);
        ASSERT_MATCHES("nil,1,2", false);
        ASSERT_MATCHES("nil, nil, 1", false);
        ASSERT_MATCHES("nil, nil, nil", false);
        ASSERT_MATCHES("1,2,3,4,5", false);
        ASSERT_MATCHES("", false);
    }
    {
        using Match = typename MatchArgumentTypes<Default<int>, Default<int>, Default<int>>;
        ASSERT_EQ(Match::max_arg_count, 3);
        ASSERT_EQ(Match::min_arg_count, 0);
        RegisterFunction(l, "Match", Match::Function);

        ASSERT_MATCHES("1,2,3", true);
        ASSERT_MATCHES("1,0,3", true);
        ASSERT_MATCHES("1.2,2,3", false);
        ASSERT_MATCHES("true,2,3", false);
        ASSERT_MATCHES("1,false,3", false);
        ASSERT_MATCHES("1,'hello',3", false);
        ASSERT_MATCHES("1,2,3,4,5", false);
        ASSERT_MATCHES("1,2,3,4", false);
        ASSERT_MATCHES("1", true);
        ASSERT_MATCHES("1,2", true);
        ASSERT_MATCHES("nil,1,2", true);
        ASSERT_MATCHES("nil, nil, 1", true);
        ASSERT_MATCHES("nil, nil, nil", true);
        ASSERT_MATCHES("", true);
    }

    {
        using Match = typename MatchArgumentTypes<Default<int>, Default<int>, Default<int>, Upvalue<int>>;
        ASSERT_EQ(Match::max_arg_count, 3);
        ASSERT_EQ(Match::min_arg_count, 0);
        RegisterClosure(l, "Match", Match::Function, 1);

        ASSERT_MATCHES("1,2,3", true);
        ASSERT_MATCHES("1,0,3", true);
        ASSERT_MATCHES("1.2,2,3", false);
        ASSERT_MATCHES("true,2,3", false);
        ASSERT_MATCHES("1,false,3", false);
        ASSERT_MATCHES("1,'hello',3", false);
        ASSERT_MATCHES("1,2,3,4,5", false);
        ASSERT_MATCHES("1,2,3,4", false);
        ASSERT_MATCHES("1", true);
        ASSERT_MATCHES("1,2", true);
        ASSERT_MATCHES("nil,1,2", true);
        ASSERT_MATCHES("nil, nil, 1", true);
        ASSERT_MATCHES("nil, nil, nil", true);
        ASSERT_MATCHES("", true);
    }

    {
        using Match = typename MatchArgumentTypes<Default<int>, lua_State*, Default<int>, Default<int>, Upvalue<int>>;
        ASSERT_EQ(Match::max_arg_count, 3);
        ASSERT_EQ(Match::min_arg_count, 0);
        RegisterClosure(l, "Match", Match::Function, 1);

        ASSERT_MATCHES("1,2,3", true);
        ASSERT_MATCHES("1,0,3", true);
        ASSERT_MATCHES("1.2,2,3", false);
        ASSERT_MATCHES("true,2,3", false);
        ASSERT_MATCHES("1,false,3", false);
        ASSERT_MATCHES("1,'hello',3", false);
        ASSERT_MATCHES("1,2,3,4,5", false);
        ASSERT_MATCHES("1,2,3,4", false);
        ASSERT_MATCHES("1", true);
        ASSERT_MATCHES("1,2", true);
        ASSERT_MATCHES("nil,1,2", true);
        ASSERT_MATCHES("nil, nil, 1", true);
        ASSERT_MATCHES("nil, nil, nil", true);
        ASSERT_MATCHES("", true);
    }

    {
        using Match = typename MatchArgumentTypes<int, lua_State*, Default<int>, Default<int>, Upvalue<int>>;
        ASSERT_EQ(Match::max_arg_count, 3);
        ASSERT_EQ(Match::min_arg_count, 1);
        RegisterClosure(l, "Match", Match::Function, 1);

        ASSERT_MATCHES("1,2,3", true);
        ASSERT_MATCHES("1,0,3", true);
        ASSERT_MATCHES("1.2,2,3", false);
        ASSERT_MATCHES("true,2,3", false);
        ASSERT_MATCHES("1,false,3", false);
        ASSERT_MATCHES("1,'hello',3", false);
        ASSERT_MATCHES("1,2,3,4,5", false);
        ASSERT_MATCHES("1,2,3,4", false);
        ASSERT_MATCHES("1", true);
        ASSERT_MATCHES("1,2", true);
        ASSERT_MATCHES("nil,1,2", false);
        ASSERT_MATCHES("nil, nil, 1", false);
        ASSERT_MATCHES("nil, nil, nil", false);
        ASSERT_MATCHES("", false);
    }
    {
        using Match = typename MatchArgumentTypes<Default<int>, lua_State*, int, Default<int>, Upvalue<int>>;
        ASSERT_EQ(Match::max_arg_count, 3);
        ASSERT_EQ(Match::min_arg_count, 2);
        RegisterClosure(l, "Match", Match::Function, 1);

        ASSERT_MATCHES("1,2,3", true);
        ASSERT_MATCHES("1,0,3", true);
        ASSERT_MATCHES("1.2,2,3", false);
        ASSERT_MATCHES("true,2,3", false);
        ASSERT_MATCHES("1,false,3", false);
        ASSERT_MATCHES("1,'hello',3", false);
        ASSERT_MATCHES("1,2,3,4,5", false);
        ASSERT_MATCHES("1,2,3,4", false);
        ASSERT_MATCHES("1", false);
        ASSERT_MATCHES("1,2", true);
        ASSERT_MATCHES("nil,1,2", true);
        ASSERT_MATCHES("nil, nil, 1", false);
        ASSERT_MATCHES("nil, nil, nil", false);
        ASSERT_MATCHES("", false);
    }

    {
        using Match = typename MatchArgumentTypes<Default<int>, lua_State*, int, int, Upvalue<int>>;
        ASSERT_EQ(Match::max_arg_count, 3);
        ASSERT_EQ(Match::min_arg_count, 3);
        RegisterClosure(l, "Match", Match::Function, 1);

        ASSERT_MATCHES("1,2,3", true);
        ASSERT_MATCHES("1,0,3", true);
        ASSERT_MATCHES("1.2,2,3", false);
        ASSERT_MATCHES("true,2,3", false);
        ASSERT_MATCHES("1,false,3", false);
        ASSERT_MATCHES("1,'hello',3", false);
        ASSERT_MATCHES("1,2,3,4,5", false);
        ASSERT_MATCHES("1,2,3,4", false);
        ASSERT_MATCHES("1", false);
        ASSERT_MATCHES("1,2", false);
        ASSERT_MATCHES("nil,1,2", true);
        ASSERT_MATCHES("nil, nil, 1", false);
        ASSERT_MATCHES("nil, nil, nil", false);
        ASSERT_MATCHES("", false);
    }

    {
        using Match = typename MatchArgumentTypes<Default<float>, lua_State*, float, float, Upvalue<float>>;
        ASSERT_EQ(Match::max_arg_count, 3);
        ASSERT_EQ(Match::min_arg_count, 3);
        RegisterClosure(l, "Match", Match::Function, 1);

        ASSERT_MATCHES("1,2,3", true);
        ASSERT_MATCHES("1,0,3", true);
        ASSERT_MATCHES("1.2,2,3", true);
        ASSERT_MATCHES("true,2,3", false);
        ASSERT_MATCHES("1,false,3", false);
        ASSERT_MATCHES("1,'hello',3", false);
        ASSERT_MATCHES("1,2,3,4,5", false);
        ASSERT_MATCHES("1,2,3,4", false);
        ASSERT_MATCHES("1", false);
        ASSERT_MATCHES("1,2", false);
        ASSERT_MATCHES("nil,1,2", true);
        ASSERT_MATCHES("nil, nil, 1", false);
        ASSERT_MATCHES("nil, nil, nil", false);
        ASSERT_MATCHES("", false);
    }

#undef ASSERT_MATCHES
}


TEST_F(TypeMatchingTests, UpvalueMatcingTests)
{
    using namespace LTL;
    using namespace LTL::FuncUtility;
#define ASSERT_TRUE_TYPE(...)  ASSERT_TRUE(__VA_ARGS__::value)
#define ASSERT_FALSE_TYPE(...)  ASSERT_FALSE(__VA_ARGS__::value)
    {
        ASSERT_TRUE_TYPE(MatchUpvalues<int, float, bool>::Matches<Upvalue<int>, int, float, bool, Upvalue<float>, Upvalue<bool>>);
        ASSERT_TRUE_TYPE(MatchUpvalues<int, float, bool>::Matches<Upvalue<int>, Upvalue<float>, Upvalue<bool>>);
        ASSERT_TRUE_TYPE(MatchUpvalues<int, float, bool>::Matches<Upvalue<int>, float, bool, Upvalue<float>, Upvalue<bool>>);
        ASSERT_TRUE_TYPE(MatchUpvalues<int, float, bool>::Matches<Upvalue<int>, Upvalue<float>, Upvalue<bool>, int, float, bool>);
        ASSERT_TRUE_TYPE(MatchUpvalues<int, float, bool>::Matches<Upvalue<int>, int, Upvalue<float>, Upvalue<bool>, float, bool>);
        ASSERT_TRUE_TYPE(MatchUpvalues<int, float, bool>::Matches<Upvalue<int>, Upvalue<float>, int, float, bool, Upvalue<bool>>);
        ASSERT_TRUE_TYPE(MatchUpvalues<int, float, bool>::Matches<int, float, bool, Upvalue<int>, Upvalue<float>, Upvalue<bool>>);
        ASSERT_TRUE_TYPE(MatchUpvalues<int, float, bool>::Matches<int, Upvalue<int>, float, Upvalue<float>, bool, Upvalue<bool>>);

        ASSERT_FALSE_TYPE(MatchUpvalues<int, float, bool>::Matches<int, float, bool>);
        ASSERT_FALSE_TYPE(MatchUpvalues<int, float, bool>::Matches<int, float, bool, Upvalue<int>>);
        ASSERT_FALSE_TYPE(MatchUpvalues<int, float, bool>::Matches<int, float, bool, Upvalue<int>, Upvalue<float>>);
        ASSERT_FALSE_TYPE(MatchUpvalues<int, float, bool>::Matches<Upvalue<float>, int, float, bool, Upvalue<int>>);
        ASSERT_FALSE_TYPE(MatchUpvalues<int, float, bool>::Matches<Upvalue<float>, Upvalue<int>, Upvalue<float>, Upvalue<bool>>);
        ASSERT_FALSE_TYPE(MatchUpvalues<int, float, bool>::Matches<>);
        ASSERT_FALSE_TYPE(MatchUpvalues<int, float, bool>::Matches<Upvalue<int>, Upvalue<float>, Upvalue<int>>);
        ASSERT_FALSE_TYPE(MatchUpvalues<int, float, bool>::Matches<Upvalue<int>>);
        ASSERT_FALSE_TYPE(MatchUpvalues<int, float, bool>::Matches<int, float, Upvalue<bool>>);

        ASSERT_TRUE_TYPE(MatchUpvalues<>::Matches<>);
        ASSERT_TRUE_TYPE(MatchUpvalues<>::Matches<int, float, bool>);
        ASSERT_FALSE_TYPE(MatchUpvalues<>::Matches<int, float, bool, Upvalue<int>>);
        ASSERT_FALSE_TYPE(MatchUpvalues<>::Matches<int, float, bool, Upvalue<int>, Upvalue<float>>);
        ASSERT_FALSE_TYPE(MatchUpvalues<>::Matches<Upvalue<float>, int, float, bool, Upvalue<int>>);
        ASSERT_FALSE_TYPE(MatchUpvalues<>::Matches<Upvalue<float>, Upvalue<int>, Upvalue<float>, Upvalue<bool>>);
        ASSERT_FALSE_TYPE(MatchUpvalues<>::Matches<Upvalue<int>, Upvalue<float>, Upvalue<int>>);
        ASSERT_FALSE_TYPE(MatchUpvalues<>::Matches<Upvalue<int>>);
        ASSERT_FALSE_TYPE(MatchUpvalues<>::Matches<int, float, Upvalue<bool>>);
    }

    {

    }
#undef ASSERT_TRUE_TYPE
#undef ASSERT_FALSE_TYPE
}
#pragma endregion

#pragma region STL Containers Tests
struct STDContainersTests : TestBase
{

};

TEST_F(STDContainersTests, VectorTests)
{
    using namespace LTL;
    using namespace std;

    {
        StackRestorer rst{ l };
        using TestType = vector<int>;
        TestType v1 = { 1,2,3,4,54 };
        PushValue(l, v1);
        auto  v2 = GetValue<TestType>(l, -1);
        ASSERT_EQ(v1, v2);
    }


}

TEST_F(STDContainersTests, UnorderedMapTests)
{
    using namespace LTL;
    using namespace std;

    {
        StackRestorer rst{ l };
        using TestType = unordered_map<int, int>;
        TestType v1 = {
            {1,2},
            {0, 4},
            {4,3},
            {7,-1}
        };
        PushValue(l, v1);
        auto  v2 = GetValue<TestType>(l, -1);
        ASSERT_EQ(v1, v2);
    }

    {
        StackRestorer rst{ l };
        using TestType = unordered_map<string, int>;
        TestType v1 = {
            {"a",2},
            {"b", 4},
            {"bcd",3},
            {"s",-1}
        };
        PushValue(l, v1);
        auto  v2 = GetValue<TestType>(l, -1);
        ASSERT_EQ(v1, v2);
    }

    {
        StackRestorer rst{ l };
        using TestType = unordered_map<string, GRefObject>;
        TestType v1 = {
            {"a",{l,2}},
            {"b",{l,3}},
            {"c",{l,4}},
            {"d",{l,5}},
        };
        PushValue(l, v1);
        auto  v2 = GetValue<TestType>(l, -1);
        ASSERT_EQ(v1, v2);

    }


}


struct MultReturnTests :TestBase
{

};


TEST_F(MultReturnTests, Tests)
{
    using namespace LTL;
    using namespace std;
    {

        constexpr auto f = +[]() -> MultReturn<int, int>
            {
                return { 1,2 };
            };

        RegisterFunction(l, "f", CFunction<f>::Function);

        Run("a,b = f()");

        ASSERT_TRUE(GRefObject::Global(l, "a").Is<int>());
        ASSERT_EQ(GRefObject::Global(l, "a").To<int>(), 1);
        ASSERT_TRUE(GRefObject::Global(l, "b").Is<int>());
        ASSERT_EQ(GRefObject::Global(l, "b").To<int>(), 2);

    }

}
struct OptionalTests :TestBase
{

};


TEST_F(OptionalTests, Tests)
{
    using namespace LTL;
    using namespace std;
    {
        constexpr auto f = +[](const optional<int>& value) -> bool
            {
                return value.has_value();
            };


        RegisterFunction(l, "f", CFunction<f, optional<int>>::Function);
        {
            Run("result = f()");
            ASSERT_FALSE(Result().To<bool>());
        }
        {
            Run("result = f(1)");
            ASSERT_TRUE(Result().To<bool>());
        }
        {
            Run("result = f(null)");
            ASSERT_FALSE(Result().To<bool>());
        }
        {
            ASSERT_THROW(Run("result = f('a')"), Exception);
        }
        RegisterFunction(l, "f", CFunction<&optional<int>::has_value, optional<int>>::Function);
        {
            Run("result = f()");
            ASSERT_FALSE(Result().To<bool>());
        }
        {
            Run("result = f(1)");
            ASSERT_TRUE(Result().To<bool>());
        }
        {
            Run("result = f(null)");
            ASSERT_FALSE(Result().To<bool>());
        }
        {
            ASSERT_THROW(Run("result = f('a')"), Exception);

        }

    }

}
#pragma endregion

#pragma region Pcall tests

struct TestPcall :TestBase
{

};


TEST_F(TestPcall, Tests)
{
    using namespace LTL;
    using namespace std;
    {
        State s;
        s.OpenLibs();
        s.Run(
            "function f() end"
        );
        ASSERT_EQ(s.PCall("f"), PCallResult::Ok);
        ASSERT_TRUE(s.PCall("f").IsOk());
        ASSERT_TRUE(s.PCall("f"));
    }

    {
        State s;
        s.OpenLibs();
        s.Run(
            "function f() error() end"
        );
        ASSERT_EQ(s.PCall("f"), PCallResult::ERRRUN);
        ASSERT_FALSE(s.PCall("f").IsOk());
        ASSERT_FALSE(s.PCall("f"));
    }

    /*  {// later
          State s;
          s.OpenLibs();

          constexpr auto f = +[](lua_State* l) ->int {
              StackObjectView i{ l ,1 };
              lua_createtable(l, 0, i.To<int>());
              return 1;
              };

          s.AddFunction("maketbl", f);

          s.Run(
              "function f() "
              " local t = {} "
              " for i = 1, 10000000 do "
              " local tinner = {} "
              " table.insert(t, maketbl(2000000000)) "
              " end"
              " end"
          );
          ASSERT_EQ(s.PCall("f"), PCallResult::ERRMEM);
          ASSERT_FALSE(s.PCall("f").IsOk());
      }*/
    {
        State s;
        s.OpenLibs();
        constexpr auto f = +[](lua_State* l) ->int {
            lua_error(l);
            return 0;
            };

        s.AddFunction("myerror", f);
        s.Run(
            "function f() myerror() end"
        );
        ASSERT_EQ(s.PCall("f"), PCallResult::ERRRUN);
        ASSERT_FALSE(s.PCall("f").IsOk());
        ASSERT_FALSE(s.PCall("f"));
    }
    {
        State s;
        s.OpenLibs();
        constexpr auto f = +[](lua_State* l) ->int {
            luaL_checkstring(l, -1);
            return 0;
            };

        s.AddFunction("myerror", f);
        s.Run(
            "function f() myerror() end"
        );
        ASSERT_EQ(s.PCall("f"), PCallResult::ERRRUN);
        ASSERT_FALSE(s.PCall("f").IsOk());
    }

}
#pragma endregion

#pragma region Memory tests
#ifdef _WIN32

// Tets memory leaks ans safety
// https://stackoverflow.com/questions/29174938/googletest-and-memory-leaks
#include <crtdbg.h>

class MemoryLeakDetector {
public:
    MemoryLeakDetector() {
        _CrtMemCheckpoint(&memState_);
    }

    ~MemoryLeakDetector() {
        _CrtMemState stateNow, stateDiff;
        _CrtMemCheckpoint(&stateNow);
        int diffResult = _CrtMemDifference(&stateDiff, &memState_, &stateNow);
        if (diffResult)
            reportFailure(stateDiff.lSizes[1]);
    }
private:
    void reportFailure(unsigned int unfreedBytes) {
        FAIL() << "Memory leak of " << unfreedBytes << " byte(s) detected.";
    }
    _CrtMemState memState_;
};

struct TestNotRegisteredUserDataClass :TestBase
{

};


struct MyMemoryClass
{
public:
    std::vector<int> v;


    MyMemoryClass(int n)
    {
        v.reserve(n);
    }
    ~MyMemoryClass() = default;

};

TEST_F(TestNotRegisteredUserDataClass, Tests)
{
    using namespace LTL;
    using namespace std;
    {

        MemoryLeakDetector leakDetector;
        State s;
        s.ThrowExceptions();
        ASSERT_THROW(s.MakeUserData<Vector3f>(1, 2, 3), Exception);
    }
    {

        MemoryLeakDetector leakDetector;
        State s;
        s.ThrowExceptions();
        ASSERT_THROW(s.MakeUserData<MyMemoryClass>(10), Exception);
    }
}
#endif // WINDOWS
#pragma endregion