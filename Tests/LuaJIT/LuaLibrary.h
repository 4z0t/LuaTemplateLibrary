#pragma once

#if LUA_VERSION >= 501

#include "luajit.hpp"
#else

#error "LUA_VERSION is not defined"

#endif
