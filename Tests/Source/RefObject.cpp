#include "TestBase.hpp"


struct RefObjectTests : TestBase
{

};

TEST_F(RefObjectTests, ValueAccess)
{
    Run("result = true");
    ASSERT_TRUE(Result().Is<bool>());
    ASSERT_TRUE(Result().To<bool>());
    ASSERT_TRUE((bool)Result());
    
    Run("result = 4");
    ASSERT_TRUE(Result().Is<int>());
    ASSERT_EQ(Result().To<int>(), 4);
    ASSERT_EQ((int)Result(), 4);




}
