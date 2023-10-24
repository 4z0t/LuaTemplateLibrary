#include "TestBase.hpp"


struct ExceptionTests : TestBase
{

};

TEST_F(ExceptionTests, ThrowTests)
{
    Run("result = true");
    ASSERT_THROW(Result().Call(), Lua::Exception);

}