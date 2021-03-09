# gif
option(${PROJECT_NAME_PREFIX}_INSTALL_GIF_PLUGIN ON)
if(${PROJECT_NAME_PREFIX}_INSTALL_GIF_PLUGIN)
  set(_plugin_name plugin_gif)
  add_library(${_plugin_name} SHARED ${CMAKE_CURRENT_SOURCE_DIR}/gif.cc
                                      $<$<BOOL:${WIN32}>:${PROJECT_SOURCE_DIR}/emapp/include/emapp/sdk/win32/Encoder.def>)
  target_link_libraries(${_plugin_name} bx)
  set_property(TARGET ${_plugin_name} PROPERTY FOLDER plugins)
  set_property(TARGET ${_plugin_name} APPEND PROPERTY INCLUDE_DIRECTORIES ${BX_INCLUDE_DIR} ${BX_COMPAT_INCLUDE_PATH} ${PROJECT_SOURCE_DIR}/emapp/include)
  set_target_properties(${_plugin_name} PROPERTIES OUTPUT_NAME ${_plugin_name} PREFIX "" DEFINE_SYMBOL "")
  nanoem_emapp_plugin_install(${_plugin_name})
endif()
