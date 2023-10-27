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

        Class(lua_State* l, const char* name) :m_state(l), m_name(name)
        {
            MakeCtor();
            MakeMetaTable();
            MakeClassTable();
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                //std::cout << "Assigning dtor for " << typeid(T).name() << std::endl;
                AddMetaMethod("__gc", UData::DestructorFunction);
            }
        }

        Class(const State& state, const char* name) : Class(state.GetState()->Unwrap(), name) {}



        template<auto fn, typename ...TArgs>
        Class& AddMethod(const char* name)
        {
            auto method = Closure<fn, UserDataValue<T>, TArgs...>::Function;

            AddMetaMethod(name, method);

            return *this;
        }
    private:

        void AddMetaMethod(const char* name, lua_CFunction func)
        {
            UData::PushMetaTable(m_state);
            lua_pushstring(m_state, name);
            lua_pushcfunction(m_state, func);
            lua_rawset(m_state, -3);
            lua_pop(m_state, 1);
        }

        void MakeCtor()
        {
            RegisterFunction(m_state, m_name.c_str(), UData::ConstructorFunction);
        }

        void MakeMetaTable()
        {
            lua_newtable(m_state);
            lua_pushstring(m_state, "__index");
            lua_pushvalue(m_state, -2);
            lua_rawset(m_state, -3);
            lua_rawsetp(m_state, LUA_REGISTRYINDEX, UData::GetMetaTableKey());
        }

        void MakeClassTable()
        {
            lua_newtable(m_state);
            lua_rawsetp(m_state, LUA_REGISTRYINDEX, UData::GetClassTableKey());
        }

        lua_State* m_state;
        std::string m_name;
    };
}