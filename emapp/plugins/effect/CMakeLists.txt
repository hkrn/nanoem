# effect
option(${PROJECT_NAME_PREFIX}_INSTALL_EFFECT_PLUGIN ON)
file(GLOB EFFECT_PROTO_SOURCES ${PROJECT_SOURCE_DIR}/emapp/src/effect*.pb-c.c)
if(${PROJECT_NAME_PREFIX}_INSTALL_EFFECT_PLUGIN)
  nanoem_emapp_plugin_set_library_type()
  set(FX9_DEPENDENCIES_PATH ${PROJECT_SOURCE_DIR}/dependencies)
  set(FX9_PROTOBUF_PATH ${PROJECT_SOURCE_DIR}/emapp/src/protoc)
  set(FX9_PATH ${PROJECT_SOURCE_DIR}/dependencies/fx9)
  set(FX9_TARGET_COMPILER ${NANOEM_TARGET_COMPILER})
  set(FX9_TARGET_ARCHITECTURE ${NANOEM_TARGET_ARCHITECTURE})
  include(${FX9_PATH}/cmake/common.cmake)
  nanoem_cmake_enable_lto(fx9)
  # plugin
  set(_plugin_name plugin_effect)
  add_library(${_plugin_name} ${_library_type} ${CMAKE_CURRENT_SOURCE_DIR}/effect.cc
              $<$<BOOL:${WIN32}>:${PROJECT_SOURCE_DIR}/emapp/include/emapp/sdk/win32/Effect.def>)
  nanoem_cmake_enable_lto(${_plugin_name})
  set_property(TARGET ${_plugin_name} PROPERTY FOLDER plugins)
  set_property(TARGET ${_plugin_name} APPEND PROPERTY INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/dependencies/fx9/include
                                     ${BX_INCLUDE_DIR}
                                     ${BX_COMPAT_INCLUDE_PATH}
                                     ${PROJECT_SOURCE_DIR}/emapp/include
                                     ${FX9_DEPENDENCIES_PATH}/glslang)
  set_property(TARGET ${_plugin_name} APPEND PROPERTY COMPILE_DEFINITIONS $<$<BOOL:${MSVC}>:NOMINMAX>)
  if(WIN32 AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set_property(TARGET ${_plugin_name} PROPERTY CXX_STANDARD 14)
  else()
    set_property(TARGET ${_plugin_name} PROPERTY CXX_STANDARD 11)
  endif()
  set_target_properties(${_plugin_name} PROPERTIES OUTPUT_NAME ${_plugin_name} PREFIX "" DEFINE_SYMBOL "")
  target_link_libraries(${_plugin_name} fx9)
  nanoem_emapp_plugin_install(${_plugin_name})
endif()
