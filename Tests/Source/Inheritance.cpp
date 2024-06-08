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
        .Inherit<A, B>()
        .AddConstructor<>()
        .Add("getC", Method<&C::GetC>{})
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


TEST_F(InheritanceTest, CallBaseMethods)
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
        .Inherit<A, B>()
        .AddConstructor<>()
        .Add("getC", Method<&C::GetC>{})
        ;

    s.Run(R"(
    a = A()
    b = B()
    c = C()

    r = c:getA()
            )");
    {
        auto r = s.GetGlobal("r");
        ASSERT_EQ(r, 2);
    }

    s.Run(R"(
    r = c:getB()
            )");
    {
        auto r = s.GetGlobal("r");
        ASSERT_EQ(r, 4);
    }

}


TEST_F(InheritanceTest, CallMetaMethods)
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

        int operator+(int v)const
        {
            return b + v;
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
        .Add(MetaMethods::add, Method<&B::operator+, int>{})
        .Add("getB", Method<&B::GetB>{})
        ;

    Class<C>(s, "C")
        .Inherit<A, B>()
        .AddConstructor<>()
        .Add("getC", Method<&C::GetC>{})
        ;

    s.Run(R"(
    a = A()
    b = B()
    c = C()

    r = c:getA()
            )");
    {
        auto r = s.GetGlobal("r");
        ASSERT_EQ(r, 2);
    }

    s.Run(R"(
    r = c:getB()
            )");
    {
        auto r = s.GetGlobal("r");
        ASSERT_EQ(r, 4);
    }


    s.Run(R"(
    r = c + 2
            )");
    {
        auto r = s.GetGlobal("r");
        ASSERT_EQ(r, 6);
    }

}