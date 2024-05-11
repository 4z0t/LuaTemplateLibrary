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
        lua_State* m_state;
        std::string m_reason;
    };
}