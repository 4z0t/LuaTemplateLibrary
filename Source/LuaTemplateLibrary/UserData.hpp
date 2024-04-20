#pragma once
#include "LuaAux.hpp"
#include "LuaTypes.hpp"
#include "FuncArguments.hpp"
#include "Exception.hpp"
#include "RefObject.hpp"

namespace LTL
{
    template<typename T>
    struct RegistryTableBase
    {
        static const void* GetKey()
        {
            static const char key;
            return &key;
        }

        static int Push(lua_State* l)
        {
            return lua_getregp(l, RegistryTableBase<T>::GetKey());
        }
    };

    /**
     * @brief Класс представляющий пользовательский тип в Lua
     *
     * @tparam T пользовательский тип
     */
    template<typename T>
    struct UserData : Internal::UserDataValue<T>
    {
        using UserDataValue::UserDataValue;

        struct MetaTable : public RegistryTableBase<MetaTable> {};
        struct ClassTable : public RegistryTableBase<ClassTable> {};
        struct IndexTable : public RegistryTableBase<IndexTable> {};
        struct NewIndexTable : public RegistryTableBase<NewIndexTable> {};

        struct Data
        {
            T object;
            bool isDestroyed = false;
        };

        /**
         * @brief Помещает на стек UserData<T> и возвращает указатель на его место
         * в памяти для дальнейшего использования с оператором new
         *
         * @param l
         * @return void* const
         */
        static void* const  Allocate(lua_State* l)
        {
            Data* const object = (Data*)lua_newuserdata(l, sizeof(Data));
            object->isDestroyed = false;
            SetClassMetaTable(l);
            return &object->object;
        }

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
            new(Allocate(l)) T(std::forward<TArgs>(args)...);
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

                MetaTable::Push(l);
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
                lua_pop(l, 2); // pop nil and index table

                const char* s = lua_tostring(l, 2);
                luaL_error(l, "Attempt to set field '%s' on %s", s ? s : "NONSTRINGVALUE", GetClassName(l));

                return 1;
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
            ClassTable::Push(l);
            lua_pushstring(l, "className");
            lua_rawget(l, -2);
            const char* s = lua_tostring(l, -1);
            lua_pop(l, 2);
            return s;
        }

    private:
#pragma region ThrowFunctions
        static void ThrowInvalidUserData(lua_State* l, int index)
        {
            luaL_argerror(l, index, "invalind UserData");
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
            luaL_argerror(l, index, "Userdata was destroyed");
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

        static void Push(lua_State* l, T&& value)
        {
            UD::PushCopy(l, std::forward<T>(value));
        }

        static void Push(lua_State* l, const T& value)
        {
            UD::PushCopy(l, value);
        }
    };

}