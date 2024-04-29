#include "TestBase.hpp"

#pragma region RefObjectTests
struct RefObjectTests : TestBase
{

};
TEST_F(RefObjectTests, Identity)
{
    using namespace LTL;

    {
        GRefObject obj{ l };

        ASSERT_EQ(obj.GetState(), l);
        ASSERT_EQ(obj.Type(), Type::Nil);
        ASSERT_TRUE(obj.Is<Type::Nil>());
        ASSERT_TRUE(obj.IsNil());
        ASSERT_TRUE(obj == nullptr);
    }

    {
        GRefObject obj{ l , 3 };

        ASSERT_EQ(obj.Type(), Type::Number);
        ASSERT_TRUE(obj.Is<Type::Number>());
        ASSERT_TRUE(obj.Is<int>());
        ASSERT_EQ(obj.To<int>(), 3);
        ASSERT_TRUE(obj == 3);
    }

    {
        using namespace std;
        GRefObject obj{ l , "string" };

        ASSERT_EQ(obj.Type(), Type::String);
        ASSERT_TRUE(obj.Is<Type::String>());
        ASSERT_TRUE(obj.Is<const char*>());
        ASSERT_TRUE(obj.Is<string>());
        ASSERT_TRUE(obj.Is<string_view>());

        ASSERT_FALSE(obj.Is<Type::Number>());
        ASSERT_FALSE(obj.Is<Type::Nil>());
        ASSERT_FALSE(obj.Is<int>());
        ASSERT_FALSE(obj.Is<float>());
        ASSERT_FALSE(obj.Is<bool>());

        ASSERT_STREQ(obj.To<const char*>(), "string");
        ASSERT_EQ(obj.To<string>(), "string");
        ASSERT_EQ(obj.To<string_view>(), "string");

        ASSERT_TRUE(obj == "string");
    }

    {
        using namespace std;
        GRefObject obj{ l , true };

        ASSERT_EQ(obj.Type(), Type::Boolean);
        ASSERT_TRUE(obj.Is<Type::Boolean>());
        ASSERT_TRUE(obj.Is<bool>());
        ASSERT_TRUE(obj.To<bool>());
        ASSERT_TRUE(obj == true);
        ASSERT_FALSE(obj == false);
        ASSERT_FALSE(obj != true);
        ASSERT_TRUE(obj != false);
    }

    {

    }

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
    ASSERT_EQ(Result()["a"].To<int>(), 4);

    GRefObject key{ l, "a" };
    ASSERT_TRUE(Result()[key].Is<int>());
    ASSERT_EQ(Result()[key].To<int>(), 4);
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
    {
        State s;
        s.OpenLibs();
        constexpr auto f = +[](lua_State* l) ->void {
            throw exception("something wrong!");
            };

        s.Add("myerror", CFunction<f, lua_State*>{});
        s.Run(
            "function f() myerror() end"
        );
        ASSERT_EQ(s.PCall("f"), PCallResult::ERRRUN);
        ASSERT_FALSE(s.PCall("f").IsOk());
    }

}
#pragma endregion
