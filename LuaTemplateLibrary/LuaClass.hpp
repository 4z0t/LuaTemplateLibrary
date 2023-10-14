#pragma once
#include "LuaAux.hpp"

namespace Lua
{
    using std::size_t;

    constexpr size_t RegisterSize(const struct luaL_Reg* reg)
    {
        size_t s = 0;
        while ((reg + s)->name != NULL) s++;
        return s;
    }

    template<class TClass>
    class ClassWrapper
    {
        //static_assert(std::is_same<decltype(TClass::meta),const luaL_Reg[]>::value);
        static_assert(TClass::meta != nullptr, "Metatable required");
    public:
        static void Init(lua_State* l, const char* name, const struct luaL_Reg class_reg[])
        {
            //create metatable with class name as metatable name
            luaL_newmetatable(l, typeid(TClass).name());
            lua_pushstring(l, "__index");
            lua_pushvalue(l, -2);
            lua_settable(l, -3);
            luaL_setfuncs(l, TClass::meta, 0);

            //create lib
            lua_createtable(l, 0, RegisterSize(class_reg));
            luaL_setfuncs(l, class_reg, 0);
            lua_setglobal(l, name);
        }
    };

}