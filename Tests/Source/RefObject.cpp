#include "Lua/LuaLibrary.h"
#include "LuaTemplateLibrary/LTL.hpp"
#include <gtest/gtest.h>

struct TestBase : public testing::Test
{

    lua_State* l = nullptr;

    void SetUp() override
    {
        l = nullptr;
        l = luaL_newstate();
        luaL_openlibs(l);
    }

    void TearDown() override
    {
        if (l != nullptr)
        {
            lua_close(l);
        }
    }
};