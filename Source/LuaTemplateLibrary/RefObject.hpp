#pragma once
#include "LuaAux.hpp"
#include "Types.hpp"
#include "StackObject.hpp"

namespace LTL
{
    template<typename T>
    class State;
    /**
     * @brief Базовый класс для классов-ссылок.
     * 
     * @tparam RefClass Класс реализующий интерфейс
     * @tparam ParentClass Главный класс
     * @tparam RefAccess Класс для доступа к объектам по ссылке
     * 
     * @see RefObject
     * @see RefTableEntryObject
     * @see RefGlobalAccess
     */
    template<typename RefClass, typename ParentClass, typename RefAccess>
    class RefObjectBase
    {
    public:
        RefObjectBase() = default;
        RefObjectBase(lua_State* l) noexcept : m_state(l) { assert(m_state, "Expected not null lua_State"); };
        template<typename T>
        RefObjectBase(const State<T>& state) noexcept : RefObjectBase(state.GetState()->Unwrap()) {};

        class Iterator
        {
        public:
            Iterator(const ParentClass& iterable) : m_table(iterable), m_key(iterable.GetState()) {}

            Iterator& Next()
            {
                lua_State* const l = m_table.GetState();
                m_table.Push();
                m_key.Push();
                if (lua_next(l, -2) != 0)
                {
                    m_value = ParentClass::FromTop(l);
                    m_key = ParentClass::FromTop(l);
                }
                else
                {
                    m_key.Unref();
                    m_value.Unref();
                }
                lua_pop(l, 1);
                return *this;
            }

            bool operator==(const Iterator& other)const
            {
                return m_table == other.m_table && m_key == other.m_key;
            }

            bool operator!=(const Iterator& other)const
            {
                return !(*this == other);
            }

            Iterator& operator++()
            {
                return this->Next();
            }

            const std::pair<ParentClass, ParentClass> operator*()const
            {
                return { m_key,m_value };
            }

            const ParentClass& operator->()const
            {
                return m_value;
            }

            const ParentClass& Key()const { return m_key; }
            const ParentClass& Value()const { return m_value; }

        private:
            ParentClass m_table{};
            ParentClass m_key{};
            ParentClass m_value{};
        };
        friend class Iterator;

        Iterator begin()const { return Iterator(ParentClass(_This())).Next(); };
        Iterator end()const { return Iterator(ParentClass(_This())); }

        /**
         * @brief Преобразует объект к данному типу.
         * 
         * @tparam T 
         * @return T 
         */
        template<typename T>
        T To()const
        {
            auto r = PushView().To<T>();
            Pop();
            return r;
        }

        /**
         * @brief Преобразует объект к данному типу.
         * 
         * @tparam T 
         * @return T 
         */
        template<typename T>
        operator T()const
        {
            return this->To<T>();
        }

        template<typename T>
        bool operator<(const T& other)const
        {
            return this->Compare<LUA_OPLT>(other);
        }

        template<typename T>
        bool operator<=(const T& other)const
        {
            return this->Compare<LUA_OPLE>(other);
        }

        template<typename T>
        bool operator==(const T& other)const
        {
            return this->Compare<LUA_OPEQ>(other);
        }

        template<>
        bool operator==(const std::nullptr_t&)const
        {
            return this->IsNil();
        }

        template<typename T>
        bool operator>(const T& other)const
        {
            return !(*this <= other);
        }

        template<typename T>
        bool operator>=(const T& other)const
        {
            return !(*this < other);
        }

        template<typename T>
        bool operator!=(const T& other)const
        {
            return !(*this == other);
        }

        template<typename TReturn = void, typename ...TArgs>
        TReturn Call(TArgs&& ...args)const
        {
            return CallFunction<TReturn>(m_state, _This(), std::forward<TArgs>(args)...);
        }

        template<typename TReturn = void, typename ...TArgs>
        PCallReturn<TReturn> PCall(TArgs&& ...args)const
        {
            return PCallFunction<TReturn>(m_state, _This(), std::forward<TArgs>(args)...);
        }

