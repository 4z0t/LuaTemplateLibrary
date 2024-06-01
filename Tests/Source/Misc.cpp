#include "TestBase.hpp"



#pragma region Memory tests
#ifdef _WIN32

// Tests memory leaks ans safety
// https://stackoverflow.com/questions/29174938/googletest-and-memory-leaks
#include <crtdbg.h>

class MemoryLeakDetector {
public:
    MemoryLeakDetector() {
        _CrtMemCheckpoint(&memState_);
    }

    ~MemoryLeakDetector() {
        _CrtMemState stateNow, stateDiff;
        _CrtMemCheckpoint(&stateNow);
        int diffResult = _CrtMemDifference(&stateDiff, &memState_, &stateNow);
        if (diffResult)
            reportFailure(stateDiff.lSizes[1]);
    }
private:
    void reportFailure(unsigned int unfreedBytes) {
        FAIL() << "Memory leak of " << unfreedBytes << " byte(s) detected.";
    }
    _CrtMemState memState_;
};

struct TestNotRegisteredUserDataClass :TestBase
{

};


struct MyMemoryClass
{
public:
    std::vector<int> v;


    MyMemoryClass(int n)
    {
        v.reserve(n);
    }
    ~MyMemoryClass() = default;

};

TEST_F(TestNotRegisteredUserDataClass, Tests)
{
    struct Vector3f {
        float x, y, z;
        Vector3f(float x, float y, float z) : x(x), y(y), z(z)
        {

        }
    };
    using namespace LTL;
    using namespace std;
    {

        MemoryLeakDetector leakDetector;
        State s;
        s.ThrowExceptions();
        ASSERT_THROW(s.MakeUserData<Vector3f>(1, 2, 3), Exception);
    }
    {
        MemoryLeakDetector leakDetector;
        State s;
        s.ThrowExceptions();
        ASSERT_THROW(s.MakeUserData<MyMemoryClass>(10), Exception);
    }
    {
        MemoryLeakDetector leakDetector;
        State s;
        s.ThrowExceptions();
        Class<MyMemoryClass>(s, "Mem")
            .AddConstructor<int>();
        s.MakeUserData<MyMemoryClass>(10);
    }
    {
        MemoryLeakDetector leakDetector;
        State s;
        s.ThrowExceptions();
        Class<MyMemoryClass>(s, "Mem")
            .AddConstructor<int>();
        s.MakeUserData<MyMemoryClass>(10);
        ASSERT_ANY_THROW(s.GetState()->Error());
    }
}
#endif // WINDOWS
#pragma endregion

struct TestRetTypes : TestBase
{

};

TEST_F(TestRetTypes, TestStackResult)
{
    using namespace LTL;
    using namespace std;
    struct TestClass
    {
        static StackResult F(lua_State* l, int a, int b)
        {
            if (a > b)
            {
                PushValue(l, a + b);
                PushValue(l, a - b);
                return 2;
            }
            PushValue(l, a + b);
            return 1;
        }



    };

    RegisterFunction(l, "F", CFunction<TestClass::F, lua_State*, int, int>::Function);
    {
        auto res = CallFunction<MultReturn<int, std::optional<int>>>(l, GlobalValue{ "F" }, 2, 1);
        ASSERT_EQ(get<0>(res), 2 + 1);
        ASSERT_TRUE(get<1>(res).has_value());
        ASSERT_EQ(get<1>(res).value(), 1);
    }
    {
        auto res = CallFunction<MultReturn<int, std::optional<int>>>(l, GlobalValue{ "F" }, 1, 2);
        ASSERT_EQ(get<0>(res), 2 + 1);
        ASSERT_FALSE(get<1>(res).has_value());
    }
}