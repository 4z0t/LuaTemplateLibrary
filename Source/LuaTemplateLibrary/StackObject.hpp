#pragma once
#include "LuaAux.hpp"
#include "LuaState.hpp"
#include "LuaTypes.hpp"
#include "RefObject.hpp"

namespace Lua
{
    class StackObjectView
    {
    public:
        StackObjectView() = delete;
        StackObjectView(lua_State* l, int index = -1) : m_state(l), m_index(lua_absindex(l, index)) {}

        template<typename T>
        T To()const
        {
            return StackType<T>::Get(m_state, m_index);
        }

        template<Type LType>
        bool Is()const
        {
            return lua_type(m_state, m_index) == static_cast<int>(LType);
        }

        template<typename T>
        bool Is()const
        {
            return StackType<T>::Check(m_state, m_index);
        }

        void Push()const
        {
            lua_pushvalue(m_state, m_index);
        }

        template<typename T>
        bool operator==(const T& value)const
        {
            StackPopper pop{ m_state,1 };
            PushValue(m_state, value);
            return lua_compare(m_state, m_index, -1, LUA_OPEQ);
        }

        template<>
        bool operator==(const StackObjectView& value)const
        {
            return m_state == value.m_state && lua_compare(m_state, m_index, value.m_index, LUA_OPEQ);
        }

        template<typename T>
        bool RawEqual(const T& value)
        {
            StackPopper pop{ m_state,1 };
            PushValue(m_state, value);
            return lua_rawequal(m_state, m_index, -1);
        }

        template<>
        bool RawEqual(const StackObjectView& value)
        {
            return lua_rawequal(m_state, m_index, value.m_index);
        }

        template<typename T>
        bool operator!=(const T& value)const
        {
            return !(*this == value);
        }

        template<typename R, typename T>
        R Get(const T& key)const
        {
            StackPopper pop{ m_state, 1 };
            PushValue(m_state, key);
            lua_gettable(m_state, m_index);
            return GetValue<R>(m_state, -1);
        }

        template<typename T>
        StackObjectView Get(const T& key)const
        {
            PushValue(m_state, key);
            lua_gettable(m_state, m_index);
            return { m_state };
        }

        template<typename K, typename V>
        void Set(const K& key, const V& value)
        {
            PushValue(m_state, key);
            PushValue(m_state, value);
            lua_settable(m_state, m_index);
        }

        template<typename R, typename T>
        R RawGet(const T& key)const
        {
            StackPopper pop{ m_state, 1 };
            PushValue(m_state, key);
            lua_rawget(m_state, m_index);
            return GetValue<R>(m_state, -1);
        }

        template<typename T>
        StackObjectView RawGet(const T& key)const
        {
            PushValue(m_state, key);
            lua_rawget(m_state, m_index);
            return { m_state };
        }

        template<typename K, typename V>
        void RawSet(const K& key, const V& value)
        {
            PushValue(m_state, key);
            PushValue(m_state, value);
            lua_rawset(m_state, m_index);
        }

        template<typename R>
        R Len()const
        {
            StackPopper pop{ m_state, 1 };
            lua_len(m_state, m_index);
            return GetValue<R>(m_state, -1);
        }

        StackObjectView Len()const
        {
            lua_len(m_state, m_index);
            return { m_state };
        }

        size_t RawLen()const
        {
            return lua_rawlen(m_state, m_index);
        }

        StackObjectView GetMetaTable()const
        {
            if (!lua_getmetatable(m_state, m_index))
            {
                lua_pushnil(m_state);
            }
            return { m_state };
        }

        template<typename T>
        void SetMetaTable(const T& value)
        {
            PushValue(m_state, value);
            lua_setmetatable(m_state, m_index);
        }

        static StackObjectView Global(lua_State* l, const char* name)
        {
            lua_getglobal(l, name);
            return StackObjectView{ l };
        }

        ~StackObjectView() {}

        lua_State* const GetState()const
        {
            return m_state;
        }
    protected:
        lua_State* const m_state;
        const int m_index;
    };

    template<>
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

    class StackObject : public StackObjectView
    {
    public:
        StackObject(lua_State* l, int index) : StackObjectView(l, PushIndex(l, index)) {}

        template<typename T>
        StackObject(const RefObject<T>& obj) : StackObjectView(obj.GetState(), PushRefObject(obj)) {}

        StackObject() = delete;
        StackObject(const StackObject&) = delete;
        StackObject(StackObject&&) = delete;

        StackObject& operator=(const StackObject&) = delete;
        StackObject& operator=(StackObject&&) = delete;

        template<typename T>
        static StackObject FromValue(lua_State* l, const T& value)
        {
            PushValue(l, value);
            return StackObject{ l };
        }

        static StackObject Global(lua_State* l, const char* name)
        {
            return StackObjectView::Global(l, name);
        }

        using StackObjectView::Get;

        template<typename T>
        StackObject Get(const T& key)const
        {
            return StackObjectView::Get(key);
        }

        using StackObjectView::RawGet;

        template<typename T>
        StackObject RawGet(const T& key)const
        {
            return StackObjectView::RawGet(key);
        }

        StackObject GetMetaTable()const
        {
            return StackObjectView::GetMetaTable();
        }

        StackObject Len()const
        {
            return StackObjectView::Len();
        }

        template<typename T>
        bool RawEqual(const T& value)
        {
            return StackObjectView::RawEqual(value);
        }

        template<>
        bool RawEqual(const StackObject& value)
        {
            return StackObjectView::RawEqual(static_cast<StackObjectView>(value));
        }

        ~StackObject()
        {
            if (lua_gettop(m_state) == m_index)
            {
                lua_pop(m_state, 1);
            }
            else
            {
                lua_remove(m_state, m_index);
            }
            /*
            * lua_pushnil(m_state);
            * lua_replace(m_state, m_index);
            */
        }

    protected:
        StackObject(StackObjectView&& view) :StackObjectView(view) {}
        StackObject(lua_State* l) : StackObjectView(l, -1) {}

        static int PushIndex(lua_State* l, int index)
        {
            lua_pushvalue(l, index);
            return index;
        }

        template<typename T>
        static int PushRefObject(const RefObject<T>& obj)
        {
            obj.Push();
            return -1;
        }
    };



    template<>
    struct StackType<StackObject>
    {
        static StackObject Get(lua_State* l, int index)
        {
            return { l, index };
        }

        static bool Check(lua_State* l, int index)
        {
            return !lua_isnone(l, index);
        }

        static void Push(lua_State* l, const StackObject& value)
        {
            assert(value.GetState() == l);
            value.Push();
        }
    };


}