        template<typename TReturn = void, typename ...TArgs>
        TReturn SelfCall(const char* key, TArgs&& ...args)const
        {
            Push();
            lua_getfield(m_state, -1, key);
            lua_rotate(m_state, -2, 1);
            size_t n = PushArgs(m_state, std::forward<TArgs>(args)...) + 1;
            return CallStack<TReturn>(m_state, n);
        }

        template<typename TReturn = void, typename ...TArgs>
        PCallReturn<TReturn> SelfPCall(const char* key, TArgs&& ...args)const
        {
            Push();
            lua_getfield(m_state, -1, key);
            lua_rotate(m_state, -2, 1);
            size_t n = PushArgs(m_state, std::forward<TArgs>(args)...) + 1;
            return PCallStack<TReturn>(m_state, n);
        }

        template<typename ...TArgs>
        ParentClass operator()(TArgs&& ...args)const
        {
            return this->Call<ParentClass>(std::forward<TArgs>(args)...);
        }

        /**
         * @brief Возвращает RefObject с метатаблицей объекта.
         * Если ее нет возвращает nil.
         * @return ParentClass
         */
        ParentClass GetMetaTable() const
        {
            PushView().GetMetaTable();
            ParentClass res = ParentClass::FromTop(m_state);
            Pop();
            return res;
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
            PushView().SetMetaTable(value);
            Pop();
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
            R l = PushView().Len<R>();
            Pop();
            return l;
        }

        /**
         * @brief возвращает результат работы метаметода
         * __len, если таковой присутствует, в противном
         * случае возвращает результат RawLen.
         *
         * @return ParentClass
         */
        ParentClass Len() const
        {
            PushView().Len();
            ParentClass res = ParentClass::FromTop(m_state);
            Pop();
            return res;
        }

        /**
         * @brief Возвращает необработанную
         * длину объекта.
         * @return size_t длина объекта
         */
        auto RawLen() const
        {
            auto r = PushView().RawLen();
            Pop();
            return r;
        }

        /**
         * @brief Проверяет является ли объект данного типа.
         * 
         * @tparam LType 
         * @return true 
         * @return false 
         */
        template <Type LType>
        bool Is() const
        {
            return Type() == LType;
        }

        /**
         * @brief Проверяет может ли объект быть преобразован к данному типу.
         * 
         * @tparam LType 
         * @return true 
         * @return false 
         */
        template<typename T>
        bool Is()const
        {
            bool r = PushView().Is<T>();
            Pop();
            return r;
        }

        bool IsNil()const
        {
            return Type() == Type::Nil;
        }

        bool IsTable()const
        {
            return Type() == Type::Table;
        }

        bool IsUserData()const
        {
            return Type() == Type::Userdata;
        }

        /**
         * @brief Возвращает тип объекта.
         * 
         * @return Type 
         */
        Type Type()const
        {
            auto t = PushView().Type();
            Pop();
            return t;
        }

        /**
         * @brief Возвращает имя типа объекта.
         * 
         * @return const char* 
         */
        const char* TypeName()const
        {
            Push();
            auto t = lua_typename(m_state, lua_type(m_state, -1));
            Pop();
            return t;
        }

        /**
         * @brief Возвращает строковую репрезентацию объекта.
         * 
         * @return const char* 
         */
        const char* ToString()const
        {
            Push();
            const char* s = luaL_tolstring(m_state, -1, nullptr);
            lua_pop(m_state, 2);
            return s;
        }

        friend static std::ostream& operator<<(std::ostream& os, const RefObjectBase& obj)
        {
            return os << obj.ToString();
        }

        lua_State* const GetState()const
        {
            return this->m_state;
        }

        /**
         * @brief Помещает на стек объект и возвращает его StackObjectView
         * 
         * @return StackObjectView 
         * @see StackObjectView
         */
        StackObjectView PushView()const
        {
            Push();
            return { m_state };
        }

        ~RefObjectBase()
        {
            Unref();
            Clear();
        }

    private:
        template <int COMPARE_OP, typename T>
        bool Compare(const T& value)const
        {
            Push();
            PushValue(m_state, value);
            bool r = lua_compare(m_state, -2, -1, COMPARE_OP);
            lua_pop(m_state, 2);
            return r;
        }

        void Unref()
        {
            _This().Unref();
        }

