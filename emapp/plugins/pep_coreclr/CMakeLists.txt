# pep_coreclr
option(${PROJECT_NAME_PREFIX}_INSTALL_PEP_CORECLR_PLUGIN OFF)
if(${PROJECT_NAME_PREFIX}_INSTALL_PEP_CORECLR_PLUGIN)
  set(_plugin_name plugin_pep_coreclr)
  add_library(${_plugin_name} SHARED ${CMAKE_CURRENT_SOURCE_DIR}/pep_coreclr.cc
                                     $<$<BOOL:${WIN32}>:${PROJECT_SOURCE_DIR}/emapp/include/emapp/sdk/win32/Model.def>)
  set_property(TARGET ${_plugin_name} PROPERTY FOLDER plugins)
  set_property(TARGET ${_plugin_name} APPEND PROPERTY INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/emapp/include
                                                                          ${PROJECT_SOURCE_DIR}/dependencies/protobuf-c
																		  ${NETCORE_CORECLR_NETHOST_INCLUDE_DIR})
  set_target_properties(${_plugin_name} PROPERTIES OUTPUT_NAME ${_plugin_name} PREFIX "" DEFINE_SYMBOL "")
  target_link_libraries(${_plugin_name} protobuf-c ${NETCORE_CORECLR_NETHOST_LIBRARY})
  nanoem_emapp_plugin_install(${_plugin_name})
endif()
