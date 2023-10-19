#include "TestBase.hpp"


struct RefObjectTests : TestBase
{

};

TEST_F(RefObjectTests, ValueAccess)
{
    Run("result = true");
    ASSERT_TRUE(Result().Is<bool>());


}
