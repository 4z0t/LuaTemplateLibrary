#pragma once



#if defined(LTL_SAME_LUA_DIR)
/*
* ��������� ������������ ����� Lua, ������� � ����� ����� � LTL
*/
extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}
#elif defined(LTL_INCLUDED_LUA_DIR)
/*
* ��������� ������������ ����� Lua, ������� �������� � ������.
*/
extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}
#else
/*
    �������� �� ���������� ������������ ������ Lua.
    �������� ��� ������������� �� ���� ���� � ������������� �������
    ��� ����������� ������� ����.
*/
extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}
#endif