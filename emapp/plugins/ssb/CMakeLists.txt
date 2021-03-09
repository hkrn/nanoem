set(_plugin_name plugin_ssb)
set(_plugin_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/ssb.cc
  ${PROJECT_SOURCE_DIR}/nanoem/nanoem.c
  ${PROJECT_SOURCE_DIR}/nanoem/ext/mutable.c
  ${PROJECT_SOURCE_DIR}/dependencies/protobuf-c/protobuf-c/protobuf-c.c
  ${PROJECT_SOURCE_DIR}/emapp/src/protoc/common.pb-c.c
  ${PROJECT_SOURCE_DIR}/emapp/src/protoc/plugin.pb-c.c)

if(EMSCRIPTEN)
  add_executable(${_plugin_name} ${_plugin_sources})
  set_target_properties(${_plugin_name} PROPERTIES LINK_FLAGS "-s WASM=1 -s STANDALONE_WASM -s ALLOW_MEMORY_GROWTH")
else()
  add_library(${_plugin_name} SHARED ${_plugin_sources})
  set_target_properties(${_plugin_name} PROPERTIES OUTPUT_NAME ${_plugin_name} PREFIX "" DEFINE_SYMBOL "")
  if(MSVC)
    set_property(TARGET ${_plugin_name} PROPERTY COMPILE_DEFINITIONS NANOEM_BUILDING_DLL NANOEM_DLL_EXPORTS)
    set_property(TARGET ${_plugin_name} PROPERTY COMPILE_OPTIONS "-utf-8")
  endif()
endif()
set_property(TARGET ${_plugin_name} PROPERTY FOLDER plugins)
set_property(TARGET ${_plugin_name} PROPERTY INCLUDE_DIRECTORIES
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/dependencies/protobuf-c
  ${PROJECT_SOURCE_DIR}/dependencies/glm
  ${PROJECT_SOURCE_DIR}/emapp/src/protoc)

# install(TARGETS ${_plugin_name} DESTINATION .)
#set(CPACK_GENERATOR "ZIP")
#set(CPACK_STRIP_FILES ON)
#set(CPACK_PACKAGE_VENDOR "hkrn")
#set(CPACK_PACKAGE_VERSION "1.1.0")
#set(CPACK_INSTALL_CMAKE_PROJECTS "${PROJECT_BINARY_DIR};${_plugin_name};ALL;/")
#include(CPack)
