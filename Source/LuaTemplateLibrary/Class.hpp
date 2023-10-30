#pragma once
#include "LuaAux.hpp"
#include "UserData.hpp"
#include "LuaFunctions.hpp"
#include "LuaState.hpp"
#include "ClassConstructor.hpp"

namespace Lua
{
    template<typename T>
    struct Class;

    template<class C>
    struct UserDataValueClassWrapper
    {
        template<typename T>
        struct AddUserDataValue : TypeBase<T> {};

        template<>
        struct AddUserDataValue<C> : TypeBase<UserDataValue<C>> {};

        template<typename T>
        using AUDV_t = typename AddUserDataValue<T>::type;
    };

    template<class C, auto fn, typename ...TArgs>
    class Method : private UserDataValueClassWrapper<C>
    {
    public:
        using TClass = Class<C>;
        using UserDataValueClassWrapper::AUDV_t;

        Method() = default;

        static void AddMethod(TClass& c, const char* name)
        {
            auto method = Closure<fn, UserDataValue<C>, AUDV_t<TArgs>...>::Function;
            c.AddMetaMethod(name, method);
        }
    };


    template<class C, auto fn, typename TReturn, typename ...TArgs>
    class Method<C, fn, TReturn(TArgs...)> : private UserDataValueClassWrapper<C>
    {
    public:
        using TClass = Class<C>;
        using UserDataValueClassWrapper::AUDV_t;

        Method() = default;

        static void AddMethod(TClass& c, const char* name)
        {
            auto method = Closure<fn, AUDV_t<TReturn>(UserDataValue<C>, AUDV_t<TArgs>...)>::Function;
            c.AddMetaMethod(name, method);
        }
    };

    template<typename T>
    struct Class
    {
        using UData = UserData<T>;

        Class(lua_State* l, const char* name) :m_state(l), m_name(name)
        {
            MakeMetaTable();
            MakeClassTable();
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                //std::cout << "Assigning dtor for " << typeid(T).name() << std::endl;
                AddMetaMethod("__gc", UData::DestructorFunction);
            }
        }

        Class(const State& state, const char* name) : Class(state.GetState()->Unwrap(), name) {}

        template<typename TMethod>
        Class& AddMethod(const char* name, const TMethod&)
        {
            TMethod::AddMethod(*this, name);

            return *this;
        }



        void AddMetaMethod(const char* name, lua_CFunction func)
        {
            UData::PushMetaTable(m_state);
            lua_pushstring(m_state, name);
            lua_pushcfunction(m_state, func);
            lua_rawset(m_state, -3);
            lua_pop(m_state, 1);
        }

        template<typename ...TArgs>
        Class& AddConstructor()
        {
            RegisterFunction(m_state, m_name.c_str(), Constructor<T, TArgs...>::Function);
            return *this;
        }

    private:

        void MakeCtor()
        {
            RegisterFunction(m_state, m_name.c_str(), Constructor<T>::Function);
        }

        void MakeMetaTable()
        {
            lua_newtable(m_state);
            lua_pushstring(m_state, "__index");
            lua_pushvalue(m_state, -2);
            lua_rawset(m_state, -3);
            lua_setregp(m_state, UData::GetMetaTableKey());
        }

        void MakeClassTable()
        {
            lua_newtable(m_state);
            lua_pushstring(m_state, "className");
            lua_pushstring(m_state, m_name.c_str());
            lua_rawset(m_state, -3);
            lua_setregp(m_state, UData::GetClassTableKey());
        }

        lua_State* m_state;
        std::string m_name;
    };
}