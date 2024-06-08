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


TEST_F(InheritanceTest, CallGettersAndSetters)
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
        .Add("a", AProperty<&A::a>{})
        .Add("getA", Method<&A::GetA>{})
        ;

    Class<B>(s, "B")
        .AddConstructor<>()
        .Add(MetaMethods::add, Method<&B::operator+, int>{})
        .Add("b", AProperty<&B::b>{})
        .Add("getB", Method<&B::GetB>{})
        ;

    Class<C>(s, "C")
        .Inherit<A, B>()
        .Add("c", AProperty<&C::c>{})
        .AddConstructor<>()
        .Add("getC", Method<&C::GetC>{})
        ;

    s.Run(R"(
    a = A()
    b = B()
    c = C()
            )");


    s.Run(R"(
    r = c.a
            )");
    {
        auto r = s.GetGlobal("r");
        ASSERT_EQ(r, 2);
    }

    s.Run(R"(
    r = c.b
            )");
    {
        auto r = s.GetGlobal("r");
        ASSERT_EQ(r, 4);
    }

    s.Run(R"(
    r = c.c
            )");
    {
        auto r = s.GetGlobal("r");
        ASSERT_EQ(r, 5);
    }

}

TEST_F(InheritanceTest, TestVirtualClasses)
{
    struct I
    {
        //doesnt support purely virtual classes
        virtual int GetValue() { return 0; }
    };

    struct A : I
    {
        int GetValue()override
        {
            return 1;
        }
    };

    struct B : I
    {
        int GetValue()override
        {
            return 2;
        }
    };

    struct C : I
    {
        int GetValue()override
        {
            return 3;
        }
    };

    using namespace LTL;
    using namespace std;

    State s;
    s.OpenLibs();

    Class<I>(s, "I")
        .AddGetter("value", Method<&I::GetValue>{})
        ;

    Class<A>(s, "A")
        .AddConstructor<>()
        .Inherit<I>()
        ;
    Class<B>(s, "B")
        .AddConstructor<>()
        .Inherit<I>()
        ;

    Class<C>(s, "C")
        .AddConstructor<>()
        .Inherit<I>()
        ;

    s.Run(R"(
    a = A()
    b = B()
    c = C()
            )");

    s.Run(R"(
    r = a.value
            )");
    {
        auto r = s.GetGlobal("r");
        ASSERT_EQ(r, 1);
    }

    s.Run(R"(
    r = b.value
            )");
    {
        auto r = s.GetGlobal("r");
        ASSERT_EQ(r, 2);
    }

    s.Run(R"(
    r = c.value
            )");
    {
        auto r = s.GetGlobal("r");
        ASSERT_EQ(r, 3);
    }


}


