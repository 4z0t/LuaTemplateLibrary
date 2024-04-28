#include "TestBase.hpp"


struct UserDataTests : TestBase
{

};

TEST_F(UserDataTests, Class_Basic)
{
    struct MyClass
    {
        int a = 0;
        MyClass() = default;
        MyClass(int a) :a(a) {}
        MyClass(const MyClass&) = default;
        MyClass(MyClass&&) = default;
        ~MyClass() = default;

        int GetA()const { return a; }

        void SetA(int a) { this->a = a; }
    };
    using namespace LTL;

    Class<MyClass>(l, "Class")
        .AddConstructor<int>()
        .Add("GetA", Method<&MyClass::GetA>{})
        .Add("SetA", Method<&MyClass::SetA, int>{})
        ;
    Run(R"===(
    result = Class(4)
    )===");

    ASSERT_TRUE(Result().Is<Type::Userdata>());
    ASSERT_TRUE(Result().Is<UserData<MyClass>>());

    Run(R"===(
    a = result
    result = a:GetA()
    )===");

    ASSERT_TRUE(Result().Is<int>());
    ASSERT_EQ(Result().To<int>(), 4);

    Run(R"===(
    a:SetA(3)
    result = a:GetA()
    )===");

    ASSERT_TRUE(Result().Is<int>());
    ASSERT_EQ(Result().To<int>(), 3);
    {
        // wrong value call
        ASSERT_THROW(Run("a:SetA('df')"), Exception);
        ASSERT_THROW(Run("a:SetA({})"), Exception);
        ASSERT_THROW(Run("a:SetA(nil)"), Exception);
        ASSERT_THROW(Run("a:SetA()"), Exception);
        ASSERT_THROW(Run("a:SetA(false)"), Exception);
        ASSERT_THROW(Run("a:SetA(a)"), Exception);
    }
    {
        // wrong u data call
        ASSERT_THROW(Run("a.SetA(4)"), Exception);
        ASSERT_THROW(Run("a.SetA('df')"), Exception);
        ASSERT_THROW(Run("a.SetA({})"), Exception);
        ASSERT_THROW(Run("a.SetA(nil)"), Exception);
        ASSERT_THROW(Run("a.SetA()"), Exception);
        ASSERT_THROW(Run("a.SetA(false)"), Exception);
        ASSERT_THROW(Run("a.SetA(a)"), Exception);
    }

}

TEST_F(UserDataTests, Class_MethodBased_GettersAndSetters)
{
    struct MyClass
    {
        int a = 0;
        MyClass() = default;
        MyClass(int a) :a(a) {}
        MyClass(const MyClass&) = default;
        MyClass(MyClass&&) = default;
        ~MyClass() = default;

        int GetA()const { return a; }

        void SetA(int a) { this->a = a; }
    };
    using namespace LTL;

    Class<MyClass>(l, "Class")
        .AddConstructor<int>()
        .AddGetter("A", Method<&MyClass::GetA>{})
        .AddSetter("A", Method<&MyClass::SetA, int>{})
        ;

    Run(R"===(
    result = Class(4)
    )===");

    ASSERT_TRUE(Result().Is<Type::Userdata>());
    ASSERT_TRUE(Result().Is<UserData<MyClass>>());

    Run(R"===(
    a = result
    result = a.A
    )===");

    ASSERT_TRUE(Result().Is<int>());
    ASSERT_EQ(Result().To<int>(), 4);

    Run(R"===(
    a.A = 3
    result = a.A
    )===");

    ASSERT_TRUE(Result().Is<int>());
    ASSERT_EQ(Result().To<int>(), 3);

    {
        // wrong value call
        ASSERT_THROW(Run("a.A = 'df'"), Exception);
        ASSERT_THROW(Run("a.A = {}"), Exception);
        ASSERT_THROW(Run("a.A = nil"), Exception);
        ASSERT_THROW(Run("a.A = false"), Exception);
        ASSERT_THROW(Run("a.A = a"), Exception);
    }
}


TEST_F(UserDataTests, CantCopyTest)
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

        int GetA()
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
        .Add("GetA", Method<&CantCopy::GetA>{})
        ;
    Run("local a = CantCopy() "
        "result = a:GetA()"
    );


    ASSERT_TRUE(Result().Is<int>());
    ASSERT_EQ(Result().To<int>(), 0);

    Run("local a = CantCopy(4) "
        "result = a:Duplicate():GetA()"
    );

    ASSERT_TRUE(Result().Is<int>());
    ASSERT_EQ(Result().To<int>(), 8);
}

TEST_F(UserDataTests, CantMoveTest)
{
    using namespace LTL;
    struct CantMove
    {

        CantMove(int a) :a(a) {}
        CantMove(CantMove&&) = delete;
        CantMove(const CantMove& other)noexcept
        {
            a = other.a;
        }

        int GetA()
        {
            return a;
        }

        CantMove Dup()
        {
            return CantMove(a * 2);
        }

        int a = 4;
    };

    Class<CantMove>(l, "CantCopy")
        .AddConstructor<Default<int>>()
        .Add("Duplicate", Method<&CantMove::Dup, CantMove()>{})
        .Add("GetA", Method<&CantMove::GetA>{})
        ;
    Run("local a = CantCopy() "
        "result = a:GetA()"
    );

    ASSERT_TRUE(Result().Is<int>());
    ASSERT_EQ(Result().To<int>(), 0);

    Run("local a = CantCopy(4) "
        "result = a:Duplicate():GetA()"
    );

    ASSERT_TRUE(Result().Is<int>());
    ASSERT_EQ(Result().To<int>(), 8);
}

TEST_F(UserDataTests, CantCopyAndMove)
{
    using namespace LTL;
    struct CantMove
    {

        CantMove(int a) :a(a) {}
        CantMove(CantMove&&) = delete;
        CantMove(const CantMove& other) = delete;

        int GetA()
        {
            return a;
        }

        StackResult Duplicate(CState* cstate)
        {
            UserData<CantMove>::New(cstate, a * 2);
            return 1;
        }

        int a = 4;
    };

    Class<CantMove>(l, "CantCopy")
        .AddConstructor<Default<int>>()
        .Add("Duplicate", Method<&CantMove::Duplicate, CState*>{})
        .Add("GetA", Method<&CantMove::GetA>{})
        ;

    Run("local a = CantCopy() "
        "result = a:GetA()"
    );

    ASSERT_TRUE(Result().Is<int>());
    ASSERT_EQ(Result().To<int>(), 0);

    Run("local a = CantCopy(4) "
        "result = a:Duplicate()"
        ":GetA()"
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
