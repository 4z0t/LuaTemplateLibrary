set(LTL_TEST_SOURCE_FILES
    Source/RefObject.cpp
    Source/Exception.cpp
    Source/Main.cpp
)

source_group("Source" FILES ${LTL_TEST_SOURCE_FILES})


# Lua 5.4
file(GLOB LUA54_FILES
    Lua/master/*.c
    Lua/master/*.h
    Lua/master/*.hpp
    Lua/LuaLibrary.h
)
list(FILTER LUA54_FILES EXCLUDE REGEX ".*lua\\.c$")
add_library(Lua54 ${LUA54_FILES})
target_compile_definitions(Lua54 PUBLIC LUA_VERSION=504)
target_include_directories(Lua54 PUBLIC Lua/master)
source_group("Source" FILES ${LUA54_FILES})

# LuaJIT
file(GLOB LUAJIT_FILES
    LuaJIT/master/src/*.h
    LuaJIT/master/src/*.hpp
    LuaJIT/LuaLibrary.h
)

list(FILTER LUAJIT_FILES EXCLUDE REGEX ".*lua\\.c$")
add_library(LuaJIT ${LUAJIT_FILES})
target_compile_definitions(LuaJIT PUBLIC LUA_VERSION=504)
target_include_directories(LuaJIT PUBLIC LuaJIT/master/src)
source_group("Source" FILES ${LUAJIT_FILES})
set_target_properties(LuaJIT PROPERTIES
        CXX_STANDARD 17
        LINKER_LANGUAGE CXX
)

macro(add_test_module Module)
    add_executable(Test_${Module}
        ${LTL_TEST_SOURCE_FILES}
    )

    set_target_properties(Test_${Module} PROPERTIES
        CXX_STANDARD 17
        LINKER_LANGUAGE CXX
    )
    target_include_directories(Test_${Module} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

    target_link_libraries(Test_${Module}
        LuaTemplateLibrary
        ${Module}
        GTest::gtest
    )

    target_compile_options(Test_${Module} PRIVATE "$<$<CONFIG:Release>:/Zi;/EHa>")
    target_link_options(Test_${Module} PRIVATE "$<$<CONFIG:Release>:/DEBUG>")
    target_link_options(Test_${Module} PRIVATE "$<$<CONFIG:Release>:/OPT:REF>")
    target_link_options(Test_${Module} PRIVATE "$<$<CONFIG:Release>:/OPT:ICF>")

    add_test(Test_${Module} Test_${Module})
endmacro(add_test_module)

add_test_module(Lua54)
add_test_module(LuaJIT)

add_executable(FreeTest_LTL
    Main.cpp
)
set_target_properties(FreeTest_LTL PROPERTIES
    CXX_STANDARD 17
)
target_link_libraries(FreeTest_LTL
    LuaTemplateLibrary
    Lua54
)
target_compile_options(FreeTest_LTL PRIVATE "$<$<CONFIG:Release>:/Zi;/EHa>")
target_link_options(FreeTest_LTL PRIVATE "$<$<CONFIG:Release>:/DEBUG>")
target_link_options(FreeTest_LTL PRIVATE "$<$<CONFIG:Release>:/OPT:REF>")
target_link_options(FreeTest_LTL PRIVATE "$<$<CONFIG:Release>:/OPT:ICF>")
add_custom_command(
        TARGET FreeTest_LTL  POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/main.lua
                ${CMAKE_CURRENT_BINARY_DIR}/main.lua)