#pragma once

#if LUA_VERSION >= 501

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}
#else

#error "LUA_VERSION is not defined"

#endif
