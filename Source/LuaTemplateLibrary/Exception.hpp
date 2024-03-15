#pragma once
#include "LuaAux.hpp"

namespace LTL
{
    class Exception : public std::runtime_error
    {
    public:
        Exception(lua_State* l, int index) :std::runtime_error(GetReason(l, index)), m_state(l) {}

        lua_State* GetState()const
        {
            return m_state;
        }

        static int PanicFunc(lua_State* l)
        {
            throw Exception(l, -1);
        }
    private:
        static const char* GetReason(lua_State* l, int index)
        {
            const char* reason = lua_tostring(l, index);
            return reason ? reason : "Unknown reason";
        }
        lua_State* m_state;
    };
}