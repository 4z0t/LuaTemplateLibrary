#include "TestBase.hpp"
#include <algorithm>


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

struct PCallSafety :TestBase
{

};
// I wonder if it is ok to use PCall with long jump
TEST_F(PCallSafety, StringTest)
{

    using namespace LTL;
    using namespace std;
    struct MyFunctions
    {
        static bool Fn(const vector<int>& s, int a)
        {
            return find(s.begin(), s.end(), a) != s.end();
        }
    };

    {
        MemoryLeakDetector leakDetector;
        State s;
        s.ThrowExceptions();
        s.Add("Fn", CFunction<MyFunctions::Fn, vector<int>, int>{});


        GRefObject tbl{ s,vector<int>({ 1,2,3,4 }) };
        s.PCall("Fn", tbl);
    }
}


#endif // WINDOWS
#pragma endregion