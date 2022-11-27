set(_plugin_name plugin_ssb)
if(NOT PROJECT_SOURCE_DIR)
  project(plugin_ssb)
  cmake_minimum_required(VERSION 3.5)
  set(_project_source_dir "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
  set(WASM_PLUGIN ON)
else()
  set(_project_source_dir ${PROJECT_SOURCE_DIR})
endif()
set(_plugin_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/ssb.cc
  ${_project_source_dir}/nanoem/nanoem.c
  ${_project_source_dir}/nanoem/ext/mutable.c
  ${_project_source_dir}/dependencies/protobuf-c/protobuf-c/protobuf-c.c
  ${_project_source_dir}/emapp/src/protoc/common.pb-c.c
  ${_project_source_dir}/emapp/src/protoc/plugin.pb-c.c)

add_library(${_plugin_name} SHARED ${_plugin_sources})
if(WASM_PLUGIN)
  set_target_properties(${_plugin_name} PROPERTIES SUFFIX ".wasm" NO_SONAME TRUE)
  target_compile_definitions(${_plugin_name} PRIVATE WASM_WASI_SDK)
  target_compile_options(${_plugin_name} PRIVATE -fno-exceptions)
  target_link_options(${_plugin_name} PRIVATE -Wl,--no-entry -Wl,--export-all)
endif()
set_target_properties(${_plugin_name} PROPERTIES OUTPUT_NAME ${_plugin_name} PREFIX "" DEFINE_SYMBOL "")
if(MSVC)
  target_compile_definitions(${_plugin_name} PRIVATE NANOEM_BUILDING_DLL NANOEM_DLL_EXPORTS)
  target_compile_options(${_plugin_name} PRIVATE /wd4244 /wd4267 -utf-8)
endif()
set_property(TARGET ${_plugin_name} PROPERTY FOLDER plugins)
set_property(TARGET ${_plugin_name} PROPERTY INCLUDE_DIRECTORIES
  ${_project_source_dir}
  ${_project_source_dir}/dependencies/protobuf-c
  ${_project_source_dir}/dependencies/glm
  ${_project_source_dir}/emapp/src/protoc)

# install(TARGETS ${_plugin_name} DESTINATION .)
#set(CPACK_GENERATOR "ZIP")
#set(CPACK_STRIP_FILES ON)
#set(CPACK_PACKAGE_VENDOR "hkrn")
#set(CPACK_PACKAGE_VERSION "1.1.0")
#set(CPACK_INSTALL_CMAKE_PROJECTS "${PROJECT_BINARY_DIR};${_plugin_name};ALL;/")
#include(CPack)
