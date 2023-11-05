#pragma once
#include "LuaAux.hpp"
#include "LuaState.hpp"
#include "LuaTypes.hpp"

namespace Lua
{
    class StackObject
    {
        StackObject(lua_State* l, int index) : m_state(l), m_index(lua_absindex(l, index))
        {

        }



    private:
        lua_State* const m_state;
        const int m_index;
    };
}