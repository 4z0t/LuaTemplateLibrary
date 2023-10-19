set(LTL_TEST_SOURCE_FILES
    Source/RefObject.cpp
    Source/Main.cpp
)


source_group("Source" FILES ${LTL_TEST_SOURCE_FILES})


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

add_executable(TestLTL
    ${LTL_TEST_SOURCE_FILES}
)

set_target_properties(TestLTL PROPERTIES
    CXX_STANDARD 17
)

target_include_directories(TestLTL PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(TestLTL
    LuaTemplateLibrary
    Lua54
    GTest::gtest
)

add_test(TestLTL TestLTL)