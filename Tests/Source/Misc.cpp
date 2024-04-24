#include "TestBase.hpp"



#pragma region Memory tests
#ifdef _WIN32

// Tets memory leaks ans safety
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
}
#endif // WINDOWS
#pragma endregion