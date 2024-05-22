#pragma once
#include "LuaAux.hpp"

namespace LTL
{
    /**
     * @brief Класс для исключений, возникающих внутри Lua.
     *
     */
    class Exception : public std::exception
    {
        using _Base = std::exception;
    public:
        Exception(lua_State* l) :_Base(GetReason(l)), m_state(l) {}

        lua_State* GetState()const noexcept
        {
            return m_state;
        }

        static int PanicFunc(lua_State* l) noexcept(false)
        {
            throw Exception(l);
        }
    private:
        static const char* GetReason(lua_State* l)
        {
            const char* reason = nullptr;
            if (lua_gettop(l) > 0)
            {
                reason = lua_tostring(l, -1);
                lua_pop(l, 1);
            }
            return reason ? reason : "Unknown error";
        }
        lua_State* const m_state;
    };
}