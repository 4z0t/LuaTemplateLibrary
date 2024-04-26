#pragma once
#include "LuaAux.hpp"

namespace LTL
{
    /**
     * @brief Класс для исключений, возникающих внутри Lua.
     *
     */
    class Exception
    {
    public:
        Exception(lua_State* l) :m_reason{ GetReason(l) }, m_state(l) {}

        lua_State* GetState()const noexcept
        {
            return m_state;
        }

        const std::string& GetReason()const noexcept
        {
            return m_reason;
        }

        static int PanicFunc(lua_State* l) throw(Exception)
        {
            throw Exception(l);
        }
    private:
        static const char* GetReason(lua_State* l)
        {
            const char* reason;
            if (lua_gettop(l) > 0)
            {
                reason = lua_tostring(l, -1);
                lua_pop(l, 1);
            }
            else
            {
                reason = "Unknown reason";
            }
            return reason;
        }
        lua_State* m_state;
        std::string m_reason;
    };
}