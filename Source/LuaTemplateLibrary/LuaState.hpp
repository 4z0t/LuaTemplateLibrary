#pragma once
#include "LuaAux.hpp"
#include "LuaTypes.hpp"
#include "Exception.hpp"
#include "LuaFunctions.hpp"
#include "CState.hpp"
#include "RefObject.hpp"
#include "StackObject.hpp"
#include "Libs.hpp"

namespace LTL
{
    template<typename T>
    struct UserData;

    struct OpNewAllocator
    {
        static void* Function(void* ud, void* ptr, size_t osize, size_t nsize)
        {
            if (nsize == 0)
            {
                Delete(ptr);
                return nullptr;
            }
            return NewMem(ptr, osize, nsize);
        }
    protected:
        static void Delete(void* ptr)
        {
            delete[] static_cast<char*>(ptr);
        }

        static void* NewMem(void* ptr, size_t osize, size_t nsize)
        {
            if (ptr == nullptr)
            {
                ptr = static_cast<void*>(new char[nsize]);
                return ptr;
            }
            size_t min_size = std::min(osize, nsize);
            void* new_ptr = static_cast<void*>(new char[nsize]);
            std::memcpy(new_ptr, ptr, min_size);
            Delete(ptr);
            return new_ptr;
        }
    };

    template<typename Allocator = void>
    class State
    {
    public:
        State(void* obj = nullptr)
        {
            if constexpr (!std::is_void_v<Allocator>)
            {
                m_cstate = CState::Create(Allocator::Function, obj);
            }
            else
            {
                m_cstate = CState::Create();
            }
        }



        State(const State&) = delete;
        State(State&&) = delete;
        State& operator=(const State&) = delete;
        State& operator=(State&&) = delete;

        ~State()
        {
            if (m_cstate)
                m_cstate->Close();
            m_cstate = nullptr;
        }

        void ThrowExceptions()
        {
            m_cstate->SetAtPanicFuntion(Exception::PanicFunc);
        }

        void OpenLibs()
        {
            return m_cstate->OpenLibs();
        }

        void OpenLib(const Lib& lib)
        {
            lib.Register(m_cstate->Unwrap());
        }

        void OpenLibs(const Lib& lib)
        {
            OpenLib(lib);
        }

        template<typename ...Ts>
        void OpenLibs(const Lib& lib, const Ts&...libs)
        {
            OpenLib(lib);
            return OpenLibs(libs...);
        }

        template<typename T>
        void Push(const T& value)
        {
            return m_cstate->Push<T>(value);
        }

        void Pop(size_t n)
        {
            return m_cstate->Pop(n);
        }

        State& AddFunction(const char* const name, lua_CFunction func)
        {
            return this->AddClosure(name, func);
        }

        template<typename ...Ts>
        State& AddClosure(const char* const name, lua_CFunction func, Ts&&... args)
        {
            m_cstate->RegisterClosure(name, func, std::forward<Ts>(args)...);
            return *this;
        }

        bool DoFile(const char* const path)
        {
            return m_cstate->DoFile(path);
        }

        template<typename TReturn = void, typename ...Ts>
        TReturn Call(const char* name, Ts&&... args)
        {
            return m_cstate->Call<TReturn>(name, std::forward<Ts>(args)...);
        }

        template<typename TReturn = void, typename ...Ts>
        PCallReturn<TReturn> PCall(const char* name, Ts&&... args)
        {
            return m_cstate->PCall<TReturn>(name, std::forward<Ts>(args)...);
        }

        template<typename T>
        T To(int index)
        {
            return m_cstate->Get<T>(index);
        }

        CState* const GetState()const
        {
            return m_cstate;
        }

        void Run(const char* const s) throw(Exception)
        {
            return m_cstate->Run(s);
        }

        void Run(const std::string& s)
        {
            return Run(s.c_str());
        }

        template<typename TReturn = GRefObject>
        TReturn GetGlobal(const char* key)
        {
            return m_cstate->GetGlobal<TReturn>(key);
        }

        template<typename T>
        State& SetGlobal(const char* name, const T& value)
        {
            m_cstate->SetGlobal(name, value);
            return *this;
        }

        template<typename TClass, typename ...TArgs>
        GRefObject MakeUserData(TArgs&&... args)
        {
            return UserData<TClass>::Make(m_cstate->Unwrap(), std::forward<TArgs>(args)...);
        }

        template<typename Base, typename Derived>
        using EnableIfBaseOf = std::enable_if_t<std::is_base_of_v<Base, Derived>, State&>;


        template<typename T, typename ...TUpvalues>
        EnableIfBaseOf<Internal::CFunctionBase, T> Add(const char* name, const T&, TUpvalues&&... upvalues)
        {
            static_assert(T::template ValidUpvalues<const_decay_t<TUpvalues>...>::value,
                "Passed upvalues don't match with ones defined in closure");
            return AddClosure(name, T::Function, std::forward<TUpvalues>(upvalues)...);
        }

    private:
        CState* m_cstate = nullptr;

    };
}