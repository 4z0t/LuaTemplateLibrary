#pragma once
#include "LuaAux.hpp"
#include "UserData.hpp"
#include "LuaFunctions.hpp"
#include "LuaState.hpp"
#include "ClassConstructor.hpp"
#include "Property.hpp"

#define lua_regptr_isnt_set(l, p) assert(lua_getregp(l, p) == LUA_TNIL)

namespace Lua
{
    template<typename T>
    struct Class;

    struct MethodBase {};

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
    class Method : private UserDataValueClassWrapper<C>, public MethodBase
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
    class Method<C, fn, TReturn(TArgs...)> : private UserDataValueClassWrapper<C>, public MethodBase
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

        template<typename ...TArgs>
        Class& AddConstructor()
        {
            RegisterFunction(m_state, m_name.c_str(), Constructor<T, TArgs...>::Function);
            return *this;
        }

        template<typename TMethod>
        Class& AddMethod(const char* name, const TMethod&)
        {
            TMethod::AddMethod(*this, name);

            return *this;
        }

        template<typename Element>
        std::enable_if_t<std::is_base_of_v<MethodBase, Element>, Class&> Add(const char* name, const Element& element)
        {
            return this->AddMethod(name, element);
        }

        template<typename Element>
        std::enable_if_t<std::is_base_of_v<PropertyBase, Element>, Class&> Add(const char* name, const Element&)
        {
            AddGetter(name, Element::Get);
            AddSetter(name, Element::Set);
            return *this;
        }

        void AddGetter(const char* name, lua_CFunction func)
        {
            MakeIndexTable();
            UData::IndexTable::Push(m_state);
            PushValue(m_state, func);
            lua_setfield(m_state, -2, name);
            lua_pop(m_state, 1);
        }

        void AddSetter(const char* name, lua_CFunction func)
        {
            MakeNewIndexTable();
            UData::NewIndexTable::Push(m_state);
            PushValue(m_state, func);
            lua_setfield(m_state, -2, name);
            lua_pop(m_state, 1);
        }

        void AddMetaMethod(const char* name, lua_CFunction func)
        {
            UData::MetaTable::Push(m_state);
            lua_pushstring(m_state, name);
            lua_pushcfunction(m_state, func);
            lua_rawset(m_state, -3);
            lua_pop(m_state, 1);
        }

        Class& SetIndexFunction(lua_CFunction func)
        {
            UData::MetaTable::Push(m_state);
            lua_pushstring(m_state, "__index");
            lua_pushcfunction(m_state, func);
            lua_rawset(m_state, -3);
            lua_pop(m_state, 1);
            return *this;
        }

        Class& SetNewIndexFunction(lua_CFunction func)
        {
            UData::MetaTable::Push(m_state);
            lua_pushstring(m_state, "__newindex");
            lua_pushcfunction(m_state, func);
            lua_rawset(m_state, -3);
            lua_pop(m_state, 1);
            return *this;
        }

    private:

        void MakeMetaTable()
        {
            lua_regptr_isnt_set(m_state, UData::MetaTable::GetKey());

            lua_newtable(m_state);
            lua_pushstring(m_state, "__index");
            lua_pushvalue(m_state, -2);
            lua_rawset(m_state, -3);
            lua_setregp(m_state, UData::MetaTable::GetKey());
        }

        void MakeClassTable()
        {
            lua_regptr_isnt_set(m_state, UData::ClassTable::GetKey());

            lua_newtable(m_state);
            lua_pushstring(m_state, "className");
            lua_pushstring(m_state, m_name.c_str());
            lua_rawset(m_state, -3);
            lua_setregp(m_state, UData::ClassTable::GetKey());
        }

        void MakeIndexTable()
        {
            if (m_has_index_table)
                return;
            lua_regptr_isnt_set(m_state, UData::IndexTable::GetKey());

            SetIndexFunction(UData::IndexMethod);

            lua_newtable(m_state);
            lua_setregp(m_state, UData::IndexTable::GetKey());
            m_has_index_table = true;
        }

        void MakeNewIndexTable()
        {
            if (m_has_newindex_table)
                return;
            lua_regptr_isnt_set(m_state, UData::NewIndexTable::GetKey());

            SetNewIndexFunction(UData::NewIndexMethod);

            lua_newtable(m_state);
            lua_setregp(m_state, UData::NewIndexTable::GetKey());
            m_has_newindex_table = true;
        }

        lua_State* m_state;
        std::string m_name;
        bool m_has_index_table = false;
        bool m_has_newindex_table = false;
    };
}