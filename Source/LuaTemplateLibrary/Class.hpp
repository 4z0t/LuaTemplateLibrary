#pragma once
#include "LuaAux.hpp"
#include "UserData.hpp"
#include "LuaFunctions.hpp"
#include "LuaState.hpp"

namespace Lua
{
    template<typename T>
    struct Class
    {
        using UData = UserData<T>;

        Class(lua_State* l) :m_state(l)
        {

        }

        Class(const State& state) : Class(state.GetState()->Unwrap()) {}



        template<auto fn, typename ...TArgs>
        Class& AddMethod(const char* name)
        {
            auto method = Closure<fn, UserDataValue<T>, TArgs...>::Function;
            lua_pushcfunction(m_state, method);
            return *this;
        }
    private:
        lua_State* m_state;
    };
}