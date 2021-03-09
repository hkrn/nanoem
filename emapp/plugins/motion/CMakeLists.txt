# motion
option(${PROJECT_NAME_PREFIX}_INSTALL_MOTION_PLUGIN ON)
if(${PROJECT_NAME_PREFIX}_INSTALL_MOTION_PLUGIN)
  set(_plugin_name plugin_motion)
  add_library(${_plugin_name} SHARED ${CMAKE_CURRENT_SOURCE_DIR}/motion.cc
                                      $<$<BOOL:${WIN32}>:${PROJECT_SOURCE_DIR}/emapp/include/emapp/sdk/win32/Motion.def>)
  set_property(TARGET ${_plugin_name} PROPERTY FOLDER plugins)
  set_property(TARGET ${_plugin_name} APPEND PROPERTY INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/emapp/include ${PROJECT_SOURCE_DIR}/dependencies/protobuf-c)
  set_target_properties(${_plugin_name} PROPERTIES OUTPUT_NAME ${_plugin_name} PREFIX "" DEFINE_SYMBOL "")
  target_link_libraries(${_plugin_name} protobuf-c)
  nanoem_emapp_plugin_install(${_plugin_name})
endif()
