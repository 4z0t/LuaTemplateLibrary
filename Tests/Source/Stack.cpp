#include "TestBase.hpp"


struct StackObjectViewTest : TestBase
{

};


TEST_F(StackObjectViewTest, Tests)
{
    using namespace LTL;
    using namespace std;
    {
        const StackTopRestorer rst{ l };
        lua_pushnumber(l, 1);
        StackObjectView so{ l };
        ASSERT_TRUE(so.Is<float>());
        ASSERT_TRUE(so.Is<Type::Number>());
    }
    ASSERT_EQ(0, Top());
    {
        const StackTopRestorer rst{ l };
        PushValue(l, 1);
        StackObjectView so{ l };
        ASSERT_TRUE(so.Is<int>());
        ASSERT_TRUE(so.Is<Type::Number>());
        ASSERT_EQ(so.To<int>(), 1);
    }
    ASSERT_EQ(0, Top());
    {
        const StackTopRestorer rst{ l };
        PushValue(l, "Hello");
        StackObjectView so{ l };
        ASSERT_TRUE(so.Is<const char*>());
        ASSERT_TRUE(so.Is<Type::String>());
        ASSERT_STREQ(so.To<const char*>(), "Hello");
    }
    ASSERT_EQ(0, Top());

    {
        const StackTopRestorer rst{ l };
        PushValue(l, string_view{ "Hello" });
        StackObjectView so{ l };
        ASSERT_TRUE(so.Is<const char*>());
        ASSERT_TRUE(so.Is<Type::String>());
        ASSERT_TRUE(so.Is<string_view>());
        ASSERT_TRUE(so.Is<string>());
        ASSERT_STREQ(so.To<const char*>(), "Hello");
        ASSERT_EQ(so.To<string_view>(), string_view{ "Hello" });
        ASSERT_EQ(so.To<string>(), string{ "Hello" });
    }
    ASSERT_EQ(0, Top());
    {
        const StackTopRestorer rst{ l };
        PushValue(l, false);
        StackObjectView so{ l };
        ASSERT_TRUE(so.Is<bool>());
        ASSERT_TRUE(so.Is<Type::Boolean>());
        ASSERT_EQ(so.To<bool>(), false);
    }
    ASSERT_EQ(0, Top());

    {
        const StackTopRestorer rst{ l };
        PushValue(l, 1);
        StackObjectView s1{ l };
        PushValue(l, 2);
        StackObjectView s2{ l };
        ASSERT_FALSE(s1 == s2);
        ASSERT_TRUE(s1 != s2);
        ASSERT_TRUE(s1 == 1);
        ASSERT_TRUE(s2 == 2);


    }
    ASSERT_EQ(0, Top());
    {
        const StackTopRestorer rst{ l };
        Run("a = {b = 3}");
        lua_getglobal(l, "a");
        StackObjectView a{ l };
        auto b = a.Get("b");
        ASSERT_TRUE(b.Is<int>());
        ASSERT_TRUE(b.Is<Type::Number>());
        ASSERT_EQ(b.To<int>(), 3);
    }
    ASSERT_EQ(0, Top());

    {
        const StackTopRestorer rst{ l };
        Run("a = {b = 3}");
        lua_getglobal(l, "a");
        StackObjectView a{ l };
        PushValue(l, "b");
        StackObjectView key{ l };
        auto b = a.Get(key);
        ASSERT_TRUE(b.Is<int>());
        ASSERT_EQ(b.To<int>(), 3);
    }
    ASSERT_EQ(0, Top());

    {
        const StackTopRestorer rst{ l };
        Run("a = {}");
        lua_getglobal(l, "a");
        StackObjectView a{ l };
        PushValue(l, "b");
        StackObjectView key{ l };
        auto b1 = a.Get(key);
        ASSERT_TRUE(b1.Is<void>());
        ASSERT_TRUE(b1.Is<Type::Nil>());
        a.Set("b", 3);
        auto b2 = a.Get(key);
        ASSERT_TRUE(b2.Is<int>());
        ASSERT_EQ(b2.To<int>(), 3);
    }
    ASSERT_EQ(0, Top());
    {
        const StackTopRestorer rst{ l };
        Run("a = {}");
        lua_getglobal(l, "a");
        StackObjectView a{ l };
        PushValue(l, "b");
        StackObjectView key{ l };
        auto b1 = a.Get(key);
        ASSERT_TRUE(b1.Is<void>());
        a.Set("b", 3);
        auto b2 = a.Get<int>(key);
        ASSERT_EQ(b2, 3);
    }
    ASSERT_EQ(0, Top());
}



