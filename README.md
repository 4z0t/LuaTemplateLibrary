# LuaTemplateLibrary

## Overview

Lua Template Library (LTL for short) is a lightweight C++ 17  library for binding Lua 5.4.
This library pursuits performance with safety and flexibility of C++ templates. It is completely static which means functions and methods used in binding are getting inlined and optimized by compiler.

LTL is made of following components:

* **UserData and Classes** - Used for defining C++ defined structure/class in Lua. Supports Methods and Properties.
* **Function Wrapper** - Used to statically define `lua_CFunction` which involves static type matching.
* **RefObject** - Represents object defined in Lua.
* **StateWrapper** - provides C++ interface for `lua_State`.

## Build and Use

TODO

## Documentation

TODO
  