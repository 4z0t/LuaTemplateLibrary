#pragma once
#include "LuaAux.hpp"
#include "Types.hpp"
#include "CState.hpp"

namespace LTL
{
    template<typename T>
    class State;

    /**
     * @brief Представляет ООП доступ к стеку Lua.
     * После назначения индекса объекта на стеке с ним можно работать.
     * Очистка стека должна выполняться вручную.
     */
    class StackObjectView
    {
    public:
#pragma region Ctors

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
        StackObjectView(CState* cstate, int index = -1) : StackObjectView(cstate->Unwrap(), index) {}
        template<typename T>
        StackObjectView(const State<T>& state, int index = -1) : StackObjectView(state.GetState(), index) {}

        StackObjectView(const StackObjectView& other) = default;
        StackObjectView(StackObjectView&& other) = default;
        StackObjectView& operator=(const StackObjectView& other) = default;
        StackObjectView& operator=(StackObjectView&& other) = default;

#pragma endregion

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
            return Type() == LType;
        }

        /**
         * @brief Возвращает тип объекта на стеке
         *
         * @return Type
         */
        Type Type() const
        {
            return static_cast<LTL::Type>(lua_type(m_state, m_index));
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

        const char* ToString()const
        {
            const char* s = luaL_tolstring(m_state, m_index, nullptr);
            lua_pop(m_state, 1);
            return s;
        }

        friend static std::ostream& operator<<(std::ostream& os, const StackObjectView& obj)
        {
            return os << obj.ToString();
        }

#pragma region Comparison operators

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
            return Compare<LUA_OPEQ>(value);
        }

        template<>
        bool operator==(const std::nullptr_t&)const
        {
            return Type() == Type::Nil;
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
        * @brief Результат сравнения объектов с использованием метаметода
        *
        * @tparam T Тип сравниваемого значения.
        * @param value сравниваемое значение.
        * @return true Объекты равны с точки зрения Lua
        * @return false Объекты неравны с точки зрения Lua
        */
        template <typename T>
        bool operator<=(const T& value) const
        {
            return Compare<LUA_OPLE>(value);
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
        bool operator>(const T& value) const
        {
            return !(*this <= value);
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
        bool operator<(const T& value) const
        {
            return Compare<LUA_OPLT>(value);
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
        bool operator>=(const T& value) const
        {
            return !(*this < value);
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

#pragma endregion

#pragma region Get/Set methods

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
          * @brief Помещает на стек значение по заданному индексу с вызовом метаметода
          *
          * @param key
          * @return StackObjectView
          */
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
         * @brief Устанавливает по индексу значение на объект с вызовом метаметода
         *
         * @tparam V
         * @param i
         * @param value
         */
        template <typename V>
        void SetI(lua_Integer i, const V& value) const
        {
            PushValue(m_state, value);
            lua_seti(m_state, m_index, i);
        }

#pragma endregion

#pragma region Raw Get/Set methods

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
        * @brief Помещает на стек значение по индексу без вызова метаметода
        *
        * @param i
        * @return StackObjectView
        */
        StackObjectView RawGetI(lua_Integer i) const
        {
            lua_rawgeti(m_state, m_index, i);
            return { m_state };
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

#pragma endregion

#pragma region Len methods

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

#pragma endregion

#pragma region get/set metatable methods

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

#pragma endregion


#pragma region Call methods

        template<typename TReturn = void, typename ...TArgs>
        TReturn Call(TArgs&& ...args)const
        {
            return CallFunction<TReturn>(m_state, *this, std::forward<TArgs>(args)...);
        }

        template<typename TReturn = void, typename ...TArgs>
        PCallReturn<TReturn> PCall(TArgs&& ...args)const
        {
            return PCallFunction<TReturn>(m_state, *this, std::forward<TArgs>(args)...);
        }

        template<typename TReturn = void, typename ...TArgs>
        TReturn SelfCall(const char* key, TArgs&& ...args)const
        {
            Push();
            lua_getfield(m_state, -1, key);
            lua_rotate(m_state, -2, 1);
            const size_t n = PushArgs(m_state, std::forward<TArgs>(args)...) + 1;
            return CallStack<TReturn>(m_state, n);
        }

        template<typename TReturn = void, typename ...TArgs>
        PCallReturn<TReturn> SelfPCall(const char* key, TArgs&& ...args)const
        {
            Push();
            lua_getfield(m_state, -1, key);
            lua_rotate(m_state, -2, 1);
            const size_t n = PushArgs(m_state, std::forward<TArgs>(args)...) + 1;
            return PCallStack<TReturn>(m_state, n);
        }

#pragma endregion

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
    private:
        template <int COMPARE_OP, typename T>
        bool Compare(const T& value)const
        {
            PushValue(m_state, value);
            bool r = lua_compare(m_state, m_index, -1, COMPARE_OP);
            lua_pop(m_state, 1);
            return r;
        }

        template <int COMPARE_OP>
        bool Compare(const StackObjectView& other)const
        {
            return lua_compare(m_state, m_index, other.m_index, COMPARE_OP);
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