#pragma once
#include "LuaAux.hpp"
#include "Types.hpp"
#include "FuncArguments.hpp"
#include "Exception.hpp"
#include "RefObject.hpp"

namespace LTL
{
    /**
     * @brief Класс представляющий пользовательский тип в Lua
     *
     * @tparam T пользовательский тип
     */
    template<typename T>
    struct UserData : Internal::UserDataPtr<T>
    {
        using _UserDataValue = Internal::UserDataPtr<T>;
        using _UserDataValue::_UserDataValue;

        struct MetaTable : public RegistryValue<MetaTable> {};
        struct MethodsTable : public RegistryValue<MethodsTable> {};
        struct ClassTable : public RegistryValue<ClassTable> {};
        struct IndexTable : public RegistryValue<IndexTable> {};
        struct NewIndexTable : public RegistryValue<NewIndexTable> {};

        struct Data
        {
            T object;
            bool isDestroyed = false;
        };
    private:
        /**
         * @brief Помещает на стек UserData<T> и возвращает указатель на его место
         * в памяти для дальнейшего использования с оператором new
         *
         * @param l
         * @return void* const
         */
        static T* const Allocate(lua_State* l)
        {
            Data* const data = (Data*)lua_newuserdata(l, sizeof(Data));
            data->isDestroyed = false;
            SetClassMetaTable(l);
            return &data->object;
        }
    public:
        /**
         * @brief Помещает на стек UserData<T> через move copy данного объекта
         *
         * @param l
         * @param other
         */
        static void PushCopy(lua_State* l, T&& other)
        {
            static_assert(std::is_move_constructible_v<T>, "Can't move-construct type T!");
            new(Allocate(l)) T(std::forward<T>(other));
        }

        /**
         * @brief Помещает на стек UserData<T> - копию данного объекта
         *
         * @param l
         * @param other
         */
        static void PushCopy(lua_State* l, const T& other)
        {
            static_assert(std::is_copy_constructible_v<T>, "Can't copy-construct type T!");
            new(Allocate(l)) T(other);
        }

        template<typename ...TArgs>
        static T* New(lua_State* l, TArgs&&... args)
        {
            static_assert(std::is_constructible_v<T, TArgs...>, "Object of class C can't be constructed with such arguments!");
            return new(Allocate(l)) T(std::forward<TArgs>(args)...);
        }

        template<typename ...TArgs>
        static T* New(CState* cstate, TArgs&&... args)
        {
            return New(cstate->Unwrap(), std::forward<TArgs>(args)...);
        }

        /**
         * @brief Создает UserData<T> и возвращает объект-ссылку с ним
         *
         * @tparam Alloc
         * @tparam TArgs
         * @param s
         * @param args
         * @return GRefObject
         */
        template<typename Alloc, typename ...TArgs>
        static GRefObject Make(const State<Alloc>& s, TArgs&&... args)
        {
            return Make(s.GetState()->Unwrap(), std::forward<TArgs>(args)...);
        }

        /**
         * @brief Создает UserData<T> и возвращает объект-ссылку с ним
         *
         * @tparam TArgs
         * @param l
         * @param args
         * @return GRefObject
         */
        template<typename ...TArgs>
        static GRefObject Make(lua_State* l, TArgs&&... args)
        {
            New(l, std::forward<TArgs>(args)...);
            return GRefObject::FromTop(l);
        }

        /**
         * @brief Устанавливает метатаблицу класса на объект на стеке
         *
         * @param l
         */
        static void SetClassMetaTable(lua_State* l)
        {
            if (MetaTable::Push(l) != LUA_TTABLE)
            {
                luaL_error(l, "%s", "The class was't registered");
            }
            lua_setmetatable(l, -2);
        }

        /**
         * @brief Функция для уничтожения UserData<T>
         *
         * @param l
         * @return int
         */
        static int DestructorFunction(lua_State* l)
        {

            Data* data = ToUserData(l, 1);

            if (data != nullptr)
            {
                if (!data->isDestroyed)
                {
                    data->object.~T();
                    data->isDestroyed = true;
                }
            }

            return 0;
        }

        /**
         * @brief Проверяет является ли объект по индексу на стеке UserData<T>
         *
         * @param l
         * @param index
         * @return true
         * @return false
         */
        static bool IsUserData(lua_State* l, int index)
        {
            if (!lua_isuserdata(l, index))
            {
                return false;
            }

            if (!lua_getmetatable(l, index))
            {
                return false;
            }

            if (MetaTable::Push(l) == LUA_TNIL)
            {
                lua_pop(l, 1);
                return false;
            }
            bool r = lua_rawequal(l, -2, -1);
            lua_pop(l, 2);
            return r;
        }

