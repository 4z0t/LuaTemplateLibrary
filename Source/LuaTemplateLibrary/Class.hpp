#pragma once
#include "LuaAux.hpp"
#include "UserData.hpp"

namespace Lua
{
    template<typename T>
    struct Class
    {
        using UData = UserData<T>;



    };
}