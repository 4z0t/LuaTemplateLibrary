#pragma once
#include "LuaAux.hpp"
#include "UserData.hpp"
#include "Function.hpp"
#include "State.hpp"
#include "ClassConstructor.hpp"
#include "Property.hpp"
#include "StackObject.hpp"

namespace LTL
{
    /**
     * @brief Метаметоды ВМ Lua.
     * 
     */
    namespace MetaMethods
    {
        constexpr MetaMethodName index{ "__index" };
        constexpr MetaMethodName newindex{ "__newindex" };
        constexpr MetaMethodName call{ "__call" };
        constexpr MetaMethodName metatable{ "__metatable" };
        constexpr MetaMethodName mode{ "__mode" };
        constexpr MetaMethodName gc{ "__gc" };

        constexpr MetaMethodName add{ "__add" };
        constexpr MetaMethodName sub{ "__sub" };
        constexpr MetaMethodName mul{ "__mul" };
        constexpr MetaMethodName div{ "__div" };
        constexpr MetaMethodName mod{ "__mod" };
        constexpr MetaMethodName pow{ "__pow" };
        constexpr MetaMethodName unm{ "__unm" };
        constexpr MetaMethodName idiv{ "__idiv" };

        constexpr MetaMethodName  band{ "__band" };
        constexpr MetaMethodName  bor{ "__bor" };
        constexpr MetaMethodName  bxor{ "__bxor" };
        constexpr MetaMethodName  bnot{ "__bnot" };
        constexpr MetaMethodName  shl{ "__shl" };
        constexpr MetaMethodName  shr{ "__shr" };

        constexpr MetaMethodName  eq{ "__eq" };
        constexpr MetaMethodName  lt{ "__lt" };
        constexpr MetaMethodName  le{ "__le" };

        constexpr MetaMethodName len{ "__len" };
        constexpr MetaMethodName tostring{ "__tostring" };
        constexpr MetaMethodName concat{ "__concat" };
    }


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
        using UDVW = UserDataValueClassWrapper<C>;

        template<class C, typename T>
        using AUDVW = typename UDVW<C>::template AddUserDataValue<T>;

        template<class C, typename T>
        using AUDVW_t = typename AUDVW<C, T>::type;

