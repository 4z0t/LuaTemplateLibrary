#pragma once
#include <gtest/gtest.h>
#include "Lua/LuaLibrary.h"
#include "LuaTemplateLibrary/LTL.hpp"

struct TestBase : public testing::Test
{

    lua_State* l = nullptr;

    void SetUp() override
    {
        l = nullptr;
        l = luaL_newstate();
        luaL_openlibs(l);
        lua_atpanic(l, LTL::Exception::PanicFunc);
    }

    void TearDown() override
    {
        if (l != nullptr)
        {
            lua_close(l);
        }
    }

    void Run(const std::string& s)
    {
        if (luaL_dostring(l, s.c_str()))
        {
            lua_error(l);
        }
    }

    LTL::GRefObject Result()
    {
        return LTL::GRefObject::Global(l, "result");
    }
};