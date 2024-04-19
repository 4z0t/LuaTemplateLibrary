#pragma once
#include "LuaAux.hpp"
#include "UserData.hpp"
#include "LuaFunctions.hpp"
#include "LuaState.hpp"
#include "ClassConstructor.hpp"
#include "Property.hpp"
#include "StackObject.hpp"

#define lua_regptr_isnt_set(l, p) assert(lua_getregp(l, p) == LUA_TNIL)

namespace LTL
{
    template<typename T>
    struct Class;
    namespace Internal
    {
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

        template<class C>
        using UDVW = typename UserDataValueClassWrapper<C>;

        template<class C, typename T>
        using AUDVW = typename UserDataValueClassWrapper<C>::AddUserDataValue<T>;

        template<class C, typename T>
        using AUDVW_t = typename AUDVW<C, T>::type;
    }

    template<class C, auto fn, typename ...TArgs>
    class Method :
        public CFunction<fn, UserData<C>, Internal::AUDVW_t<C, TArgs>...>,
        public Internal::MethodBase
    {
    public:
        using TClass = Class<C>;

        Method() = default;
    };


    template<class C, auto fn, typename TReturn, typename ...TArgs>
    class Method<C, fn, TReturn(TArgs...)> :
        public CFunction<fn, Internal::AUDVW_t<C, TReturn>(UserData<C>, Internal::AUDVW_t<C, TArgs>...)>,
        public Internal::MethodBase
    {
    public:
        using TClass = Class<C>;

        Method() = default;
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
            UData::MetaTable::Push(m_state);
            StackObjectView metatable{ m_state };
            metatable.RawSet("__metatable", false);
            lua_pop(m_state, 1);

        }

        template<typename TAlloc>
        Class(const State<TAlloc>& state, const char* name) : Class(state.GetState()->Unwrap(), name) {}

        template<typename ...TArgs>
        Class& AddConstructor()
        {
            RegisterFunction(m_state, m_name, Constructor<T, TArgs...>::Function);
            return *this;
        }

        Class& AddGetter(const char* key, lua_CFunction func)
        {
            MakeIndexTable();
            UData::IndexTable::Push(m_state);
            RawSetFunction(key, func);
            return *this;
        }

        Class& AddSetter(const char* key, lua_CFunction func)
        {
            MakeNewIndexTable();
            UData::NewIndexTable::Push(m_state);
            RawSetFunction(key, func);
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
        EnableIfBaseOf<PropertyBase, Element> Add(const char* name, const Element& element)
        {
            return AddGetter(name, Element::Getter{}).AddSetter(name, Element::Setter{});
        }

        template<typename Element>
        EnableIfBaseOf<GetterBase, Element> Add(const char* name, const Element& element)
        {
            return AddGetter(name, element);
        }

        template<typename Element>
        EnableIfBaseOf<SetterBase, Element> Add(const char* name, const Element& element)
        {
            return AddSetter(name, element);
        }

        template<typename Element>
        EnableIfBaseOf<Internal::CFunctionBase, Element> Add(const char* name, const Element& element)
        {
            static_assert(Element::ValidUpvalues<>::value, "Methods dont support upvalues");
            return AddMetaMethod(name, Element::Function);
        }

        template<typename Element>
        EnableIfBaseOf<GetterBase, Element> AddGetter(const char* key, const Element&)
        {
            static_assert(std::is_same_v<T, typename Element::TClass>, "Getter must be of the same class");
            AddGetter(key, Element::Function);
            return *this;
        }

        template<typename Element>
        EnableIfBaseOf<SetterBase, Element> AddSetter(const char* key, const Element&)
        {
            static_assert(std::is_same_v<T, typename Element::TClass>, "Setter must be of the same class");
            AddSetter(key, Element::Function);
            return *this;
        }

        template<typename Element>
        EnableIfBaseOf<Internal::CFunctionBase, Element> AddGetter(const char* key, const Element&)
        {
            static_assert(Element::min_arg_count <= 1, "Getter can't receive more than 1 argument");
            AddGetter(key, Element::Function);
            return *this;
        }

        template<typename Element>
        EnableIfBaseOf<Internal::CFunctionBase, Element> AddSetter(const char* key, const Element&)
        {
            static_assert(Element::min_arg_count <= 2, "Setter can't receive more than 2 argument");
            AddSetter(key, Element::Function);
            return *this;
        }



    private:

        void RawSetFunction(const char* name, lua_CFunction func)
        {
            StackObjectView table{ m_state };
            table.RawSet(name, func);
            lua_pop(m_state, 1);
        }

        void MakeMetaTable()
        {
            lua_regptr_isnt_set(m_state, UData::MetaTable::GetKey());

            lua_newtable(m_state);
            StackObjectView metaTable{ m_state };
            metaTable.RawSet("__index", metaTable);
            lua_setregp(m_state, UData::MetaTable::GetKey());
        }

        void MakeClassTable()
        {
            lua_regptr_isnt_set(m_state, UData::ClassTable::GetKey());

            lua_newtable(m_state);
            StackObjectView classTable{ m_state };
            classTable.RawSet("className", m_name);
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