        template <auto fn>
        using DeduceClass_t = typename FuncUtility::DeduceClass<decltype(fn)>::type;
    }

    /**
     * @brief Класс универсальной функции, преобразующий методы пользовательских типов 
     * для использования с UserData.
     * Оборачивает пользовательские типы в UserData
     * 
     * @tparam fn 
     * @tparam TArgs 
     */
    template<auto fn, typename ...TArgs>
    class Method :
        public CFunction<fn, UserData<Internal::DeduceClass_t<fn>>, Internal::AUDVW_t<Internal::DeduceClass_t<fn>, TArgs>...>,
        public Internal::MethodBase
    {
    public:
        using TClass = Class<Internal::DeduceClass_t<fn>>;

        Method() = default;
    };


    template<auto fn, typename TReturn, typename ...TArgs>
    class Method<fn, TReturn(TArgs...)> :
        public CFunction<fn, Internal::AUDVW_t<Internal::DeduceClass_t<fn>, TReturn>(UserData<Internal::DeduceClass_t<fn>>, Internal::AUDVW_t<Internal::DeduceClass_t<fn>, TArgs>...)>,
        public Internal::MethodBase
    {
    public:
        using TClass = Class<Internal::DeduceClass_t<fn>>;

        Method() = default;
    };

    /**
     * @brief Класс для добавления пользовательского класса в ВМ Lua.
     * 
     * @tparam T 
     */
    template<typename T>
    struct Class
    {
        using UData = UserData<T>;

        Class(lua_State* l, const char* name) :m_state(l), m_name(name)
        {
            MakeClassTable();
            MakeMetaTable();
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                Add(MetaMethods::gc, UData::DestructorFunction);
            }
            UData::MetaTable::Push(m_state);
            StackObjectView metatable{ m_state };
            metatable.RawSet(MetaMethods::metatable, true);
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
            Pop();
            return *this;
        }

        Class& AddSetter(const char* key, lua_CFunction func)
        {
            MakeNewIndexTable();
            UData::NewIndexTable::Push(m_state);
            RawSetFunction(key, func);
            Pop();
            return *this;
        }

        Class& AddMetaMethod(const char* name, lua_CFunction func)
        {
            UData::MetaTable::Push(m_state);
            RawSetFunction(name, func);
            Pop();
            return *this;
        }

        Class& AddMethod(const char* name, lua_CFunction func)
        {
            UData::MethodsTable::Push(m_state);
            RawSetFunction(name, func);
            Pop();
            return *this;
        }

        Class& SetIndexFunction(lua_CFunction func)
        {
            return Add(MetaMethods::index, func);
        }

        Class& SetNewIndexFunction(lua_CFunction func)
        {
            return Add(MetaMethods::newindex, func);
        }

        Class& Add(const MetaMethodName& key, lua_CFunction func)
        {
            return AddMetaMethod(key.method, func);
        }

        Class& Add(const char* key, lua_CFunction func)
        {
            return AddMethod(key, func);
        }


        template<bool condition>
        using EnableIf = std::enable_if_t<condition, Class&>;

        template<typename Base, typename Derived>
        static constexpr bool BaseOf = std::is_base_of_v<Base, Derived>;

        template<typename Element>
        EnableIf<BaseOf<PropertyBase, Element>> Add(const char* name, const Element&)
        {
            using GetterClass = typename Element::GetterClass;
            using SetterClass = typename Element::SetterClass;

            AddGetter(name, GetterClass{});
            AddSetter(name, SetterClass{});
            return *this;
        }

        template<typename Element>
        EnableIf<BaseOf<GetterBase, Element >> Add(const char* name, const Element& element)
        {
            return AddGetter(name, element);
        }

        template<typename Element>
        EnableIf<BaseOf<SetterBase, Element >> Add(const char* name, const Element& element)
        {
            return AddSetter(name, element);
        }

        template<typename Element>
        EnableIf<BaseOf<Internal::CFunctionBase, Element> && !BaseOf<Internal::MethodBase, Element>> Add(const char* name, const Element&)
        {
            static_assert(Element::template ValidUpvalues<>::value, "Methods dont support upvalues");
            return AddMethod(name, Element::Function);
        }

        template<typename Element>
        EnableIf<BaseOf<Internal::CFunctionBase, Element> && !BaseOf<Internal::MethodBase, Element>> Add(const MetaMethodName& name, const Element&)
        {
            static_assert(Element::template ValidUpvalues<>::value, "Methods dont support upvalues");
            return Add(name, Element::Function);
        }

        template<typename Element>
        EnableIf<BaseOf<Internal::MethodBase, Element>> Add(const MetaMethodName& name, const Element&)
        {
            static_assert(Element::template ValidUpvalues<>::value, "Methods dont support upvalues");
            static_assert(std::is_same_v<typename Element::TClass, Class>, "Method must be of the same class");
            return Add(name, Element::Function);
        }

        template<typename Element>
        EnableIf<BaseOf<Internal::MethodBase, Element>> Add(const char* name, const Element&)
        {
            static_assert(Element::template ValidUpvalues<>::value, "Methods dont support upvalues");
            static_assert(std::is_same_v<typename Element::TClass, Class>, "Method must be of the same class");
            return AddMethod(name, Element::Function);
        }

        template<typename Element>
        EnableIf<BaseOf<GetterBase, Element>> AddGetter(const char* key, const Element&)
        {
            static_assert(std::is_same_v<T, typename Element::TClass>, "Getter must be of the same class");
            AddGetter(key, Element::Function);
            return *this;
        }

        template<typename Element>
        EnableIf<BaseOf< SetterBase, Element>> AddSetter(const char* key, const Element&)
        {
            static_assert(std::is_same_v<T, typename Element::TClass>, "Setter must be of the same class");
            AddSetter(key, Element::Function);
            return *this;
        }

        template<typename Element>
        EnableIf<BaseOf< Internal::CFunctionBase, Element>> AddGetter(const char* key, const Element&)
        {
            static_assert(Element::min_arg_count <= 1, "Getter can't receive more than 1 argument");
            AddGetter(key, Element::Function);
            return *this;
        }

        template<typename Element>
        EnableIf<BaseOf< Internal::CFunctionBase, Element>> AddSetter(const char* key, const Element&)
        {
            static_assert(Element::min_arg_count <= 2, "Setter can't receive more than 2 arguments");
            AddSetter(key, Element::Function);
            return *this;
        }



    private:
        inline void Pop(int n = 1)const
        {
            lua_pop(m_state, n);
        }

        void RawSetFunction(const char* name, lua_CFunction func)
        {
            StackObjectView table{ m_state };
            table.RawSet(name, func);
        }

        void MakeMetaTable()
        {
            if (UData::MetaTable::Push(m_state) != LUA_TNIL)
            {
                Pop();
                return;
            }
            Pop();
            lua_newtable(m_state);
            StackObjectView metaTable{ m_state };
            lua_newtable(m_state);
            StackObjectView methodsTable{ m_state };
            metaTable.RawSet(MetaMethods::index, methodsTable);
            lua_setregp(m_state, UData::MethodsTable::GetKey());
            lua_setregp(m_state, UData::MetaTable::GetKey());
        }

        void MakeClassTable()
        {
            if (UData::ClassTable::Push(m_state) != LUA_TNIL)
            {
                Pop();
                return;
            }
            lua_newtable(m_state);
            StackObjectView classTable{ m_state };
            classTable.RawSet("className", m_name);
            lua_setregp(m_state, UData::ClassTable::GetKey());
            Pop();
        }

        void MakeIndexTable()
        {
            if (UData::IndexTable::Push(m_state) != LUA_TNIL)
            {
                Pop();
                return;
            }

            SetIndexFunction(UData::IndexMethod);

            lua_newtable(m_state);
            lua_setregp(m_state, UData::IndexTable::GetKey());
            Pop();
        }

        void MakeNewIndexTable()
        {
            if (UData::NewIndexTable::Push(m_state) != LUA_TNIL)
            {
                Pop();
                return;
            }

            SetNewIndexFunction(UData::NewIndexMethod);

            lua_newtable(m_state);
            lua_setregp(m_state, UData::NewIndexTable::GetKey());
            Pop();
        }

        lua_State* m_state;
        std::string m_name;
    };
}