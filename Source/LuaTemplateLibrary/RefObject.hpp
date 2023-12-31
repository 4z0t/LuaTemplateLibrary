#pragma once
#include "LuaAux.hpp"
#include "LuaState.hpp"
#include "LuaTypes.hpp"

namespace Lua
{

    struct RefGlobalAccess
    {
        static int GetRef(lua_State* l)
        {
            return luaL_ref(l, LUA_REGISTRYINDEX);
        }

        static void Unref(lua_State* l, int ref)
        {
            luaL_unref(l, LUA_REGISTRYINDEX, ref);
        }

        static void PushRef(lua_State* l, int ref)
        {
            lua_rawgeti(l, LUA_REGISTRYINDEX, ref);
        }
    };

    template<typename RefClass, typename ParentClass, typename RefAccess>
    class RefObjectBase
    {
    public:
        RefObjectBase() {};
        RefObjectBase(lua_State* l) noexcept : m_state(l) { assert(m_state, "Expected not null lua_State"); };
        template<typename T>
        RefObjectBase(const State<T>& state) noexcept : RefObjectBase(state.GetState()->Unwrap()) {};

        class AutoPop
        {
        public:
            AutoPop(const RefObjectBase& obj) : m_obj(obj)
            {
                m_obj.Push();
            }
            ~AutoPop()
            {
                m_obj.Pop();
            }
            const RefObjectBase& m_obj;
        };
        friend class AutoPop;

        class Iterator
        {
        public:
            Iterator(const ParentClass& iterable) : m_table(iterable), m_key(iterable.GetState()) {}

            Iterator& Next()
            {
                lua_State* l = m_table.GetState();
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

            ParentClass operator->()const
            {
                return m_value;
            }

            ParentClass Key()const { return m_key; }
            ParentClass Value()const { return m_value; }

        private:
            ParentClass m_table{};
            ParentClass m_key{};
            ParentClass m_value{};
        };
        friend class Iterator;

        Iterator begin()const { return Iterator(ParentClass(_This())).Next(); };
        Iterator end()const { return Iterator(ParentClass(_This())); }

        template<typename T>
        T To()const
        {
            AutoPop pop(*this);
            return StackType<T>::Get(m_state, -1);
        }

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
        TReturn Call(TArgs&& ...args)
        {
            Push();
            size_t n = PushArgs(m_state, std::forward<TArgs>(args)...);
            return CallStack<TReturn>(m_state, n);
        }

        template<typename TReturn = void, typename ...TArgs>
        TReturn SelfCall(const char* key, TArgs&& ...args)
        {
            Push();
            lua_getfield(m_state, -1, key);
            lua_rotate(m_state, -2, 1);
            size_t n = PushArgs(m_state, std::forward<TArgs>(args)...) + 1;
            return CallStack<TReturn>(m_state, n);
        }

        template<typename ...TArgs>
        ParentClass operator()(TArgs&& ...args)
        {
            return this->Call<ParentClass>(std::forward<TArgs>(args)...);
        }

        template<typename T>
        bool Is()const
        {
            AutoPop pop(*this);
            return StackType<T>::Check(m_state, -1);
        }

        bool IsNil()const
        {
            AutoPop pop(*this);
            return lua_isnil(m_state, -1);
        }

        bool IsTable()const
        {
            AutoPop pop(*this);
            return lua_istable(m_state, -1);
        }

        bool IsUserData()const
        {
            AutoPop pop(*this);
            return lua_isuserdata(m_state, -1);
        }

        Type Type()const
        {
            AutoPop pop(*this);
            return static_cast<Lua::Type>(lua_type(m_state, -1));
        }

        const char* TypeName()const
        {
            AutoPop pop(*this);
            return lua_typename(m_state, lua_type(m_state, -1));
        }

        const char* ToString()const
        {
            lua_getglobal(m_state, "tostring");
            AutoPop pop(*this);
            lua_call(m_state, 1, 1);
            return lua_tostring(m_state, -1);
        }

        friend static std::ostream& operator<<(std::ostream& os, const RefObjectBase& obj)
        {
            return os << obj.ToString();
        }

        lua_State* const GetState()const
        {
            return this->m_state;
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
            StackPopper pop{ m_state, 2 };
            this->Push();
            PushValue(m_state, value);
            return lua_compare(m_state, -2, -1, COMPARE_OP);
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

        int GetRef()
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
    class RefTableObject;

    template<typename RefAccess>
    class RefObject : public RefObjectBase<RefObject<RefAccess>, RefObject<RefAccess>, RefAccess>
    {
    public:
        using Base = RefObjectBase<RefObject<RefAccess>, RefObject<RefAccess>, RefAccess>;
        using RefTableObjectT = RefTableObject<RefAccess>;
        friend class Base;
        using Base::Base;

        RefObject(const RefObject& obj) noexcept : Base(obj.m_state)
        {
            obj.Push();
            Ref();
        }

        template<typename T>
        RefObject(lua_State* l, const T& value) noexcept :Base(l)
        {
            PushValue(l, value);
            Ref();
        };

        RefObject(RefObject&& obj) noexcept : Base(obj.m_state)
        {
            m_ref = obj.m_ref;
            obj.Clear();
        }

        RefObject(const RefTableObjectT& obj) noexcept : Base(obj.m_state)
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

        RefObject& operator=(const RefTableObjectT& obj)
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

        template<typename T>
        RefTableObjectT operator[](const T& key)
        {
            RefTableObjectT obj{ this->m_state };
            PushValue(this->m_state, key);
            obj.m_key_ref = this->GetRef();
            Push();
            obj.m_table_ref = this->GetRef();
            return obj;
        }

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


        void Clear()
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
    class RefTableObject : public RefObjectBase<RefTableObject<RefAccess>, RefObject<RefAccess>, RefAccess>
    {
    public:
        using RefClass = RefObject<RefAccess>;
        using Base = RefObjectBase<RefTableObject, RefObject<RefAccess>, RefAccess>;
        friend class RefClass;
        friend class Base;

        template<typename T>
        RefTableObject operator[](const T& key)
        {
            return RefClass(*this)[key];
        }

        template<typename T>
        RefTableObject& operator=(const T& value)
        {
            PushTable();
            PushKey();
            PushValue(this->m_state, value);
            lua_settable(this->m_state, -3);
            Pop();
            return *this;
        }

        template<>
        RefTableObject& operator=(const RefClass& obj)
        {
            PushTable();
            PushKey();
            obj.Push();
            lua_settable(this->m_state, -3);
            Pop();
            return *this;
        }

        RefTableObject& operator=(const RefTableObject& obj)
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
        RefTableObject() :Base() {};
        RefTableObject(lua_State* l) noexcept :Base(l) { };
        template<typename T>
        RefTableObject(const State<T>& state) noexcept : Base(state) {};
        RefTableObject(const RefTableObject& obj) : Base(obj.m_state)
        {
            obj.PushKey();
            m_key_ref = this->GetRef();

            obj.PushTable();
            m_table_ref = this->GetRef();
        }

        void PushKey()const
        {
            this->PushRef(m_key_ref);
        }

        void PushTable()const
        {
            this->PushRef(m_table_ref);
        }

        void Clear()
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
    struct StackType<RefTableObject<T>>
    {
        using Type = RefTableObject<T>;

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