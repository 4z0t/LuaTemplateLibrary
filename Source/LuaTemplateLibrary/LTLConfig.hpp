#pragma once



#if defined(LTL_SAME_LUA_DIR)
/*
* Добавляет заголовочные файлы Lua, которые в одной папке с LTL
*/
extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}
#elif defined(LTL_INCLUDED_LUA_DIR)
/*
* Добавляет заголовочные файлы Lua, которые включены в сборку.
*/
extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}
#else
/*
    Отвечает за добавление заголовочных файлов Lua.
    Измените при необходимости на свой файл с заголовочными файлами
    или используйте макросы выше.
*/
extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}
#endif