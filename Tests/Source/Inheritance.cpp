#include "TestBase.hpp"

struct InheritanceTest : testing::Test
{

};

TEST_F(InheritanceTest, Basic)
{
    struct A
    {
        int a = 2;

        int GetA()const
        {
            return a;
        }
    };

    struct B
    {
        int b = 4;

        int GetB()const
        {
            return b;
        }
    };

    struct C : A, B
    {
        int c = 5;

        int GetC()const
        {
            return c;
        }
    };

    using namespace LTL;
    using namespace std;

    State s;
    s.OpenLibs();

    Class<A>(s, "A")
        .AddConstructor<>()
        .Add("getA", Method<&A::GetA>{})
        ;

    Class<B>(s, "B")
        .AddConstructor<>()
        .Add("getB", Method<&B::GetB>{})
        ;

    Class<C>(s, "C")
        .AddConstructor<>()
        .Add("getC", Method<&C::GetC>{})
        .CanCastTo<B>()
        .CanCastTo<A>()
        ;

    s.Run(R"(
    a = A()
    b = B()
    c = C()

    r = a.getA(c)
            )");
    {
        auto r = s.GetGlobal("r");
        ASSERT_EQ(r, 2);
    }

    s.Run(R"(
    r = b.getB(c)
            )");
    {
        auto r = s.GetGlobal("r");
        ASSERT_EQ(r, 4);
    }

}