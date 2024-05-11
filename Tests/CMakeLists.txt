set(LTL_TEST_SOURCE_FILES
    Source/RefObject.cpp
    Source/Stack.cpp
    Source/Exception.cpp
    Source/Main.cpp
    Source/TestBase.hpp
    Source/STL.cpp
    Source/State.cpp
    Source/UserData.cpp
    Source/Misc.cpp
    Source/Types.cpp
    Source/Libs.cpp
)

source_group("Source" FILES ${LTL_TEST_SOURCE_FILES})

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

add_executable(TestLTL
    ${LTL_TEST_SOURCE_FILES}
)

add_executable(FreeTest_LTL
    Main.cpp
)

set_target_properties(TestLTL FreeTest_LTL PROPERTIES
    CXX_STANDARD 17
)

target_include_directories(TestLTL PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(TestLTL
    LuaTemplateLibrary
    Lua54
    GTest::gtest
)
target_link_libraries(FreeTest_LTL
    LuaTemplateLibrary
    Lua54
)

target_compile_options(FreeTest_LTL PRIVATE "$<$<CONFIG:Release>:/Zi;/EHa>")
target_link_options(FreeTest_LTL PRIVATE "$<$<CONFIG:Release>:/DEBUG>")
target_link_options(FreeTest_LTL PRIVATE "$<$<CONFIG:Release>:/OPT:REF>")
target_link_options(FreeTest_LTL PRIVATE "$<$<CONFIG:Release>:/OPT:ICF>")

target_compile_options(TestLTL PRIVATE "/Zi;/EHs")
target_compile_options(TestLTL PRIVATE "$<$<CONFIG:Release>:/MD>")
target_compile_options(TestLTL PRIVATE "$<$<CONFIG:Debug>:/MDd>")
target_compile_options(TestLTL PRIVATE "/permissive-")
target_link_options(TestLTL PRIVATE "$<$<CONFIG:Release>:/OPT:REF>")
target_link_options(TestLTL PRIVATE "$<$<CONFIG:Release>:/OPT:ICF>")

add_custom_command(
        TARGET FreeTest_LTL  POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/main.lua
                ${CMAKE_CURRENT_BINARY_DIR}/main.lua)


add_test(TestLTL TestLTL)