        static Data* ToUserData(lua_State* l, int index)
        {
            if (!lua_isuserdata(l, index))
            {
                ThrowWrongType(l, index);
                return nullptr;
            }

            if (!lua_getmetatable(l, index))
            {
                ThrowInvalidUserData(l, index);
                return nullptr;
            }

            if (MetaTable::Push(l) == LUA_TNIL)
            {
                lua_pop(l, 1);
                ThrowNoMetaTableForUD(l, index);
                return nullptr;
            }

            bool res = lua_rawequal(l, -2, -1);
            lua_pop(l, 2);

            if (!res)
            {
                ThrowWrongUserDataType(l, index);
                return nullptr;
            }
            return static_cast<Data*>(lua_touserdata(l, index));
        }

        static T* ValidateUserData(lua_State* l, int index)
        {
            Data* data = ToUserData(l, index);
            if (data == nullptr || data->isDestroyed)
            {
                ThrowUDDestroyed(l, index);
                return nullptr;
            }

            return &data->object;
        }

        static int IndexMethod(lua_State* l)
        {
            IndexTable::Push(l);
            lua_pushvalue(l, 2);
            int type = lua_rawget(l, -2);
            if (type == LUA_TNIL)
            {
                lua_pop(l, 2); // pop nil and index table

                MethodsTable::Push(l);
                lua_pushvalue(l, 2);
                lua_rawget(l, -2); // get value in metatable

                lua_remove(l, -2); // remove metatable

                return 1;
            }

            lua_remove(l, -2); // remove index table
            lua_pushvalue(l, 1); // push userdata

            assert(lua_isuserdata(l, -1));
            assert(lua_isfunction(l, -2));

            lua_call(l, 1, 1);

            return 1;
        }

        static int NewIndexMethod(lua_State* l)
        {
            NewIndexTable::Push(l);
            lua_pushvalue(l, 2);
            int type = lua_rawget(l, -2);
            if (type == LUA_TNIL)
            {
                const char* s = luaL_tolstring(l, 2, nullptr);
                luaL_error(l, "Attempt to set field '%s' on %s", s ? s : "UNCONVRTIBLE_KEY", GetClassName(l));
            }
            lua_remove(l, -2); // remove newindex table
            lua_pushvalue(l, 1); // push userdata
            lua_pushvalue(l, 3); // push value

            assert(lua_isuserdata(l, -2));
            assert(lua_isfunction(l, -3));

            lua_call(l, 2, 0);

            return 0;
        }

        static const char* GetClassName(lua_State* l)
        {
            if (ClassTable::Push(l) == LUA_TNIL)
            {
                lua_pushstring(l, typeid(T).name());
            }
            else
            {
                lua_pushstring(l, "className");
                lua_rawget(l, -2);
            }
            const char* s = lua_tostring(l, -1);
            lua_pop(l, 2);
            return s;
        }

    private:
#pragma region ThrowFunctions
        static void ThrowInvalidUserData(lua_State* l, int index)
        {
            luaL_argerror(l, index, "Invalind UserData");
        }

        static void ThrowWrongUserDataType(lua_State* l, int index)
        {
            luaL_error(l, "Expected %s but got other userdata", GetClassName(l));
        }

        static void ThrowWrongType(lua_State* l, int index)
        {
            luaL_error(l, "Expected %s but got %s", GetClassName(l), lua_typename(l, lua_type(l, index)));
        }

        static void ThrowNoMetaTableForUD(lua_State* l, int index)
        {
            luaL_argerror(l, index, "Expected userdata with metatable");
        }

        static void ThrowUDDestroyed(lua_State* l, int index)
        {
            luaL_argerror(l, index, "Userdata has been destroyed");
        }
#pragma endregion
    };

    template<typename T>
    struct StackType<UserData<T>>
    {
        using UD = UserData<T>;

        static bool Check(lua_State* l, int index)
        {
            return UD::IsUserData(l, index);
        }

        static UD Get(lua_State* l, int index)
        {
            return UD::ValidateUserData(l, index);
        }

        static void Push(lua_State* l, T& value)
        {
            if constexpr (std::is_copy_constructible_v<T>)
            {
                UD::PushCopy(l, value);
            }
            else if constexpr (std::is_move_constructible_v <T>)
            {
                UD::PushCopy(l, std::move(value));
            }
            else
            {
                static_assert(!std::is_move_constructible_v <T> && !std::is_copy_constructible_v<T>, "Can't create copy of userdata! Use direct creation of userdata on stack with UserData<T>::New.");
            }
        }
    };

}