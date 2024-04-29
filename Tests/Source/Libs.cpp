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