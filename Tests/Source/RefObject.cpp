#include "TestBase.hpp"


struct RefObjectTests : TestBase
{

};

TEST_F(RefObjectTests, ValueAccess)
{
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
        lua_pushlightuserdata(l, GetIndex());
        lua_gettable(l, LUA_REGISTRYINDEX);
        assert(lua_istable(l, -1));
    }

    static void AssignTable(lua_State* l)
    {
        lua_pushlightuserdata(l, GetIndex());
        lua_newtable(l);
        lua_settable(l, LUA_REGISTRYINDEX);
        assigned = true;
    }

    static constexpr void* GetIndex()
    {
        return (void*)(&index);
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