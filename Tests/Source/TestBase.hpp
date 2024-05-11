#pragma once
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

    int Top()const
    {
        return lua_gettop(l);
    }

    LTL::GRefObject Result()
    {
        return LTL::GRefObject::Global(l, "result");
    }
};

//struct TestBase : public testing::Test
//{
//    void SetUp() override
//    {
//        //arranging the state and env
//    }
//
//    void TearDown() override
//    {
//        //destroying state and env
//    }
//};