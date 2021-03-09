# pep_mono
option(${PROJECT_NAME_PREFIX}_INSTALL_PEP_MONO_PLUGIN OFF)
if(${PROJECT_NAME_PREFIX}_INSTALL_PEP_MONO_PLUGIN)
  set(_plugin_name plugin_pep_mono)
  find_path(MONO_INCLUDE_DIR mono/jit/jit.h PATH_SUFFIXES include/mono-2.0
            PATHS /Library/Frameworks/Mono.framework/Versions/Current
                              "C:/Program Files/Mono")
  find_library(MONO_LIBRARY_PATH NAMES mono-2.0 mono-2.0-sgen PATH_SUFFIXES lib
               PATHS /Library/Frameworks/Mono.framework/Versions/Current
                                 "C:/Program Files/Mono")
  mark_as_advanced(MONO_INCLUDE_DIR MONO_LIBRARY_PATH)
  add_library(${_plugin_name} SHARED ${CMAKE_CURRENT_SOURCE_DIR}/pep_mono.cc
                                     $<$<BOOL:${WIN32}>:${PROJECT_SOURCE_DIR}/emapp/include/emapp/sdk/win32/Model.def>)
  set_property(TARGET ${_plugin_name} PROPERTY FOLDER plugins)
  set_property(TARGET ${_plugin_name} APPEND PROPERTY INCLUDE_DIRECTORIES ${MONO_INCLUDE_DIR}
                                                                          ${PROJECT_SOURCE_DIR}/emapp/include
                                                                          ${PROJECT_SOURCE_DIR}/dependencies/protobuf-c)
  set_target_properties(${_plugin_name} PROPERTIES OUTPUT_NAME ${_plugin_name} PREFIX "" DEFINE_SYMBOL "")
  target_link_libraries(${_plugin_name} protobuf-c ${MONO_LIBRARY_PATH})
  nanoem_emapp_plugin_install(${_plugin_name})
  # bundle version
  add_library(${_plugin_name}_nanoem SHARED ${PROJECT_SOURCE_DIR}/nanoem/nanoem.c
                                            ${PROJECT_SOURCE_DIR}/nanoem/ext/mutable.c
                                            ${PROJECT_SOURCE_DIR}/nanoem/ext/converter.c
                                            ${PROJECT_BINARY_DIR}/nanoem/version.c)
  nanoem_cmake_enable_lto(${_plugin_name}_nanoem)
  set_property(TARGET ${_plugin_name}_nanoem PROPERTY FOLDER plugins)
  set_property(TARGET ${_plugin_name}_nanoem APPEND PROPERTY COMPILE_DEFINITIONS NANOEM_DLL_EXPORTS)
  set_target_properties(${_plugin_name}_nanoem PROPERTIES OUTPUT_NAME nanoem)
  nanoem_emapp_plugin_install(${_plugin_name}_nanoem)
endif()
