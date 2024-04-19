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
        StackObjectView(lua_State* l, int index = -1) : m_state(l), m_index(lua_absindex(l, index)) {}

        StackObjectView(const StackObjectView& other) = default;
        StackObjectView(StackObjectView&& other) = default;
        StackObjectView& operator=(const StackObjectView& other) = default;
        StackObjectView& operator=(StackObjectView&& other) = default;

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
         * @brief Результат сравнения объектов с использованием метаметода
         *
         * @tparam T Тип сравниваемого значения.
         * @param value сравниваемое значение.
         * @return true Объекты равны с точки зрения Lua
         * @return false Объекты неравны с точки зрения Lua
         */
        template <typename T>
        bool operator==(const T& value) const
        {
            PushValue(m_state, value);
            bool r = lua_compare(m_state, m_index, -1, LUA_OPEQ);
            lua_pop(m_state, 1);
            return r;
        }

        /**
         * @brief Результат сравнения объектов с использованием метаметода
         *
         * @param value сравниваемый объект.
         * @return true Объекты равны с точки зрения Lua
         * @return false Объекты неравны с точки зрения Lua
         */
        template <>
        bool operator==(const StackObjectView& value) const
        {
            return m_state == value.m_state && lua_compare(m_state, m_index, value.m_index, LUA_OPEQ);
        }

        /**
         * @brief Результат сравнения объектов с использованием метаметода
         *
         * @tparam T
         * @param value
         * @return true
         * @return false
         */
        template <typename T>
        bool operator!=(const T& value) const
        {
            return !(*this == value);
        }

        /**
         * @brief Результат сравнения объектов без вызова метаметода
         *
         * @tparam T
         * @param value
         * @return true
         * @return false
         */
        template <typename T>
        bool RawEqual(const T& value) const
        {
            PushValue(m_state, value);
            bool r = lua_rawequal(m_state, m_index, -1);
            lua_pop(m_state, 1);
            return r;
        }

        /**
         * @brief Результат сравнения объектов без вызова метаметода
         *
         * @tparam
         * @param value
         * @return true
         * @return false
         */
        template <>
        bool RawEqual(const StackObjectView& value) const
        {
            return lua_rawequal(m_state, m_index, value.m_index);
        }

        /**
         * @brief Возвращает значение по заданному ключу с вызовом метаметода
         *
         * @tparam R
         * @tparam T
         * @param key
         * @return R
         */
        template <typename R, typename T>
        R Get(const T& key) const
        {
            PushValue(m_state, key);
            lua_gettable(m_state, m_index);
            R r = GetValue<R>(m_state, -1);
            lua_pop(m_state, 1);
            return r;
        }

        /**
   * @brief Возвращает значение по ключу с вызовом метаметода
   *
   * @tparam T
   * @param key
   * @return StackObjectView
   */
        template <typename T>
        StackObjectView Get(const T& key) const
        {
            PushValue(m_state, key);
            lua_gettable(m_state, m_index);
            return { m_state };
        }

        /**
       * @brief Возвращает значение по заданному индексу с вызовом метаметода
       *
       * @tparam R
       * @param key
       * @return R
       */
        template <typename R>
        R GetI(lua_Integer index) const
        {
            lua_geti(m_state, m_index, index);
            R r = GetValue<R>(m_state, -1);
            lua_pop(m_state, 1);
            return r;
        }

        /**
          * @brief Возвращает значение по заданному индексу с вызовом метаметода
          *
          * @tparam R
          * @param key
          * @return R
          */
        template <>
        StackObjectView GetI(lua_Integer index) const
        {
            lua_geti(m_state, m_index, index);
            return { m_state };
        }

        /**
         * @brief Устанавливает по ключу значение на объект с вызовом метаметода
         *
         * @tparam K
         * @tparam V
         * @param key
         * @param value
         */
        template <typename K, typename V>
        void Set(const K& key, const V& value) const
        {
            PushValue(m_state, key);
            PushValue(m_state, value);
            lua_settable(m_state, m_index);
        }

        /**
         * @brief Возвращает значение по ключу без вызова метаметода
         *
         * @tparam R
         * @tparam T
         * @param key
         * @return R
         */
        template <typename R, typename T>
        R RawGet(const T& key) const
        {
            PushValue(m_state, key);
            lua_rawget(m_state, m_index);
            R r = GetValue<R>(m_state, -1);
            lua_pop(m_state, 1);
            return r;
        }

        /**
         * @brief Возвращает значение по ключу без вызова метаметода
         *
         * @tparam T
         * @param key
         * @return StackObjectView
         */
        template <typename T>
        StackObjectView RawGet(const T& key) const
        {
            PushValue(m_state, key);
            lua_rawget(m_state, m_index);
            return { m_state };
        }

        /**
         * @brief Возвращает значение по индексу без вызова метаметода
         *
         * @tparam R
         * @param i
         * @return R
         */
        template <typename R>
        R RawGetI(lua_Integer i) const
        {
            lua_rawgeti(m_state, m_index, i);
            R r = GetValue<R>(m_state, -1);
            lua_pop(m_state, 1);
            return r;
        }

        /**
         * @brief Устанавливает значение с заданным индексом на объект
         * без вызова метаметодов.
         *
         * @tparam V
         * @param i индекс
         * @param value значение
         */
        template <typename V>
        void RawSetI(lua_Integer i, const V& value) const
        {
            PushValue(m_state, value);
            lua_rawseti(m_state, m_index, i);
        }

        /**
         * @brief Устанавливает значение с заданным ключом на объект
         * без вызова метаметодов.
         *
         * @tparam K
         * @tparam V
         * @param key ключ
         * @param value значение
         */
        template <typename K, typename V>
        void RawSet(const K& key, const V& value) const
        {
            PushValue(m_state, key);
            PushValue(m_state, value);
            lua_rawset(m_state, m_index);
        }

        /**
         * @brief возвращает результат работы метаметода
         * __len, если таковой присутствует, в противном
         * случае возвращает результат RawLen.
         *
         * @return R
         */
        template <typename R>
        R Len() const
        {
            lua_len(m_state, m_index);
            R r = GetValue<R>(m_state, -1);
            lua_pop(m_state, 1);
            return r;
        }

        /**
         * @brief возвращает результат работы метаметода
         * __len, если таковой присутствует, в противном
         * случае возвращает результат RawLen.
         *
         * @return StackObjectView
         */
        StackObjectView Len() const
        {
            lua_len(m_state, m_index);
            return { m_state };
        }

        /**
         * @brief Возвращает необработанную
         * длину объекта.
         * @return size_t длина объекта
         */
        lua_Unsigned RawLen() const
        {
            return lua_rawlen(m_state, m_index);
        }

        /**
         * @brief Возвращает StackObjectView с метатаблицей объекта.
         * Если ее нет возвращает nil.
         * @return StackObjectView
         */
        StackObjectView GetMetaTable() const
        {
            if (!lua_getmetatable(m_state, m_index))
            {
                lua_pushnil(m_state);
            }
            return { m_state };
        }

        /**
         * @brief Устанавливает на объект данную метатаблицу
         *
         * @tparam T тип мететаблицы
         * @param value метатаблица
         */
        template <typename T>
        void SetMetaTable(const T& value) const
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
        static StackObjectView Global(lua_State* l, const char* name)
        {
            lua_getglobal(l, name);
            return StackObjectView{ l };
        }

        ~StackObjectView() = default;

        /**
         * @brief Возвращает состояние ВМ Lua.
         *
         * @return lua_State* const
         */
        lua_State* const GetState() const noexcept
        {
            return m_state;
        }

        /**
         * @brief Возвращает индекс объекта на стеке.
         *
         * @return int
         */
        const int GetIndex() const noexcept
        {
            return m_index;
        }

    protected:
        lua_State* m_state = nullptr;
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
        static StackObjectView Get(lua_State* l, int index)
        {
            return { l, index };
        }

        static bool Check(lua_State* l, int index)
        {
            return !lua_isnone(l, index);
        }

        static void Push(lua_State* l, const StackObjectView& value)
        {
            assert(value.GetState() == l);
            value.Push();
        }
    };

}