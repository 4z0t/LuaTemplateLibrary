#include "TestBase.hpp"

struct StateTests : TestBase
{

};


TEST_F(StateTests, UpvalueTest)
{
    using namespace LTL;

    constexpr auto func = +[](int a, CState* s, int b)->int
        {
            int c = s->GetGlobal<int>("globalValue");
            return a + b + c;
        };

    Run("globalValue = 4 ");
    RegisterClosure(l, "Func", CFunction<func, Upvalue<int>, CState*, Upvalue<int>>::Function, 1, 2);
    Run("result = Func()");

    ASSERT_EQ(1 + 2 + 4, Result().To<int>());
}


TEST_F(StateTests, AllocTest)
{
    using namespace LTL;

    State<OpNewAllocator> s;
    s.Run("result = 4");
    ASSERT_TRUE(GRefObject::Global(s, "result").Is<int>()) << "The actiual type is " << GRefObject::Global(s, "result").TypeName();
    ASSERT_EQ(GRefObject::Global(s, "result").To<int>(), 4);

    ASSERT_TRUE(s.GetGlobal("result").Is<int>());
    ASSERT_EQ(s.GetGlobal("result").To<int>(), 4);

}