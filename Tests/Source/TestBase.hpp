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
        lua_atpanic(l, Lua::Exception::PanicFunc);
    }

    void TearDown() override
    {
        if (l != nullptr)
        {
            lua_close(l);
        }
    }

    void Run(const std::string &s)
    {
        if (luaL_loadstring(l, s.c_str()) != 0)
        {
            throw std::runtime_error(lua_tostring(l, -1));
        }

        if (lua_pcall(l, 0, 0, 0) != 0)
        {
            throw std::runtime_error(lua_tostring(l, -1));
        }
    }

    Lua::GRefObject Result()
    {
        return Lua::GRefObject::Global(l, "result");
    }
};