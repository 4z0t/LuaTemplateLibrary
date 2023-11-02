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
        struct AddUserDataValue<C> : TypeBase<UserData<C>> {};

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
            auto method = Closure<fn, UserData<C>, AUDV_t<TArgs>...>::Function;
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
            auto method = Closure<fn, AUDV_t<TReturn>(UserData<C>, AUDV_t<TArgs>...)>::Function;
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
            RegisterFunction(m_state, m_name, Constructor<T, TArgs...>::Function);
            return *this;
        }

        template<typename TMethod>
        Class& AddMethod(const char* name, const TMethod&)
        {
            TMethod::AddMethod(*this, name);

            return *this;
        }

        template<typename TProperty>
        Class& AddProperty(const char* name, const TProperty&)
        {
            return AddGetter(name, TProperty::Get)
                .AddSetter(name, TProperty::Set);
        }

        Class& AddGetter(const char* name, lua_CFunction func)
        {
            MakeIndexTable();
            UData::IndexTable::Push(m_state);
            RawSetFunction(name, func);
            return *this;
        }

        Class& AddSetter(const char* name, lua_CFunction func)
        {
            MakeNewIndexTable();
            UData::NewIndexTable::Push(m_state);
            RawSetFunction(name, func);
            return *this;
        }

        Class& AddMetaMethod(const char* name, lua_CFunction func)
        {
            UData::MetaTable::Push(m_state);
            RawSetFunction(name, func);
            return *this;
        }

        Class& SetIndexFunction(lua_CFunction func)
        {
            return this->AddMetaMethod("__index", func);
        }

        Class& SetNewIndexFunction(lua_CFunction func)
        {
            return this->AddMetaMethod("__newindex", func);
        }

        template<typename Base, typename Derived>
        using EnableIfBaseOf = std::enable_if_t<std::is_base_of_v<Base, Derived>, Class&>;

        template<typename Element>
        EnableIfBaseOf<MethodBase, Element> Add(const char* name, const Element& element)
        {
            return this->AddMethod(name, element);
        }

        template<typename Element>
        EnableIfBaseOf<PropertyBase, Element> Add(const char* name, const Element& element)
        {
            return this->AddProperty(name, element);
        }

        template<typename Element>
        EnableIfBaseOf<GetterBase, Element> Add(const char* name, const Element& element)
        {
            return AddGetter(name, Element::Function);
        }

        template<typename Element>
        EnableIfBaseOf<SetterBase, Element> Add(const char* name, const Element& element)
        {
            return AddSetter(name, Element::Function);
        }

    private:

        void RawSetFunction(const char* name, lua_CFunction func)
        {
            lua_pushstring(m_state, name);
            lua_pushcfunction(m_state, func);
            lua_rawset(m_state, -3);
            lua_pop(m_state, 1);
        }

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