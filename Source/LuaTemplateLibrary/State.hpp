#pragma once
#include "LuaAux.hpp"
#include "Types.hpp"
#include "Exception.hpp"
#include "Function.hpp"
#include "CState.hpp"
#include "RefObject.hpp"
#include "StackObject.hpp"
#include "Libs.hpp"

namespace LTL
{
    template <typename T>
    struct UserData;

    struct OpNewAllocator
    {
        static void *Function(void *ud, void *ptr, size_t osize, size_t nsize)
        {
            if (nsize == 0)
            {
                Delete(ptr);
                return nullptr;
            }
            return NewMem(ptr, osize, nsize);
        }

    protected:
        inline static void Delete(void *ptr)
        {
            delete[] static_cast<char *>(ptr);
        }

        inline static void *New(size_t size)
        {
            return static_cast<void *>(new char[size]);
        }

        static void *NewMem(void *ptr, size_t osize, size_t nsize)
        {
            if (ptr == nullptr)
            {
                return New(nsize);
            }
            size_t min_size = std::min(osize, nsize);
            void *new_ptr = New(nsize);
            std::memcpy(new_ptr, ptr, min_size);
            Delete(ptr);
            return new_ptr;
        }
    };

    /**
     * @brief Класс состояния виртуальной машины Lua.
     *
     * @tparam Allocator класс-интерфейс для управления памятью.
     */
    template <typename Allocator = void>
    class State
    {
    public:
        State(void *obj = nullptr)
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

        State(const State &) = delete;
        State(State &&) = delete;
        State &operator=(const State &) = delete;
        State &operator=(State &&) = delete;

        ~State()
        {
            if (m_cstate)
                m_cstate->Close();
            m_cstate = nullptr;
        }

        /**
         * @brief Включает стандартный обработчик исключений.
         *
         */
        void ThrowExceptions()
        {
            m_cstate->SetAtPanicFuntion(Exception::PanicFunc);
        }

        /**
         * @brief Открывает все библиотеки Lua.
         *
         */
        void OpenLibs()
        {
            return m_cstate->OpenLibs();
        }

        /**
         * @brief Открывает данную библиотеку Lua.
         *
         * @param lib
         */
        void OpenLib(const Lib &lib)
        {
            lib.Register(m_cstate);
        }

        void OpenLibs(const Lib &lib)
        {
            OpenLib(lib);
        }

        /**
         * @brief Открывает данные библиотеки Lua.
         *
         * @param lib
         */
        template <typename... Ts>
        void OpenLibs(const Lib &lib, const Ts &...libs)
        {
            OpenLib(lib);
            return OpenLibs(libs...);
        }

        /**
         * @brief Добавляет глобальную функцию
         *
         * @param name имя функции
         * @param func функция
         * @return State&
         */
        State &AddFunction(const char *const name, lua_CFunction func)
        {
            return AddClosure(name, func);
        }

        /**
         * @brief Добавляет глобальное замыкание с данными upvalue
         *
         * @tparam Ts
         * @param name имя замыкания
         * @param func замыкание
         * @param args
         * @return State&
         */
        template <typename... Ts>
        State &AddClosure(const char *const name, lua_CFunction func, Ts &&...args)
        {
            m_cstate->RegisterClosure(name, func, std::forward<Ts>(args)...);
            return *this;
        }

        /**
         * @brief Исполняет файл и возвращает результат работы
         *
         * @param path путь к файлу
         * @return PCallResult
         */
        PCallResult DoFile(const char *const path)
        {
            return m_cstate->DoFile(path);
        }

        /**
         * @brief Вызывает глобальную функцию с данными аргументами и возвращает результат
         *
         * @tparam TReturn Тип результата функции
         * @tparam Ts типы аргументов
         * @param name имя функции
         * @param args
         * @return TReturn
         */
        template <typename TReturn = void, typename... Ts>
        TReturn Call(const char *name, Ts &&...args)
        {
            return m_cstate->Call<TReturn>(name, std::forward<Ts>(args)...);
        }

        /**
         * @brief Безопасно вызывает глобальную функцию с данными аргументами и возвращает результат
         *
         * @tparam TReturn Тип результата функции
         * @tparam Ts типы аргументов
         * @param name имя функции
         * @param args
         * @return PCallReturn<TReturn>
         */
        template <typename TReturn = void, typename... Ts>
        PCallReturn<TReturn> PCall(const char *name, Ts &&...args)
        {
            return m_cstate->PCall<TReturn>(name, std::forward<Ts>(args)...);
        }

        /**
         * @brief Выполняет данную строку.
         * 
         * @param s 
         */
        void Run(const char *const s) noexcept(false)
        {
            return m_cstate->Run(s);
        }

        /**
         * @brief Выполняет данную строку.
         * 
         * @param s 
         */
        void Run(const std::string &s)
        {
            return Run(s.c_str());
        }

        /**
         * @brief Возвращает глобальное значение по имени
         * 
         * @tparam TReturn 
         * @param key 
         * @return TReturn 
         */
        template <typename TReturn = GRefObject>
        TReturn GetGlobal(const char *key)
        {
            return m_cstate->GetGlobal<TReturn>(key);
        }

        /**
         * @brief Устанавливает глобальное значение с данным именем
         * 
         * @tparam T 
         * @param name 
         * @param value 
         * @return State& 
         */
        template <typename T>
        State &SetGlobal(const char *name, const T &value)
        {
            m_cstate->SetGlobal(name, value);
            return *this;
        }

        /**
         * @brief Создает UserData<T> с данными аргументами и возвращает объект-ссылку на нее.
         * 
         * @tparam TClass класс пользовательского типа
         * @tparam TArgs 
         * @param args 
         * @return GRefObject 
         */
        template <typename TClass, typename... TArgs>
        GRefObject MakeUserData(TArgs &&...args)
        {
            return UserData<TClass>::Make(m_cstate->Unwrap(), std::forward<TArgs>(args)...);
        }

        template <typename Base, typename Derived>
        using EnableIfBaseOf = std::enable_if_t<std::is_base_of_v<Base, Derived>, State &>;

        template <typename T, typename... TUpvalues>
        EnableIfBaseOf<Internal::CFunctionBase, T> Add(const char *name, const T &, TUpvalues &&...upvalues)
        {
            static_assert(T::template ValidUpvalues<const_decay_t<TUpvalues>...>::value,
                          "Passed upvalues don't match with ones defined in closure");
            return AddClosure(name, T::Function, std::forward<TUpvalues>(upvalues)...);
        }

        CState* const GetState()const
        {
            return m_cstate;
        }

    private:
        CState *m_cstate = nullptr;
    };
}