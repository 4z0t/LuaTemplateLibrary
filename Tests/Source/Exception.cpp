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

TEST_F(ExceptionTests, UserExceptions)
{
    using namespace LTL;
    using namespace std;
    auto cstate = CState::Wrap(l);

    cstate->SetAtPanicFuntion(+[](lua_State* l)->int {
        string r = GetValue<string>(l, -1);
        lua_pop(l, 1);
        throw runtime_error(r);
        });


    ASSERT_THROW(Run("error('something is wrong!')"), runtime_error);

    {
        constexpr auto f = +[](float x, float y)noexcept -> float
            {
                return sqrtf(x * x + y * y);
            };

        RegisterFunction(l, "VectorLen", CFunction<f, float, float>::Function);

        ASSERT_THROW(Run("VectorLen()"), runtime_error);
        ASSERT_THROW(Run("VectorLen(1)"), runtime_error);
        ASSERT_THROW(Run("VectorLen(2, 'invalid value!')"), runtime_error);
        ASSERT_THROW(Run("VectorLen('invalid value!')"), runtime_error);
        ASSERT_THROW(Run("VectorLen('invalid value!', 2)"), runtime_error);

        ASSERT_THROW(cstate->Call<float>("VectorLen"), runtime_error);
        ASSERT_THROW(cstate->Call<float>("VectorLen", 1.f), runtime_error);
        ASSERT_THROW(cstate->Call<float>("VectorLen", 2.f, "invalid value!"), runtime_error);
        ASSERT_THROW(cstate->Call<float>("VectorLen", "invalid value!"), runtime_error);
        ASSERT_THROW(cstate->Call<float>("VectorLen", "invalid value!", 1.f), runtime_error);


        Run("a = VectorLen(3, 4)");
        ASSERT_FLOAT_EQ(cstate->GetGlobal<float>("a"), 5.f);
        ASSERT_FLOAT_EQ(cstate->Call<float>("VectorLen", 3.f, 4.f), 5);
    }

    {
        constexpr auto f = +[](float x) -> float
            {
                if (x > 0)
                {
                    return 1 / sqrtf(x);
                }
                throw invalid_argument("value must be greater than zero!");
                return 0;
            };
        RegisterFunction(l, "InvFloat", CFunction<f, float>::Function);
        ASSERT_THROW(Run("InvFloat()"), runtime_error);
        ASSERT_THROW(Run("InvFloat('a')"), runtime_error);
        ASSERT_THROW(Run("InvFloat(-1)"), runtime_error);
        ASSERT_THROW(Run("InvFloat(0)"), runtime_error);
        Run("a = InvFloat(1)");
        ASSERT_FLOAT_EQ(cstate->GetGlobal<float>("a"), 1.f);


        ASSERT_THROW(cstate->Call<float>("InvFloat"), runtime_error);
        ASSERT_THROW(cstate->Call<float>("InvFloat", "a"), runtime_error);
        ASSERT_THROW(cstate->Call<float>("InvFloat", -1), runtime_error);
        ASSERT_THROW(cstate->Call<float>("InvFloat", 0), runtime_error);
        ASSERT_FLOAT_EQ(cstate->Call<float>("InvFloat", 1), 1.f);

    }



}