#include "TestBase.hpp"

#pragma region TypeMatchingTests
struct TypeMatchingTests : TestBase
{

};

TEST_F(TypeMatchingTests, MatchArgumentTypesTests)

{
    using namespace LTL;
#define ASSERT_MATCHES(s, res)\
    Run("result = Match(" s ")"); \
    ASSERT_TRUE(Result().Is<bool>()); \
    ASSERT_EQ(Result().To<bool>(), res)

    {
        using Match = typename MatchArgumentTypes<int, int, int>;
        ASSERT_EQ(Match::max_arg_count, 3);
        ASSERT_EQ(Match::min_arg_count, 3);
        RegisterFunction(l, "Match", Match::Function);

        ASSERT_MATCHES("1,2,3", true);
        ASSERT_MATCHES("1,0,3", true);
        ASSERT_MATCHES("1.2,2,3", false);
        ASSERT_MATCHES("true,2,3", false);
        ASSERT_MATCHES("1,false,3", false);
        ASSERT_MATCHES("1,'hello',3", false);
        ASSERT_MATCHES("1,2,3,4,5", false);
        ASSERT_MATCHES("1,2,3,4", false);
        ASSERT_MATCHES("1", false);
        ASSERT_MATCHES("1,2", false);
        ASSERT_MATCHES("nil,1,2", false);
        ASSERT_MATCHES("nil, nil, 1", false);
        ASSERT_MATCHES("nil, nil, nil", false);
        ASSERT_MATCHES("1,2,3,4,5", false);
        ASSERT_MATCHES("", false);
    }
    {
        using Match = typename MatchArgumentTypes<Default<int>, Default<int>, Default<int>>;
        ASSERT_EQ(Match::max_arg_count, 3);
        ASSERT_EQ(Match::min_arg_count, 0);
        RegisterFunction(l, "Match", Match::Function);

        ASSERT_MATCHES("1,2,3", true);
        ASSERT_MATCHES("1,0,3", true);
        ASSERT_MATCHES("1.2,2,3", false);
        ASSERT_MATCHES("true,2,3", false);
        ASSERT_MATCHES("1,false,3", false);
        ASSERT_MATCHES("1,'hello',3", false);
        ASSERT_MATCHES("1,2,3,4,5", false);
        ASSERT_MATCHES("1,2,3,4", false);
        ASSERT_MATCHES("1", true);
        ASSERT_MATCHES("1,2", true);
        ASSERT_MATCHES("nil,1,2", true);
        ASSERT_MATCHES("nil, nil, 1", true);
        ASSERT_MATCHES("nil, nil, nil", true);
        ASSERT_MATCHES("", true);
    }

    {
        using Match = typename MatchArgumentTypes<Default<int>, Default<int>, Default<int>, Upvalue<int>>;
        ASSERT_EQ(Match::max_arg_count, 3);
        ASSERT_EQ(Match::min_arg_count, 0);
        RegisterClosure(l, "Match", Match::Function, 1);

        ASSERT_MATCHES("1,2,3", true);
        ASSERT_MATCHES("1,0,3", true);
        ASSERT_MATCHES("1.2,2,3", false);
        ASSERT_MATCHES("true,2,3", false);
        ASSERT_MATCHES("1,false,3", false);
        ASSERT_MATCHES("1,'hello',3", false);
        ASSERT_MATCHES("1,2,3,4,5", false);
        ASSERT_MATCHES("1,2,3,4", false);
        ASSERT_MATCHES("1", true);
        ASSERT_MATCHES("1,2", true);
        ASSERT_MATCHES("nil,1,2", true);
        ASSERT_MATCHES("nil, nil, 1", true);
        ASSERT_MATCHES("nil, nil, nil", true);
        ASSERT_MATCHES("", true);
    }

    {
        using Match = typename MatchArgumentTypes<Default<int>, lua_State*, Default<int>, Default<int>, Upvalue<int>>;
        ASSERT_EQ(Match::max_arg_count, 3);
        ASSERT_EQ(Match::min_arg_count, 0);
        RegisterClosure(l, "Match", Match::Function, 1);

        ASSERT_MATCHES("1,2,3", true);
        ASSERT_MATCHES("1,0,3", true);
        ASSERT_MATCHES("1.2,2,3", false);
        ASSERT_MATCHES("true,2,3", false);
        ASSERT_MATCHES("1,false,3", false);
        ASSERT_MATCHES("1,'hello',3", false);
        ASSERT_MATCHES("1,2,3,4,5", false);
        ASSERT_MATCHES("1,2,3,4", false);
        ASSERT_MATCHES("1", true);
        ASSERT_MATCHES("1,2", true);
        ASSERT_MATCHES("nil,1,2", true);
        ASSERT_MATCHES("nil, nil, 1", true);
        ASSERT_MATCHES("nil, nil, nil", true);
        ASSERT_MATCHES("", true);
    }

    {
        using Match = typename MatchArgumentTypes<int, lua_State*, Default<int>, Default<int>, Upvalue<int>>;
        ASSERT_EQ(Match::max_arg_count, 3);
        ASSERT_EQ(Match::min_arg_count, 1);
        RegisterClosure(l, "Match", Match::Function, 1);

        ASSERT_MATCHES("1,2,3", true);
        ASSERT_MATCHES("1,0,3", true);
        ASSERT_MATCHES("1.2,2,3", false);
        ASSERT_MATCHES("true,2,3", false);
        ASSERT_MATCHES("1,false,3", false);
        ASSERT_MATCHES("1,'hello',3", false);
        ASSERT_MATCHES("1,2,3,4,5", false);
        ASSERT_MATCHES("1,2,3,4", false);
        ASSERT_MATCHES("1", true);
        ASSERT_MATCHES("1,2", true);
        ASSERT_MATCHES("nil,1,2", false);
        ASSERT_MATCHES("nil, nil, 1", false);
        ASSERT_MATCHES("nil, nil, nil", false);
        ASSERT_MATCHES("", false);
    }
    {
        using Match = typename MatchArgumentTypes<Default<int>, lua_State*, int, Default<int>, Upvalue<int>>;
        ASSERT_EQ(Match::max_arg_count, 3);
        ASSERT_EQ(Match::min_arg_count, 2);
        RegisterClosure(l, "Match", Match::Function, 1);

        ASSERT_MATCHES("1,2,3", true);
        ASSERT_MATCHES("1,0,3", true);
        ASSERT_MATCHES("1.2,2,3", false);
        ASSERT_MATCHES("true,2,3", false);
        ASSERT_MATCHES("1,false,3", false);
        ASSERT_MATCHES("1,'hello',3", false);
        ASSERT_MATCHES("1,2,3,4,5", false);
        ASSERT_MATCHES("1,2,3,4", false);
        ASSERT_MATCHES("1", false);
        ASSERT_MATCHES("1,2", true);
        ASSERT_MATCHES("nil,1,2", true);
        ASSERT_MATCHES("nil, nil, 1", false);
        ASSERT_MATCHES("nil, nil, nil", false);
        ASSERT_MATCHES("", false);
    }

    {
        using Match = typename MatchArgumentTypes<Default<int>, lua_State*, int, int, Upvalue<int>>;
        ASSERT_EQ(Match::max_arg_count, 3);
        ASSERT_EQ(Match::min_arg_count, 3);
        RegisterClosure(l, "Match", Match::Function, 1);

        ASSERT_MATCHES("1,2,3", true);
        ASSERT_MATCHES("1,0,3", true);
        ASSERT_MATCHES("1.2,2,3", false);
        ASSERT_MATCHES("true,2,3", false);
        ASSERT_MATCHES("1,false,3", false);
        ASSERT_MATCHES("1,'hello',3", false);
        ASSERT_MATCHES("1,2,3,4,5", false);
        ASSERT_MATCHES("1,2,3,4", false);
        ASSERT_MATCHES("1", false);
        ASSERT_MATCHES("1,2", false);
        ASSERT_MATCHES("nil,1,2", true);
        ASSERT_MATCHES("nil, nil, 1", false);
        ASSERT_MATCHES("nil, nil, nil", false);
        ASSERT_MATCHES("", false);
    }

    {
        using Match = typename MatchArgumentTypes<Default<float>, lua_State*, float, float, Upvalue<float>>;
        ASSERT_EQ(Match::max_arg_count, 3);
        ASSERT_EQ(Match::min_arg_count, 3);
        RegisterClosure(l, "Match", Match::Function, 1);

        ASSERT_MATCHES("1,2,3", true);
        ASSERT_MATCHES("1,0,3", true);
        ASSERT_MATCHES("1.2,2,3", true);
        ASSERT_MATCHES("true,2,3", false);
        ASSERT_MATCHES("1,false,3", false);
        ASSERT_MATCHES("1,'hello',3", false);
        ASSERT_MATCHES("1,2,3,4,5", false);
        ASSERT_MATCHES("1,2,3,4", false);
        ASSERT_MATCHES("1", false);
        ASSERT_MATCHES("1,2", false);
        ASSERT_MATCHES("nil,1,2", true);
        ASSERT_MATCHES("nil, nil, 1", false);
        ASSERT_MATCHES("nil, nil, nil", false);
        ASSERT_MATCHES("", false);
    }

#undef ASSERT_MATCHES
}


