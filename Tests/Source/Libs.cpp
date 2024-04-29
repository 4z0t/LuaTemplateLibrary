#include "TestBase.hpp"

struct LibsTest : public testing::Test
{

};


TEST_F(LibsTest, CheckLibs)
{
    using namespace LTL;
    using namespace std;
    {
        State s;
        ASSERT_TRUE(s.GetGlobal("string").Is<Type::Nil>());
        ASSERT_TRUE(s.GetGlobal("math").Is<Type::Nil>());
        ASSERT_TRUE(s.GetGlobal("_G").Is<Type::Nil>());
        ASSERT_TRUE(s.GetGlobal("table").Is<Type::Nil>());
        ASSERT_TRUE(s.GetGlobal("package").Is<Type::Nil>());
        ASSERT_TRUE(s.GetGlobal("io").Is<Type::Nil>());
        ASSERT_TRUE(s.GetGlobal("os").Is<Type::Nil>());
        ASSERT_TRUE(s.GetGlobal("coroutine").Is<Type::Nil>());
        ASSERT_TRUE(s.GetGlobal("debug").Is<Type::Nil>());
    }


    {
        State s;
        s.OpenLibs(Libs::global, Libs::math, Libs::string, Libs::table);

        ASSERT_TRUE(s.GetGlobal("string").Is<Type::Table>());
        ASSERT_TRUE(s.GetGlobal("math").Is<Type::Table>());
        ASSERT_TRUE(s.GetGlobal("_G").Is<Type::Table>());
        ASSERT_TRUE(s.GetGlobal("table").Is<Type::Table>());

        ASSERT_TRUE(s.GetGlobal("io").Is<Type::Nil>());
        ASSERT_TRUE(s.GetGlobal("os").Is<Type::Nil>());
        ASSERT_TRUE(s.GetGlobal("coroutine").Is<Type::Nil>());
        ASSERT_TRUE(s.GetGlobal("debug").Is<Type::Nil>());
        ASSERT_TRUE(s.GetGlobal("package").Is<Type::Nil>());
    }

    {
        State s;
        s.OpenLibs(Libs::debug, Libs::io, Libs::os);

        ASSERT_TRUE(s.GetGlobal("string").Is<Type::Nil>());
        ASSERT_TRUE(s.GetGlobal("math").Is<Type::Nil>());
        ASSERT_TRUE(s.GetGlobal("_G").Is<Type::Nil>());
        ASSERT_TRUE(s.GetGlobal("table").Is<Type::Nil>());
        ASSERT_TRUE(s.GetGlobal("coroutine").Is<Type::Nil>());
        ASSERT_TRUE(s.GetGlobal("package").Is<Type::Nil>());

        ASSERT_TRUE(s.GetGlobal("io").Is<Type::Table>());
        ASSERT_TRUE(s.GetGlobal("os").Is<Type::Table>());
        ASSERT_TRUE(s.GetGlobal("debug").Is<Type::Table>());
    }
}

struct MyLib
{
    static float Sum(float f1, float f2)
    {
        return f1 + f2;
    }

    static float Mul(float f1, float f2)
    {
        return f1 * f2;
    }

    static const luaL_Reg funcs[];

    static int Open(lua_State* L)
    {
        lua_newtable(L);
        luaL_setfuncs(L, funcs, 0);
        return 1;
    }
};

const luaL_Reg MyLib::funcs[] = {
    {"sum", LTL::CFunction<&MyLib::Sum, float, float>::Function},
    {"mul", LTL::CFunction<&MyLib::Mul, float, float>::Function},
    { NULL, NULL }
};

TEST_F(LibsTest, CustomLib)
{
    using namespace LTL;
    using namespace std;

    constexpr Lib my_lib{ "lib", &MyLib::Open };

    State s;
    s.OpenLibs(my_lib);

    ASSERT_TRUE(s.GetGlobal("lib").Is<Type::Table>());
    s.Run(R"===(
    a = lib.sum(1,2)
    b = lib.mul(2,3)
    )===");

    ASSERT_EQ(s.GetGlobal("a").To<float>(), 3);
    ASSERT_EQ(s.GetGlobal("b").To<float>(), 6);

}