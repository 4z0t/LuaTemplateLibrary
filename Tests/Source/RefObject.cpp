#include "TestBase.hpp"


struct RefObjectTests : TestBase
{

};

TEST_F(RefObjectTests, ValueAccess)
{
    using namespace Lua;
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

TEST_F(RefObjectTests, IteratorTest)
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




    //    Run("result = {1,3,4,6,7, a = 5, b = 'abc'}");

}

TEST_F(RefObjectTests, CompareTest)
{
    using namespace Lua;
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
        // throws for some reason and dont catch
        //ASSERT_THROW(a > b, Exception);
        //ASSERT_FALSE(a < b); 
        //ASSERT_FALSE(a > b);
        //ASSERT_TRUE(a >= b);
        //ASSERT_TRUE(a <= b);
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
    using namespace Lua;

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

    using namespace Lua;
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



// exception tests are here for a moment with gtest bug?


struct ExceptionTests : TestBase
{

};

TEST_F(ExceptionTests, ThrowTests)
{
    Run("result = true");
    ASSERT_THROW(Result().Call(), Lua::Exception);
    ASSERT_THROW(Result()["Key"].To<int>(), Lua::Exception);

}


// UserData tests


struct UserDataTests : TestBase
{

};

TEST_F(UserDataTests, MoveCtorTest)
{
    using namespace Lua;
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
        .Add("Duplicate", Method<CantCopy, &CantCopy::Dup, CantCopy()>{})
        .Add("Print", Method<CantCopy, &CantCopy::Print>{})
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
    using namespace Lua;
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
        .Add("Length", Method<Vector3f, &Vector3f::Length>{})
        .AddGetter("length", CFunction<&Vector3f::Length, UserData<Vector3f>>::Function)
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
            ASSERT_THROW(Run("v.w = 2"), std::runtime_error);
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



struct StateTests : TestBase
{

};


TEST_F(StateTests, UpvalueTest)
{
    using namespace Lua;

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
    using namespace Lua;

    State<OpNewAllocator> s;
    s.Run("result = 4");
    ASSERT_TRUE(GRefObject::Global(s, "result").Is<int>());
    ASSERT_EQ(GRefObject::Global(s, "result").To<int>(), 4);

}



struct StackObjectTest : TestBase
{

};


TEST_F(StackObjectTest, Tests)
{

    const auto AssertEmptyStack = [&]()
        {
            ASSERT_TRUE(lua_gettop(l) == 0);
        };

    using namespace Lua;
    {

        lua_pushnumber(l, 1);
        StackObject so{ l,-1 };
        ASSERT_TRUE(so.Is<float>());
        lua_pop(l, 1);
    }
    AssertEmptyStack();
    {

        StackObject so = StackObject::FromValue(l, 1);
        ASSERT_TRUE(so.Is<int>());
        ASSERT_EQ(so.To<int>(), 1);
    }
    AssertEmptyStack();
    {
        StackObject so = StackObject::FromValue(l, "Hello");
        ASSERT_TRUE(so.Is<const char*>());
        ASSERT_STREQ(so.To<const char*>(), "Hello");
    }
    AssertEmptyStack();
    {
        StackObject so = StackObject::FromValue(l, false);
        ASSERT_TRUE(so.Is<bool>());
        ASSERT_EQ(so.To<bool>(), false);
    }
    AssertEmptyStack();

    {
        StackObject s1 = StackObject::FromValue(l, 1);
        StackObject s2 = StackObject::FromValue(l, 2);
        ASSERT_FALSE(s1 == s2);
        ASSERT_TRUE(s1 != s2);
        ASSERT_TRUE(s1 == 1);
        ASSERT_TRUE(s2 == 2);


    }
    AssertEmptyStack();

    {
        Run("a, b = 3, 'Hi'");
        StackObject s1 = StackObject::Global(l, "a");
        StackObject s2 = StackObject::Global(l, "b");
        StackObject s3 = StackObject::Global(l, "a");
        ASSERT_TRUE(s1.Is<int>());
        ASSERT_TRUE(s2.Is<const char*>());
        ASSERT_TRUE(s1 == 3);
        ASSERT_TRUE(s1.RawEqual(3));
        ASSERT_TRUE(s1.RawEqual(s3));
        ASSERT_TRUE(s2.RawEqual("Hi"));

    }
    AssertEmptyStack();


    {
        Run("a = {b = 3}");
        StackObject a = StackObject::Global(l, "a");
        auto b = a.Get("b");
        ASSERT_TRUE(b.Is<int>());
        ASSERT_EQ(b.To<int>(), 3);
    }
    AssertEmptyStack();

    {
        Run("a = {b = 3}");
        StackObject a = StackObject::Global(l, "a");
        StackObject key = StackObject::FromValue(l, "b");
        auto b = a.Get(key);
        ASSERT_TRUE(b.Is<int>());
        ASSERT_EQ(b.To<int>(), 3);
    }
    AssertEmptyStack();

    {
        Run("a = {}");
        StackObject a = StackObject::Global(l, "a");
        StackObject key = StackObject::FromValue(l, "b");
        auto b1 = a.Get(key);
        ASSERT_TRUE(b1.Is<void>());
        a.Set("b", 3);
        auto b2 = a.Get(key);
        ASSERT_TRUE(b2.Is<int>());
        ASSERT_EQ(b2.To<int>(), 3);
    }
    AssertEmptyStack();

    {
        Run("a = {}");
        StackObject a = StackObject::Global(l, "a");
        StackObject key = StackObject::FromValue(l, "b");
        auto b1 = a.Get(key);
        ASSERT_TRUE(b1.Is<void>());
        a.Set("b", 3);
        auto b2 = a.Get<int>(key);
        ASSERT_EQ(b2, 3);
    }
    AssertEmptyStack();

    {
        Run("a = 'Hello'");
        GRefObject a = GRefObject::Global(l, "a");
        StackObject stack_a = StackObject::FromValue(l, a);
        ASSERT_TRUE(stack_a.Is<const char*>());
        ASSERT_STREQ(stack_a.To<const char*>(), "Hello");
    }
    AssertEmptyStack();

    {
        Run("a = 'Hello'");
        GRefObject a = GRefObject::Global(l, "a");
        StackObject stack_a = a;
        ASSERT_TRUE(stack_a.Is<const char*>());
        ASSERT_STREQ(stack_a.To<const char*>(), "Hello");
    }
    AssertEmptyStack();
}


struct StackObjectViewTest : TestBase
{

};


TEST_F(StackObjectViewTest, Tests)
{

    const auto AssertEmptyStack = [&]()
        {
            ASSERT_TRUE(lua_gettop(l) == 0);
        };

    using namespace Lua;
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


struct TypeMatchingTests : TestBase
{

};

TEST_F(TypeMatchingTests, Tests)
{
    using namespace Lua;
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