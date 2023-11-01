cmake_minimum_required (VERSION 3.8)
include(CMakeDependentOption)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project (LuaTemplateLibrary)

add_subdirectory (Source)
#add_subdirectory (LTL)

cmake_dependent_option(LTL_TESTING "Build tests" ON
    "CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF
)

if(LTL_TESTING)
    find_package(GTest)
    if(NOT GTest_FOUND)
        set(gtest_force_shared_crt ON CACHE BOOL "Use /MD and /MDd" FORCE)
        add_subdirectory(googletest EXCLUDE_FROM_ALL)
        add_library(GTest::gtest ALIAS gtest)
    endif()

    enable_testing()
    add_subdirectory(Tests)
endif()



