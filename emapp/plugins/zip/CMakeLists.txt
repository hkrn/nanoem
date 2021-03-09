# zip
option(${PROJECT_NAME_PREFIX}_INSTALL_ZIP_PLUGIN ON)
if(${PROJECT_NAME_PREFIX}_INSTALL_ZIP_PLUGIN)
  set(MINIZIP_SUBMODULE_PATH "${PROJECT_SOURCE_DIR}/dependencies/minizip")
  set(_plugin_name plugin_zip)
  add_library(${_plugin_name} SHARED ${CMAKE_CURRENT_SOURCE_DIR}/zip.cc
                                      $<$<BOOL:${WIN32}>:${PROJECT_SOURCE_DIR}/emapp/include/emapp/sdk/win32/Encoder.def>)
  set_property(TARGET ${_plugin_name} PROPERTY FOLDER plugins)
  set_property(TARGET ${_plugin_name} APPEND PROPERTY INCLUDE_DIRECTORIES
                                      ${BX_INCLUDE_DIR}
                                      ${BX_COMPAT_INCLUDE_PATH}
                                      ${MINIZIP_SUBMODULE_PATH}
                                      ${ZLIB_INCLUDE_DIR}
                                      ${PROJECT_SOURCE_DIR}/emapp/include
                                      ${PROJECT_SOURCE_DIR}/dependencies/stb
                                      ${PROJECT_SOURCE_DIR}/dependencies/minizip)
  set_target_properties(${_plugin_name} PROPERTIES OUTPUT_NAME ${_plugin_name} PREFIX "" DEFINE_SYMBOL "")
  target_link_libraries(${_plugin_name} minizip bx)
  nanoem_emapp_plugin_install(${_plugin_name})
endif()