﻿#pragma once
#include "LuaAux.hpp"
#include "LuaTypes.hpp"

namespace LTL
{
    template <typename RefAccess = RefGlobalAccess>
    class Ref;

    /**
     * @brief Класс контекста для Ref. Совмещает в себе состояние ВМ и ссылку на объект.
     * Уничтожение объекта должно быть выполнено вручную.
     * @see Ref
     * @tparam RefAccess
     */
    template <typename RefAccess = RefGlobalAccess>
    class ContextRef
    {
    public:
        using RefClass = Ref<RefAccess>;
        friend class Ref<RefAccess>;

        ContextRef() = delete;

        ContextRef(lua_State* l, const RefClass& ref) : m_state{ l }, m_ref{ ref } {}

        ContextRef(const ContextRef& other) : m_state{ other.m_state }, m_ref{ other.m_ref } {}

        ContextRef(ContextRef&& other) : m_state{ other.m_state }, m_ref{ std::move(other.m_ref) } {}

        ContextRef& operator=(const ContextRef& other)
        {
            m_state = other.m_state;
            m_ref = other.m_ref;
            return *this;
        }

        ContextRef& operator=(ContextRef&& other)
        {
            m_state = other.m_state;
            m_ref = std::move(other.m_ref);
            return *this;
        }

        void Release()
        {
            m_ref.Unref(m_state);
        }

        ~ContextRef() = default;

    private:
        lua_State* m_state = nullptr;
        RefClass m_ref{};
    };

    /**
     * @brief Класс ссылки на объект внутри ВМ. представляет из себя только ссылку на объект без самой ВМ.
     * Уничтожение объекта должно быть выполнено вручную. Имеется класс-обертка для работы со ссылаемым объектом.
     * @see ContextRef
     * @tparam RefAccess
     */
    template <typename RefAccess = RefGlobalAccess>
    class Ref
    {
    public:
        Ref() {}

        Ref(lua_State* l, int index)
        {
            lua_pushvalue(l, index);
            _FromTop(l);
        }

        Ref(const ContextRef<RefAccess>& sref) : Ref(sref.m_ref) {}

        Ref(const Ref& other) noexcept : m_ref{ other.m_ref } {}

        Ref(Ref&& other) noexcept : m_ref{ other.m_ref }
        {
            other.NullRef();
        }

        Ref& operator=(const ContextRef<RefAccess>& sref)
        {
            *this = sref.m_ref;
            return *this;
        }

        Ref& operator=(const Ref& other)
        {
            m_ref = other.m_ref;
            return *this;
        }

        Ref& operator=(Ref&& ref) noexcept
        {
            m_ref = ref.m_ref;
            ref.NullRef();
            return *this;
        }

        void Push(lua_State* l) const
        {
            RefAccess::PushRef(l, m_ref);
        }

        void Unref(lua_State* l)
        {
            RefAccess::Unref(l, m_ref);
            NullRef();
        }

        void Duplicate(lua_State* l, const Ref& other)
        {
            _FromRef(l, other);
        }

        ~Ref() = default;

    private:
        void _FromRef(lua_State* l, const Ref& other)
        {
            other.Push(l);
            _FromTop(l);
        }

        void _FromTop(lua_State* l)
        {
            m_ref = RefAccess::GetRef(l);
        }

        void NullRef()
        {
            m_ref = LUA_NOREF;
        }

        int m_ref = LUA_NOREF;
    };

}