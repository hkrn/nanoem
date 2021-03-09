# pep_fxr
option(${PROJECT_NAME_PREFIX}_INSTALL_PEP_FXR_PLUGIN OFF)
if(${PROJECT_NAME_PREFIX}_INSTALL_PEP_FXR_PLUGIN)
  set(NETHOST_PATH_WIN32 "C:/Program Files/dotnet/packs/Microsoft.NETCore.App.Host.win-x64/3.0.0/runtimes/win-x64/native")
  set(NETHOST_PATH_MACOS "/usr/local/share/dotnet/packs/Microsoft.NETCore.App.Host.osx-x64/3.0.0/runtimes/osx-x64/native")
  find_path(NETCORE_FXR_NETHOST_INCLUDE_DIR nethost.h PATHS ${NETHOST_PATH_WIN32} ${NETHOST_PATH_MACOS})
  find_library(NETCORE_FXR_NETHOST_LIBRARY nethost PATHS ${NETHOST_PATH_WIN32} ${NETHOST_PATH_MACOS})
  mark_as_advanced(NETCORE_FXR_NETHOST_INCLUDE_DIR NETCORE_FXR_NETHOST_LIBRARY)
  set(_plugin_name plugin_pep_fxr)
  add_library(${_plugin_name} SHARED ${CMAKE_CURRENT_SOURCE_DIR}/pep_fxr.cc
                                     $<$<BOOL:${WIN32}>:${PROJECT_SOURCE_DIR}/emapp/include/emapp/sdk/win32/Model.def>)
  set_property(TARGET ${_plugin_name} PROPERTY FOLDER plugins)
  set_property(TARGET ${_plugin_name} APPEND PROPERTY INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/emapp/include
                                                                          ${PROJECT_SOURCE_DIR}/dependencies/protobuf-c
                                                                                                                                                  ${NETCORE_FXR_NETHOST_INCLUDE_DIR})
  set_target_properties(${_plugin_name} PROPERTIES OUTPUT_NAME ${_plugin_name} PREFIX "" DEFINE_SYMBOL "")
  target_link_libraries(${_plugin_name} protobuf-c ${NETCORE_FXR_NETHOST_LIBRARY})
  nanoem_emapp_plugin_install(${_plugin_name})
endif()
