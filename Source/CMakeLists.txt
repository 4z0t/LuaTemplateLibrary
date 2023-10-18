set(LTL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/LuaTemplateLibrary)

set(LTL_HEADERS
    ${LTL_DIR}/FuncArguments.hpp
    ${LTL_DIR}/FuncUtils.hpp
    ${LTL_DIR}/LuaAux.hpp
    ${LTL_DIR}/LuaClass.hpp
    ${LTL_DIR}/LuaFunctions.hpp
    ${LTL_DIR}/LuaState.hpp
    ${LTL_DIR}/LuaTypes.hpp
    ${LTL_DIR}/RefObject.hpp
)

source_group("LTL" FILES ${LTL_HEADERS})

add_library(LuaTemplateLibrary INTERFACE)

target_sources(LuaTemplateLibrary INTERFACE
    ${LTL_HEADERS}
)

target_include_directories(LuaTemplateLibrary INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})


if(MSVC AND CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    add_custom_target(LuaTemplateLibrarySources SOURCES
        ${LTL_HEADERS}
    )
endif()

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    install(DIRECTORY LuaTemplateLibrary TYPE INCLUDE)
endif()

set_target_properties(LuaTemplateLibrary PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
    )