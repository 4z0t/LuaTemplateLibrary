#pragma once
#include "LuaAux.hpp"
#include "LuaState.hpp"
#include "LuaTypes.hpp"

namespace Lua
{
    template<typename ThisClass, typename RefClass>
    class RefObjectBase
    {
    public:

        RefObjectBase() {};
        RefObjectBase(lua_State* l) noexcept : m_state(l) { assert(m_state, "Expected not null lua_State"); };
        RefObjectBase(const State& state) noexcept : RefObjectBase(state.GetState()->Unwrap()) {};

        class AutoPop
        {
        public:
            AutoPop(const RefObjectBase* obj) : m_obj(obj)
            {
                m_obj->Push();
            }
            ~AutoPop()
            {
                m_obj->Pop();
            }
            const RefObjectBase* m_obj;
        };
        friend class AutoPop;

        template<typename T>
        bool Is()const
        {
            AutoPop pop(this);
            return TypeParser<T>::Check(m_state, -1);
        }

        bool IsNil()const
        {
            AutoPop pop(this);
            return lua_isnil(m_state, -1);
        }

        bool IsTable()const
        {
            AutoPop pop(this);
            return lua_istable(m_state, -1);
        }

        Type Type()const
        {
            AutoPop pop(this);
            return static_cast<Lua::Type>(lua_type(m_state, -1));
        }

        const char* TypeName()const
        {
            AutoPop pop(this);
            return lua_typename(m_state, lua_type(m_state, -1));
        }

        const char* ToString()const
        {
            AutoPop pop(this);
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

    private:
        const ThisClass& _This() const { return static_cast<const ThisClass&>(*this); }
        ThisClass& _This() { return static_cast<ThisClass&>(*this); }
    protected:
        lua_State* m_state = nullptr;
    };

    class RefObject : public RefObjectBase<RefObject, RefObject>
    {
    public:

        class RefTableObject : public RefObjectBase<RefTableObject, RefObject>
        {
        public:
            friend class RefObject;
            friend class RefObjectBase<RefTableObject, RefObject>;
            using RefObjectBase::RefObjectBase;

            template<typename T>
            RefTableObject operator[](const T& key)
            {
                return RefObject(*this)[key];
            }

            template<typename T>
            RefTableObject& operator=(T&& value)
            {
                using TArg = std::decay_t<T>;
                lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_table_ref);
                lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_key_ref);
                TypeParser<TArg>::Push(m_state, std::forward<TArg>(value));
                lua_settable(m_state, -3);
                Pop();
                return *this;
            }

        private:
            void Unref()
            {
                if (m_state)
                {
                    luaL_unref(m_state, LUA_REGISTRYINDEX, m_table_ref);
                    luaL_unref(m_state, LUA_REGISTRYINDEX, m_key_ref);
                    m_table_ref = LUA_NOREF;
                    m_key_ref = LUA_NOREF;
                }
            }

            void Clear()
            {
                m_state = nullptr;
                m_table_ref = LUA_NOREF;
                m_key_ref = LUA_NOREF;
            }

            void Push()const
            {
                lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_table_ref);
                lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_key_ref);
                lua_gettable(m_state, -2);
                lua_remove(m_state, -2); // remove the table
            }

            void Pop()const
            {
                lua_pop(m_state, 1);
            }

            int m_table_ref = LUA_NOREF;
            int m_key_ref = LUA_NOREF;
        };

        friend class RefObjectBase<RefObject, RefObject>;
        using RefObjectBase::RefObjectBase;

        RefObject(const RefObject& obj) noexcept : RefObjectBase(obj.m_state)
        {
            obj.Push();
            Ref();
        }

        RefObject(RefObject&& obj) noexcept : RefObjectBase(obj.m_state)
        {
            m_ref = obj.m_ref;
            obj.Clear();
        }

        RefObject(const RefTableObject& obj) noexcept : RefObjectBase(obj.m_state)
        {
            obj.Push();
            Ref();
        }

        template<typename T>
        RefObject& operator=(T&& value)
        {
            using TArg = std::decay_t<T>;
            Unref();
            TypeParser<TArg>::Push(m_state, std::forward<TArg>(value));
            Ref();
            return *this;
        }

        template<>
        RefObject& operator=(const RefObject& obj)
        {
            Unref();
            obj.Push();
            m_state = obj.m_state;
            Ref();
            return *this;
        }

        template<>
        RefObject& operator=(RefObject& obj)
        {
            Unref();
            obj.Push();
            m_state = obj.m_state;
            Ref();
            return *this;
        }

        template<>
        RefObject& operator=(RefObject&& obj)
        {
            Unref();
            m_ref = obj.m_ref;
            m_state = obj.m_state;
            obj.Clear();
            return *this;
        }

        static RefObject MakeTable(lua_State* l, int narr = 0, int nhash = 0)
        {
            RefObject obj{ l };
            lua_createtable(l, narr, nhash);
            obj.Ref();
            return obj;
        }

        template<typename T>
        RefTableObject operator[](const T& key)
        {
            RefObject key_obj{ m_state };
            key_obj = key;
            key_obj.Push();
            RefTableObject obj{ m_state };
            obj.m_key_ref = luaL_ref(m_state, LUA_REGISTRYINDEX);
            Push();
            obj.m_table_ref = luaL_ref(m_state, LUA_REGISTRYINDEX);
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
    private:
        void Ref()
        {
            m_ref = luaL_ref(m_state, LUA_REGISTRYINDEX);
        }

        void Unref()
        {
            if (m_state)
            {
                luaL_unref(m_state, LUA_REGISTRYINDEX, m_ref);
                m_ref = LUA_NOREF;
            }
        }

        void Clear()
        {
            m_state = nullptr;
            m_ref = LUA_NOREF;
        }

        void Push()const
        {
            lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_ref);
        }

        void Pop()const
        {
            lua_pop(m_state, 1);
        }

        int m_ref = LUA_NOREF;
    };
}