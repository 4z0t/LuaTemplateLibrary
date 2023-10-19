#include <gtest/gtest.h>

int main(int argc, char** argv)
{
    if (argc == 1)
    {
        testing::GTEST_FLAG(filter) = "-PerformanceTests.AllTests";
    }

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}