TEST_F(TypeMatchingTests, UpvalueMatcingTests)
{
    using namespace LTL;
    using namespace LTL::FuncUtility;
#define ASSERT_TRUE_TYPE(...)  ASSERT_TRUE(__VA_ARGS__::value)
#define ASSERT_FALSE_TYPE(...)  ASSERT_FALSE(__VA_ARGS__::value)
    {
        ASSERT_TRUE_TYPE(MatchUpvalues<int, float, bool>::Matches<Upvalue<int>, int, float, bool, Upvalue<float>, Upvalue<bool>>);
        ASSERT_TRUE_TYPE(MatchUpvalues<int, float, bool>::Matches<Upvalue<int>, Upvalue<float>, Upvalue<bool>>);
        ASSERT_TRUE_TYPE(MatchUpvalues<int, float, bool>::Matches<Upvalue<int>, float, bool, Upvalue<float>, Upvalue<bool>>);
        ASSERT_TRUE_TYPE(MatchUpvalues<int, float, bool>::Matches<Upvalue<int>, Upvalue<float>, Upvalue<bool>, int, float, bool>);
        ASSERT_TRUE_TYPE(MatchUpvalues<int, float, bool>::Matches<Upvalue<int>, int, Upvalue<float>, Upvalue<bool>, float, bool>);
        ASSERT_TRUE_TYPE(MatchUpvalues<int, float, bool>::Matches<Upvalue<int>, Upvalue<float>, int, float, bool, Upvalue<bool>>);
        ASSERT_TRUE_TYPE(MatchUpvalues<int, float, bool>::Matches<int, float, bool, Upvalue<int>, Upvalue<float>, Upvalue<bool>>);
        ASSERT_TRUE_TYPE(MatchUpvalues<int, float, bool>::Matches<int, Upvalue<int>, float, Upvalue<float>, bool, Upvalue<bool>>);

        ASSERT_FALSE_TYPE(MatchUpvalues<int, float, bool>::Matches<int, float, bool>);
        ASSERT_FALSE_TYPE(MatchUpvalues<int, float, bool>::Matches<int, float, bool, Upvalue<int>>);
        ASSERT_FALSE_TYPE(MatchUpvalues<int, float, bool>::Matches<int, float, bool, Upvalue<int>, Upvalue<float>>);
        ASSERT_FALSE_TYPE(MatchUpvalues<int, float, bool>::Matches<Upvalue<float>, int, float, bool, Upvalue<int>>);
        ASSERT_FALSE_TYPE(MatchUpvalues<int, float, bool>::Matches<Upvalue<float>, Upvalue<int>, Upvalue<float>, Upvalue<bool>>);
        ASSERT_FALSE_TYPE(MatchUpvalues<int, float, bool>::Matches<>);
        ASSERT_FALSE_TYPE(MatchUpvalues<int, float, bool>::Matches<Upvalue<int>, Upvalue<float>, Upvalue<int>>);
        ASSERT_FALSE_TYPE(MatchUpvalues<int, float, bool>::Matches<Upvalue<int>>);
        ASSERT_FALSE_TYPE(MatchUpvalues<int, float, bool>::Matches<int, float, Upvalue<bool>>);

        ASSERT_TRUE_TYPE(MatchUpvalues<>::Matches<>);
        ASSERT_TRUE_TYPE(MatchUpvalues<>::Matches<int, float, bool>);
        ASSERT_FALSE_TYPE(MatchUpvalues<>::Matches<int, float, bool, Upvalue<int>>);
        ASSERT_FALSE_TYPE(MatchUpvalues<>::Matches<int, float, bool, Upvalue<int>, Upvalue<float>>);
        ASSERT_FALSE_TYPE(MatchUpvalues<>::Matches<Upvalue<float>, int, float, bool, Upvalue<int>>);
        ASSERT_FALSE_TYPE(MatchUpvalues<>::Matches<Upvalue<float>, Upvalue<int>, Upvalue<float>, Upvalue<bool>>);
        ASSERT_FALSE_TYPE(MatchUpvalues<>::Matches<Upvalue<int>, Upvalue<float>, Upvalue<int>>);
        ASSERT_FALSE_TYPE(MatchUpvalues<>::Matches<Upvalue<int>>);
        ASSERT_FALSE_TYPE(MatchUpvalues<>::Matches<int, float, Upvalue<bool>>);
    }

    {

    }
#undef ASSERT_TRUE_TYPE
#undef ASSERT_FALSE_TYPE
}
#pragma endregion
