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


    template<typename RefClass, typename RefAccess>
    class RefObjectBase
    {
    public:
        RefObjectBase() {};
        RefObjectBase(lua_State* l) noexcept : m_state(l) { assert(m_state, "Expected not null lua_State"); };
        RefObjectBase(const State& state) noexcept : RefObjectBase(state.GetState()->Unwrap()) {};

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

        template<typename T>
        T To()const
        {
            AutoPop pop(*this);
            return TypeParser<T>::Get(m_state, -1);
        }

        template<typename T>
        operator T()
        {
            return this->To<T>();
        }

        template<typename T>
        bool Is()const
        {
            AutoPop pop(*this);
            return TypeParser<T>::Check(m_state, -1);
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
            AutoPop pop(*this);
            const char* s = lua_tostring(m_state, -1);
            return s ? s : "";
        }

        ~RefObjectBase()
        {
            Unref();
            Clear();
        }

    private:
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

    template<typename RefAccess>
    class RefObject : public RefObjectBase<RefObject<RefAccess>, RefAccess>
    {
    public:
        template<typename RefAccess>
        class RefTableObject : public RefObjectBase<RefTableObject<RefAccess>, RefAccess>
        {
        public:
            friend class RefObject<RefAccess>;
            using Base = RefObjectBase<RefTableObject<RefAccess>, RefAccess>;
            friend class Base;

            template<typename T>
            RefTableObject operator[](const T& key)
            {
                return RefObject<RefAccess>(*this)[key];
            }

            template<typename T>
            RefTableObject& operator=(T value)
            {
                using TArg = std::decay_t<T>;
                PushTable();
                PushKey();
                TypeParser<TArg>::Push(this->m_state, value);
                lua_settable(this->m_state, -3);
                Pop();
                return *this;
            }

            RefTableObject& operator=(const RefObject<RefAccess>& obj)
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
                return *this = RefObject<RefAccess>(obj);
            }

            void Push()const
            {
                PushTable();
                PushKey();
                lua_gettable(this->m_state, -2);
                lua_remove(this->m_state, -2);
            }

        private:
            RefTableObject() :Base() {};
            RefTableObject(lua_State* l) noexcept :Base(l) { };
            RefTableObject(const State& state) noexcept : Base(state) {};
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

        using Base = RefObjectBase<RefObject<RefAccess>, RefAccess>;
        friend class Base;
        using Base::Base;
        using RefTableObjectT = RefTableObject<RefAccess>;

        RefObject(const RefObject& obj) noexcept : Base(obj.m_state)
        {
            obj.Push();
            Ref();
        }

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

        template<typename T>
        RefObject& operator=(T value)
        {
            using TArg = std::decay_t<T>;
            Unref();
            TypeParser<TArg>::Push(this->m_state, value);
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
            RefObject key_obj{ this->m_state };
            key_obj = key;
            key_obj.Push();
            RefTableObjectT obj{ this->m_state };
            obj.m_key_ref = RefAccess::GetRef(this->m_state);
            Push();
            obj.m_table_ref = RefAccess::GetRef(this->m_state);
            return obj;
        }

        static RefObject Global(lua_State* l, const char* key)
        {
            RefObject obj{ l };
            lua_getglobal(l, key);
            obj.Ref();
            return obj;
        }

        static RefObject Global(const State& state, const char* key)
        {
            return Global(state.GetState()->Unwrap(), key);
        }

        static RefObject MakeTable(lua_State* l, int narr = 0, int nhash = 0)
        {
            RefObject obj{ l };
            lua_createtable(l, narr, nhash);
            obj.Ref();
            return obj;
        }

        static RefObject MakeTable(const State& state, int narr = 0, int nhash = 0)
        {
            return MakeTable(state.GetState()->Unwrap(), narr, nhash);
        }

        static RefObject FromStack(lua_State* l, int index)
        {
            RefObject obj{ l };
            lua_pushvalue(l, index);
            obj.Ref();
            return obj;
        }

        static RefObject FromStack(const State& state, int index)
        {
            return FromStack(state.GetState()->Unwrap(), index);
        }

        void Push()const
        {
            this->PushRef(m_ref);
        }
    private:
        void Ref()
        {
            m_ref = this->GetRef();
        }

        void Unref()
        {
            if (this->m_state)
            {
                this->UnrefRef(m_ref);
                m_ref = LUA_NOREF;
            }
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

    using GRefObject = RefObject<RefGlobalAccess>;

    template<typename T>
    struct TypeParser<RefObject<T>>
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
            value.Push();
        }
    };
}