TEST_F(StackObjectViewTest, TestStack)
{
    using namespace LTL;
    using namespace std;
#define CheckStackChange(i, block)          \
    {                                       \
        int top = lua_gettop(l);            \
        block;                              \
        int new_top = lua_gettop(l);        \
        ASSERT_EQ(new_top, top + (i))  << "Stack has changed by " << (new_top - top) << " but expected " << i; \
        lua_settop(l, top);                 \
    }

    { // Basic stack ops
        const StackTopRestorer rst{ l };
        PushValue(l, 1);

        CheckStackChange(0, {
                StackObjectView s{ l };

                ASSERT_EQ(s.GetState(), l);
                ASSERT_EQ(s.GetIndex(), lua_gettop(l));
            });
        CheckStackChange(1, {
                StackObjectView s{ l };
                s.Push();
                ASSERT_TRUE(lua_isinteger(l, -1));
                ASSERT_EQ(lua_tonumber(l, -1), 1);
            });
        CheckStackChange(0, {
                StackObjectView s{ l };
                ASSERT_TRUE(s.Is<int>());
                ASSERT_TRUE(s.Is<Type::Number>());
                ASSERT_EQ(s.To<int>(), 1);
            });
        CheckStackChange(0, {
                StackObjectView s{ l };
                ASSERT_EQ(s, 1);
                ASSERT_NE(s, 2);
                ASSERT_NE(s, "a");
                ASSERT_NE(s, 1.5);
                ASSERT_NE(s, false);
                ASSERT_NE(s, true);
                ASSERT_NE(s, nullptr);
            });
        CheckStackChange(1, {
                StackObjectView s1{ l };
                s1.Push();
                StackObjectView s2{ l };
                ASSERT_EQ(s1, s2);
                ASSERT_EQ(s2, 1);
            });
        CheckStackChange(1, {
                StackObjectView s1{ l };
                s1.Push();
                StackObjectView s2{ l };
                ASSERT_TRUE(s1.RawEqual(s2));
                ASSERT_TRUE(s1.RawEqual(1));
                ASSERT_TRUE(s2.RawEqual(1));

                ASSERT_FALSE(s2.RawEqual(2));
                ASSERT_FALSE(s2.RawEqual("1"));
                ASSERT_FALSE(s2.RawEqual(1.5));
                ASSERT_FALSE(s2.RawEqual(false));
                ASSERT_FALSE(s2.RawEqual(true));
                ASSERT_FALSE(s2.RawEqual(nullptr));
            });
        CheckStackChange(1, {
                StackObjectView s1{ l };
                s1.Push();
                StackObjectView s2{ l };
                ASSERT_TRUE(s1 >= s2);
                ASSERT_TRUE(s1 <= s2);
                ASSERT_FALSE(s1 > s2);
                ASSERT_FALSE(s1 < s2);

                ASSERT_TRUE(s1 >= 1);
                ASSERT_TRUE(s1 >= 0);
                ASSERT_FALSE(s1 >= 2);

                ASSERT_TRUE(s1 < 2);
                ASSERT_FALSE(s1 < 1);

                ASSERT_TRUE(s1 <= 1);
                ASSERT_TRUE(s1 <= 2);
                ASSERT_FALSE(s1 <= 0);

                ASSERT_TRUE(s1 > 0);
                ASSERT_FALSE(s1 > 2);
            });
        CheckStackChange(1, {
                StackObjectView s1{ l };
                auto s2 = s1.GetMetaTable();
                ASSERT_TRUE(s2.Is<void>());
            });
    }
    {// table ops
        const StackTopRestorer rst{ l };
        Run(R"===(
                local t = {
                    __eq = function(self, other)
                        return self.n == other.n
                    end,
                    __lt = function(self, other)
                        return self.n < other
                    end,
                    __le = function(self, other)
                        return self.n <= other
                    end,
                    __len = function(self) return self.n end,
                }
                t.__index = t
                t1 = setmetatable({ n = 4 }, t)
                t2 = setmetatable({ n = 5 }, t)
                )===");
        lua_getglobal(l, "t1");
        CheckStackChange(0, {
                StackObjectView s1{ l };
                ASSERT_TRUE(s1.Is<Type::Table>());
                ASSERT_EQ(s1.Type(), Type::Table);
            });
        CheckStackChange(1, {
                StackObjectView s1{ l };
                lua_getglobal(l, "t2");
                StackObjectView s2{ l };
                ASSERT_FALSE(s1 == s2);
                ASSERT_TRUE(s1 != s2);
                ASSERT_FALSE(s1 == nullptr);
                ASSERT_TRUE(s1 != nullptr);
                ASSERT_FALSE(s1.RawEqual(s2));
            });
        CheckStackChange(0, {
                StackObjectView s1{ l };
                ASSERT_TRUE(s1 > 3);
                ASSERT_TRUE(s1 < 5);
                ASSERT_TRUE(s1 >= 4);
                ASSERT_TRUE(s1 <= 4);
                ASSERT_FALSE(s1 < 4);
                ASSERT_FALSE(s1 > 4);
                ASSERT_FALSE(s1 > 5);
                ASSERT_FALSE(s1 < 3);
            });
        CheckStackChange(0, {
                StackObjectView s1{ l };
                ASSERT_EQ(s1.RawLen(), 0);
                ASSERT_EQ(s1.Len<int>(), 4);
            });
        CheckStackChange(2, {
                StackObjectView s1{ l };
                auto s3 = s1.Get("__index");
                ASSERT_FALSE(s3 == nullptr);
                auto s4 = s1.RawGet("__index");
                ASSERT_TRUE(s4 == nullptr);
            });
        CheckStackChange(1, {
                StackObjectView s1{ l };
                auto s2 = s1.GetMetaTable();
                ASSERT_TRUE(s2.Is<Type::Table>());
            });
        CheckStackChange(3, {
                StackObjectView s1{ l };
                auto s2 = s1.GetMetaTable();
                auto s3 = s2.Get("__index");
                ASSERT_FALSE(s3 == nullptr);
                auto s4 = s2.RawGet("__index");
                ASSERT_FALSE(s4 == nullptr);
                ASSERT_TRUE(s4 == s3);
            });
    }
    { // get and set
        const StackTopRestorer rst{ l };
        lua_createtable(l, 0, 0);
        CheckStackChange(0, {
                StackObjectView s1{ l };
                s1.RawSetI(1, 10);
            });
        CheckStackChange(1, {
                StackObjectView s1{ l };
                auto s2 = s1.RawGetI(1);
                ASSERT_TRUE(s2.Is<int>());
                ASSERT_EQ(s2, 10);
            });
        CheckStackChange(0, {
                StackObjectView s1{ l };
                s1.SetI(1, 20);
            });
        CheckStackChange(1, {
                StackObjectView s1{ l };
                auto s2 = s1.GetI(1);
                ASSERT_TRUE(s2.Is<int>());
                ASSERT_EQ(s2, 20);
            });
        CheckStackChange(0, {
                StackObjectView s1{ l };
                s1.RawSet("a", 30);
            });
        CheckStackChange(1, {
                StackObjectView s1{ l };
                auto s2 = s1.RawGet("a");
                ASSERT_TRUE(s2.Is<int>());
                ASSERT_EQ(s2, 30);
            });
        CheckStackChange(0, {
                StackObjectView s1{ l };
                s1.Set("b", 40);
            });
        CheckStackChange(1, {
                StackObjectView s1{ l };
                auto s2 = s1.Get("b");
                ASSERT_TRUE(s2.Is<int>());
                ASSERT_EQ(s2, 40);
            });

    }

#undef CheckStackChange
}



TEST_F(StackObjectViewTest, TestAsArg)
{
    using namespace LTL;
    using namespace std;

    constexpr auto stack_f = +[](StackObjectView v) {
        v.Set("hello", "world");
        return v;
        };

    {
        RegisterFunction(l, "Test", CFunction<stack_f, StackObjectView>::Function);
        Run("result = Test({})");
        ASSERT_TRUE(Result().IsTable());
        ASSERT_TRUE(Result()["hello"].Is<string>());
        ASSERT_EQ(Result()["hello"].To<string>(), "world");
    }
}