        void Clear()
        {
            _This().Clear();
        }

        void Push()const
        {
            _This().Push();
        }

        void Pop()const
        {
            _This().Pop();
        }

    protected:
        void PushRef(int ref)const
        {
            RefAccess::PushRef(m_state, ref);
        }

        int GetRef()const
        {
            return RefAccess::GetRef(m_state);
        }

        void UnrefRef(int ref)
        {
            RefAccess::Unref(m_state, ref);
        }

    private:
        const RefClass& _This() const { return static_cast<const RefClass&>(*this); }
        RefClass& _This() { return static_cast<RefClass&>(*this); }
    protected:
        lua_State* m_state = nullptr;
    };

    template<typename RefAccess = RefGlobalAccess>
    class RefObject;

    template<typename RefAccess>
    class RefTableEntryObject;

    template<typename RefAccess>
    class RefObject : public RefObjectBase<RefObject<RefAccess>, RefObject<RefAccess>, RefAccess>
    {
    public:
        using Base = RefObjectBase<RefObject<RefAccess>, RefObject<RefAccess>, RefAccess>;
        using RefTableEntryObjectT = RefTableEntryObject<RefAccess>;
        friend class Base;
        using Base::Base;

        RefObject(const RefObject& obj) : Base(obj.m_state)
        {
            obj.Push();
            Ref();
        }

        template<typename T>
        RefObject(lua_State* l, const T& value) :Base(l)
        {
            PushValue(l, value);
            Ref();
        };

        RefObject(RefObject&& obj) noexcept : Base(obj.m_state)
        {
            m_ref = obj.m_ref;
            obj.Clear();
        }

        RefObject(const RefTableEntryObjectT& obj) : Base(obj.m_state)
        {
            obj.Push();
            Ref();
        }

        template<typename T>
        RefObject& operator=(const T& value)
        {
            Unref();
            PushValue(this->m_state, value);
            Ref();
            return *this;
        }

        RefObject& operator=(const RefObject& obj)
        {
            Unref();
            obj.Push();
            this->m_state = obj.m_state;
            Ref();
            return *this;
        }

        RefObject& operator=(const RefTableEntryObjectT& obj)
        {
            Unref();
            obj.Push();
            this->m_state = obj.m_state;
            Ref();
            return *this;
        }

        RefObject& operator=(RefObject&& obj) noexcept
        {
            Unref();
            m_ref = obj.m_ref;
            this->m_state = obj.m_state;
            obj.Clear();
            return *this;
        }

        /**
         * @brief Возвращает прокси-объект элемента таблицы по данному ключу.
         * 
         * @tparam T 
         * @param key 
         * @return RefTableEntryObjectT 
         */
        template<typename T>
        RefTableEntryObjectT operator[](const T& key)const
        {
            RefTableEntryObjectT obj{ this->m_state };
            PushValue(this->m_state, key);
            obj.m_key_ref = this->GetRef();
            Push();
            obj.m_table_ref = this->GetRef();
            return obj;
        }
        
        /**
         * @brief Снимает с вершины стека объект и возвращает объект-ссылку на него
         * 
         * @param l 
         * @return RefObject 
         */
        static RefObject FromTop(lua_State* l)
        {
            RefObject obj{ l };
            obj.Ref();
            return obj;
        }

        template<typename T>
        static RefObject FromTop(const State<T>& state)
        {
            return FromTop(state.GetState()->Unwrap());
        }

        static RefObject Global(lua_State* l, const char* key)
        {
            lua_getglobal(l, key);
            return FromTop(l);
        }

        template<typename T>
        static RefObject Global(const State<T>& state, const char* key = "_G")
        {
            return Global(state.GetState()->Unwrap(), key);
        }

        static RefObject MakeTable(lua_State* l, int narr = 0, int nhash = 0)
        {
            lua_createtable(l, narr, nhash);
            return FromTop(l);
        }

        template<typename T>
        static RefObject MakeTable(const State<T>& state, int narr = 0, int nhash = 0)
        {
            return MakeTable(state.GetState()->Unwrap(), narr, nhash);
        }

