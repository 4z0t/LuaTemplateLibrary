set(LTL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/LuaTemplateLibrary)

set(LTL_HEADERS
    ${LTL_DIR}/Internal.hpp
    ${LTL_DIR}/FuncArguments.hpp
    ${LTL_DIR}/FuncUtils.hpp
    ${LTL_DIR}/LuaAux.hpp
    ${LTL_DIR}/UserData.hpp
    ${LTL_DIR}/Class.hpp
    ${LTL_DIR}/LuaFunctions.hpp
    ${LTL_DIR}/LuaState.hpp
    ${LTL_DIR}/LuaTypes.hpp
    ${LTL_DIR}/RefObject.hpp
    ${LTL_DIR}/Exception.hpp
    ${LTL_DIR}/LTL.hpp
)

source_group("LTL" FILES ${LTL_HEADERS})

add_library(LuaTemplateLibrary INTERFACE)

set_target_properties(LuaTemplateLibrary PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
    )


target_sources(LuaTemplateLibrary INTERFACE
    ${LTL_HEADERS}
)

target_include_directories(LuaTemplateLibrary INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)