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
        lua_rawgetp(l, LUA_REGISTRYINDEX, GetIndex());
        assert(lua_istable(l, -1));
    }

    static void AssignTable(lua_State* l)
    {
        lua_newtable(l);
        lua_rawsetp(l, LUA_REGISTRYINDEX, GetIndex());
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
        ;
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
}