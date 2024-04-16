#pragma once
#include "LuaAux.hpp"
#include "LuaState.hpp"
#include "LuaTypes.hpp"
#include "RefObject.hpp"

namespace LTL
{
    /**
     * @brief Представляет ООП доступ к стеку Lua.
     * После назначения индекса объекта на стеке с ним можно работать.
     * Очистка стека должна выполняться вручную.
     */
    class StackObjectView
    {
    public:
        /**
         * @brief Для внутреннего использования.
         * Не использовать самому.
         */
        StackObjectView() = default;

        /**
         * @brief Назначает индекс объекта на стеке.
         *
         * @param l Состояние ВМ Lua.
         * @param index Индекс объекта на стеке.
         */
        StackObjectView(lua_State *l, int index = -1) : m_state(l), m_index(lua_absindex(l, index)) {}

        StackObjectView(const StackObjectView &other) = default;
        StackObjectView(StackObjectView &&other) = default;
        StackObjectView &operator=(const StackObjectView &other) = default;
        StackObjectView &operator=(StackObjectView &&other) = default;

        /**
         * @brief Преобразует объект к данному типу.
         *
         * @tparam T Тип преобразования.
         * @return T Результат преобразования.
         */
        template <typename T>
        T To() const
        {
            return StackType<T>::Get(m_state, m_index);
        }

        /**
         * @brief Проверяет является ли объект заданного типа.
         *
         * @tparam LType Проверяемый тип.
         * @return true Объект относится к данному типу
         * @return false Объект не относится к данному типу
         */
        template <Type LType>
        bool Is() const
        {
            return lua_type(m_state, m_index) == static_cast<int>(LType);
        }
        /**
         * @brief Проверяет является ли объект заданного типа.
         *
         * @tparam T Проверяемый тип.
         * @return true Объект относится к данному типу
         * @return false Объект не относится к данному типу
         */
        template <typename T>
        bool Is() const
        {
            return StackType<T>::Check(m_state, m_index);
        }

        /**
         * @brief Помещает объект на стек.
         */
        void Push() const
        {
            lua_pushvalue(m_state, m_index);
        }

        /**
         * @brief Сравнивает объект и данное значение.
         * 
         * @tparam T Тип сравниваемого значения.
         * @param value сравниваемое значение.
         * @return true Объекты равны с точки зрения Lua
         * @return false Объекты неравны с точки зрения Lua
         */
        template <typename T>
        bool operator==(const T &value) const
        {
            PushValue(m_state, value);
            bool r = lua_compare(m_state, m_index, -1, LUA_OPEQ);
            lua_pop(m_state, 1);
            return r;
        }

        /**
         * @brief Сравнивает объекты.
         * 
         * @param value сравниваемый объект.
         * @return true Объекты равны с точки зрения Lua
         * @return false Объекты неравны с точки зрения Lua
         */
        template <>
        bool operator==(const StackObjectView &value) const
        {
            return m_state == value.m_state && lua_compare(m_state, m_index, value.m_index, LUA_OPEQ);
        }

        template <typename T>
        bool RawEqual(const T &value) const
        {
            PushValue(m_state, value);
            bool r = lua_rawequal(m_state, m_index, -1);
            lua_pop(m_state, 1);
            return r;
        }

        template <>
        bool RawEqual(const StackObjectView &value) const
        {
            return lua_rawequal(m_state, m_index, value.m_index);
        }

        template <typename T>
        bool operator!=(const T &value) const
        {
            return !(*this == value);
        }

        template <typename R, typename T>
        R Get(const T &key) const
        {
            PushValue(m_state, key);
            lua_gettable(m_state, m_index);
            R r = GetValue<R>(m_state, -1);
            lua_pop(m_state, 1);
            return r;
        }

        template <typename T>
        StackObjectView Get(const T &key) const
        {
            PushValue(m_state, key);
            lua_gettable(m_state, m_index);
            return {m_state};
        }

        template <typename K, typename V>
        void Set(const K &key, const V &value) const
        {
            PushValue(m_state, key);
            PushValue(m_state, value);
            lua_settable(m_state, m_index);
        }

        template <typename R, typename T>
        R RawGet(const T &key) const
        {
            PushValue(m_state, key);
            lua_rawget(m_state, m_index);
            R r = GetValue<R>(m_state, -1);
            lua_pop(m_state, 1);
            return r;
        }

        template <typename T>
        StackObjectView RawGet(const T &key) const
        {
            PushValue(m_state, key);
            lua_rawget(m_state, m_index);
            return {m_state};
        }

        template <typename R>
        R RawGetI(int i) const
        {
            lua_rawgeti(m_state, m_index, i);
            R r = GetValue<R>(m_state, -1);
            lua_pop(m_state, 1);
            return r;
        }

        template <typename V>
        void RawSetI(int i, const V &value) const
        {
            PushValue(m_state, value);
            lua_rawseti(m_state, m_index, i);
        }

        template <typename K, typename V>
        void RawSet(const K &key, const V &value) const
        {
            PushValue(m_state, key);
            PushValue(m_state, value);
            lua_rawset(m_state, m_index);
        }

        template <typename R>
        R Len() const
        {
            lua_len(m_state, m_index);
            R r = GetValue<R>(m_state, -1);
            lua_pop(m_state, 1);
            return r;
        }

        StackObjectView Len() const
        {
            lua_len(m_state, m_index);
            return {m_state};
        }

        size_t RawLen() const
        {
            return lua_rawlen(m_state, m_index);
        }

        StackObjectView GetMetaTable() const
        {
            if (!lua_getmetatable(m_state, m_index))
            {
                lua_pushnil(m_state);
            }
            return {m_state};
        }

        template <typename T>
        void SetMetaTable(const T &value) const
        {
            PushValue(m_state, value);
            lua_setmetatable(m_state, m_index);
        }

        /**
         * @brief Помещает на стек глобальное значение.
         * 
         * @param l Состояние ВМ Lua.
         * @param name Имя глобального значения.
         * @return StackObjectView 
         */
        static StackObjectView Global(lua_State *l, const char *name)
        {
            lua_getglobal(l, name);
            return StackObjectView{l};
        }

        ~StackObjectView() = default;

        /**
         * @brief Возвращает состояние ВМ Lua.
         * 
         * @return lua_State* const 
         */
        lua_State *const GetState() const
        {
            return m_state;
        }

        /**
         * @brief Возвращает индекс объекта на стеке.
         * 
         * @return int
         */
        const int GetIndex() const
        {
            return m_index;
        }

    protected:
        lua_State *m_state = nullptr;
        int m_index = 0;
    };

    /**
     * @brief Реализация преобразования типа на стеке для StackObjectView.
     * 
     * @see StackObjectView
     * @tparam StackObjectView
     */
    template <>
    struct StackType<StackObjectView>
    {
        static StackObjectView Get(lua_State *l, int index)
        {
            return {l, index};
        }

        static bool Check(lua_State *l, int index)
        {
            return !lua_isnone(l, index);
        }

        static void Push(lua_State *l, const StackObjectView &value)
        {
            assert(value.GetState() == l);
            value.Push();
        }
    };

}