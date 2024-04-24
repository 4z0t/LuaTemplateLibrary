#include "TestBase.hpp"

struct STDContainersTests : TestBase
{

};

TEST_F(STDContainersTests, VectorTests)
{
    using namespace LTL;
    using namespace std;

    {
        StackRestorer rst{ l };
        using TestType = vector<int>;
        TestType v1 = { 1,2,3,4,54 };
        PushValue(l, v1);
        auto  v2 = GetValue<TestType>(l, -1);
        ASSERT_EQ(v1, v2);
    }


}

TEST_F(STDContainersTests, UnorderedMapTests)
{
    using namespace LTL;
    using namespace std;

    {
        StackRestorer rst{ l };
        using TestType = unordered_map<int, int>;
        TestType v1 = {
            {1,2},
            {0, 4},
            {4,3},
            {7,-1}
        };
        PushValue(l, v1);
        auto  v2 = GetValue<TestType>(l, -1);
        ASSERT_EQ(v1, v2);
    }

    {
        StackRestorer rst{ l };
        using TestType = unordered_map<string, int>;
        TestType v1 = {
            {"a",2},
            {"b", 4},
            {"bcd",3},
            {"s",-1}
        };
        PushValue(l, v1);
        auto  v2 = GetValue<TestType>(l, -1);
        ASSERT_EQ(v1, v2);
    }

    {
        StackRestorer rst{ l };
        using TestType = unordered_map<string, GRefObject>;
        TestType v1 = {
            {"a",{l,2}},
            {"b",{l,3}},
            {"c",{l,4}},
            {"d",{l,5}},
        };
        PushValue(l, v1);
        auto  v2 = GetValue<TestType>(l, -1);
        ASSERT_EQ(v1, v2);

    }


}


struct MultReturnTests :TestBase
{

};


TEST_F(MultReturnTests, Tests)
{
    using namespace LTL;
    using namespace std;
    {

        constexpr auto f = +[]() -> MultReturn<int, int>
            {
                return { 1,2 };
            };

        RegisterFunction(l, "f", CFunction<f>::Function);

        Run("a,b = f()");

        ASSERT_TRUE(GRefObject::Global(l, "a").Is<int>());
        ASSERT_EQ(GRefObject::Global(l, "a").To<int>(), 1);
        ASSERT_TRUE(GRefObject::Global(l, "b").Is<int>());
        ASSERT_EQ(GRefObject::Global(l, "b").To<int>(), 2);

    }

}
struct OptionalTests :TestBase
{

};


TEST_F(OptionalTests, Tests)
{
    using namespace LTL;
    using namespace std;
    {
        constexpr auto f = +[](const optional<int>& value) -> bool
            {
                return value.has_value();
            };


        RegisterFunction(l, "f", CFunction<f, optional<int>>::Function);
        {
            Run("result = f()");
            ASSERT_FALSE(Result().To<bool>());
        }
        {
            Run("result = f(1)");
            ASSERT_TRUE(Result().To<bool>());
        }
        {
            Run("result = f(null)");
            ASSERT_FALSE(Result().To<bool>());
        }
        {
            ASSERT_THROW(Run("result = f('a')"), Exception);
        }
        RegisterFunction(l, "f", CFunction<&optional<int>::has_value, optional<int>>::Function);
        {
            Run("result = f()");
            ASSERT_FALSE(Result().To<bool>());
        }
        {
            Run("result = f(1)");
            ASSERT_TRUE(Result().To<bool>());
        }
        {
            Run("result = f(null)");
            ASSERT_FALSE(Result().To<bool>());
        }
        {
            ASSERT_THROW(Run("result = f('a')"), Exception);

        }

    }

}