        static RefObject FromStack(lua_State* l, int index)
        {
            lua_pushvalue(l, index);
            return FromTop(l);
        }

        template<typename T>
        static RefObject FromStack(const State<T>& state, int index)
        {
            return FromStack(state.GetState()->Unwrap(), index);
        }

        void Push()const
        {
            this->PushRef(m_ref);
        }

        void Unref()
        {
            if (this->m_state)
            {
                this->UnrefRef(m_ref);
                m_ref = LUA_NOREF;
            }
        }
    private:
        void Ref()
        {
            m_ref = this->GetRef();
        }

        void Clear()noexcept
        {
            this->m_state = nullptr;
            m_ref = LUA_NOREF;
        }

        void Pop()const
        {
            lua_pop(this->m_state, 1);
        }

        int m_ref = LUA_NOREF;
    };

    template<typename RefAccess>
    class RefTableEntryObject : public RefObjectBase<RefTableEntryObject<RefAccess>, RefObject<RefAccess>, RefAccess>
    {
    public:
        using RefClass = RefObject<RefAccess>;
        using Base = RefObjectBase<RefTableEntryObject, RefObject<RefAccess>, RefAccess>;
        friend class RefClass;
        friend class Base;

        template<typename T>
        RefTableEntryObject operator[](const T& key)
        {
            return RefClass(*this)[key];
        }

        template<typename T>
        RefTableEntryObject& operator=(const T& value)
        {
            PushTable();
            PushKey();
            PushValue(this->m_state, value);
            lua_settable(this->m_state, -3);
            Pop();
            return *this;
        }

        template<>
        RefTableEntryObject& operator=(const RefClass& obj)
        {
            PushTable();
            PushKey();
            obj.Push();
            lua_settable(this->m_state, -3);
            Pop();
            return *this;
        }

        RefTableEntryObject& operator=(const RefTableEntryObject& obj)
        {
            return *this = RefClass(obj);
        }

        void Push()const
        {
            PushTable();
            PushKey();
            lua_gettable(this->m_state, -2);
            lua_remove(this->m_state, -2);
        }

        void Unref()
        {
            if (this->m_state)
            {
                this->UnrefRef(m_table_ref);
                this->UnrefRef(m_key_ref);
                m_table_ref = LUA_NOREF;
                m_key_ref = LUA_NOREF;
            }
        }
    private:
        RefTableEntryObject() = delete;
        RefTableEntryObject(lua_State* l) noexcept :Base(l) { };
        template<typename T>
        RefTableEntryObject(const State<T>& state) noexcept : Base(state) {};
        RefTableEntryObject(const RefTableEntryObject& obj) = delete;

        RefTableEntryObject(RefTableEntryObject&& obj)noexcept : Base(obj.m_state)
        {
            m_table_ref = obj.m_table_ref;
            m_key_ref = obj.m_key_ref;
            obj.Clear();
        }

        void PushKey()const
        {
            this->PushRef(m_key_ref);
        }

        void PushTable()const
        {
            this->PushRef(m_table_ref);
        }

        void Clear()noexcept
        {
            this->m_state = nullptr;
            m_table_ref = LUA_NOREF;
            m_key_ref = LUA_NOREF;
        }

        void Pop()const
        {
            lua_pop(this->m_state, 1);
        }

        int m_table_ref = LUA_NOREF;
        int m_key_ref = LUA_NOREF;
    };

    using GRefObject = RefObject<RefGlobalAccess>;

    template<typename T>
    struct StackType<RefObject<T>>
    {
        static RefObject<T> Get(lua_State* l, int index)
        {
            return RefObject<T>::FromStack(l, index);
        }

        static bool Check(lua_State* l, int index)
        {
            return !lua_isnone(l, index);
        }

        static void Push(lua_State* l, const RefObject<T>& value)
        {
            assert(l == value.GetState());
            value.Push();
        }
    };

    template<typename T>
    struct StackType<RefTableEntryObject<T>>
    {
        using Type = RefTableEntryObject<T>;

        static bool Check(lua_State* l, int index)
        {
            return !lua_isnone(l, index);
        }

        static void Push(lua_State* l, const Type& value)
        {
            assert(l == value.GetState());
            value.Push();
        }
    };
}