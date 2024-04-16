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
        Exception(lua_State* l, int index) :m_reason{ GetReason(l, index) }, m_state(l) {}

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
            throw Exception(l, -1);
        }
    private:
        static const char* GetReason(lua_State* l, int index)
        {
            const char* reason = lua_tostring(l, index);
            return reason ? reason : "Unknown reason";
        }
        lua_State* m_state;
        std::string m_reason;
    };
}