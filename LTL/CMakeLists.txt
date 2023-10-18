# CMakeList.txt: проект CMake для LTL; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#

# Добавьте источник в исполняемый файл этого проекта.
add_executable (LTL "main.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET LTL PROPERTY CXX_STANDARD 17)
endif()

file(GLOB LUA54_FILES
    Lua/lua-5.4.4/src/*.c
    Lua/lua-5.4.4/src/*.h
    Lua/lua-5.4.4/src/*.hpp
    Lua/LuaLibrary.h
)
add_library(Lua54 ${LUA54_FILES})
target_compile_definitions(Lua54 PUBLIC LUA_VERSION=504)
target_include_directories(Lua54 PUBLIC Lua/lua-5.4.4/src)
source_group("Source" FILES ${LUA54_FILES})

 target_link_libraries(LTL
        LuaTemplateLibrary
        Lua54
    )

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/main.lua
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})