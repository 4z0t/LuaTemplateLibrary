#include "TestBase.hpp"

struct ExceptionTests : TestBase
{

};

TEST_F(ExceptionTests, ThrowTests)
{
    using namespace LTL;
    Run("result = true");
    ASSERT_THROW(Result().Call(), Exception);
    ASSERT_THROW(int _ = Result().Call<int>(), Exception);
    ASSERT_THROW(Result()["Key"].To<int>(), Exception);

}