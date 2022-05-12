option(NANOEM_ENABLE_UPLOAD_PDB "Enable uploading debug symbols to sentry.io" OFF)
option(NANOEM_ENABLE_RENDERDOC_WIN32 "Enable renderdoc integration" OFF)
option(NANOEM_ENABLE_D3D11ON12 "Enable D3D11On12 layer" OFF)
if(MSVC)
  set(CMAKE_EXE_LINKER_FLAGS "/LARGEADDRESSAWARE")
endif()

# version
include(${PROJECT_SOURCE_DIR}/cmake/GetGitRevisionDescription.cmake)
git_describe(NANOEM_BUNDLE_VERSION)
string(REGEX MATCH "^v[0-9]+\\.([0-9]+)\\.([0-9]+)" VERSION_MATCH ${NANOEM_BUNDLE_VERSION})
set(NANOEM_WIN32_VERSION_MAJOR "${CMAKE_MATCH_1}")
set(NANOEM_WIN32_VERSION_MINOR "${CMAKE_MATCH_2}")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.rc.in ${CMAKE_CURRENT_BINARY_DIR}/version.rc @ONLY)

# configure
set(NANOEM_EXECUTABLE_NAME ${PROJECT_NAME}_win32)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src WIN32_SOURCES)
add_executable(${NANOEM_EXECUTABLE_NAME} WIN32 ${CMAKE_CURRENT_SOURCE_DIR}/main.cc ${WIN32_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
set_target_properties(${NANOEM_EXECUTABLE_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${NANOEM_EXECUTABLE_NAME})
target_compile_definitions(${NANOEM_EXECUTABLE_NAME} PRIVATE _UNICODE)
if(TARGET sokol_glcore33)
  target_compile_definitions(${NANOEM_EXECUTABLE_NAME} PRIVATE NANOEM_WIN32_HAS_OPENGL)
endif()
nanoem_emapp_link_executable(${NANOEM_EXECUTABLE_NAME})

set(win32_api_libraries psapi winhttp shlwapi d3d11 dxgi dxguid mf mfplat mfreadwrite mfuuid evr pdh gdi32 shell32 comdlg32 wmcodecdspuuid)
target_link_libraries(${NANOEM_EXECUTABLE_NAME} ${win32_api_libraries})
target_include_directories(${NANOEM_EXECUTABLE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/emapp/resources/include)
if(NANOEM_GA_TRACKING_ID)
  target_compile_definitions(${NANOEM_EXECUTABLE_NAME} PRIVATE NANOEM_GA_TRACKING_ID="${NANOEM_GA_TRACKING_ID}")
endif()

if(NANOEM_ENABLE_RENDERDOC_WIN32)
  find_path(RENDERDOC_LIBRARY_DIR renderdoc.dll PATHS "C:/Program Files/RenderDoc")
  find_path(RENDERDOC_INCLUDE_DIR renderdoc_app.h PATHS "C:/Program Files/RenderDoc")
  mark_as_advanced(RENDERDOC_LIBRARY_DIR RENDERDOC_INCLUDE_DIR)
  target_compile_definitions(${NANOEM_EXECUTABLE_NAME} PRIVATE NANOEM_ENABLE_RENDERDOC)
  target_include_directories(${NANOEM_EXECUTABLE_NAME} PRIVATE ${RENDERDOC_INCLUDE_DIR})
  add_custom_command(TARGET ${NANOEM_EXECUTABLE_NAME} POST_BUILD
                     COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different "${RENDERDOC_LIBRARY_DIR}/renderdoc.dll" "$<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>")
endif()
if(NANOEM_ENABLE_D3D11ON12)
  target_compile_definitions(${NANOEM_EXECUTABLE_NAME} PRIVATE NANOEM_ENABLE_D3D11ON12)
  target_link_libraries(${NANOEM_EXECUTABLE_NAME} d3d12)
endif()

# integration
option(NANOEM_ENABLE_TEST_RUNNER_WIN32 OFF)
if(${NANOEM_ENABLE_TEST_RUNNER_WIN32})
  add_executable(${NANOEM_EXECUTABLE_NAME}_test
                 ${CMAKE_CURRENT_SOURCE_DIR}/test.cc ${WIN32_SOURCES})
  target_include_directories(${NANOEM_EXECUTABLE_NAME}_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/emapp/resources/include)
  target_link_libraries(${NANOEM_EXECUTABLE_NAME}_test ${win32_api_libraries})
  nanoem_emapp_link_executable(${NANOEM_EXECUTABLE_NAME}_test)
endif()

# Sentry Native SDK
if(NANOEM_ENABLE_SENTRY_SDK)
  nanoem_cmake_get_install_path("sentry-native" SENTRY_NATIVE_BASE_PATH SENTRY_NATIVE_INSTALL_PATH_DEBUG SENTRY_NATIVE_INSTALL_PATH_RELEASE)
  find_library(SENTRY_CRASHPAD_PATH sentry PATH_SUFFIXES lib PATHS ${SENTRY_NATIVE_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  find_program(SENTRY_CRASHPAD_HANDLER_PATH crashpad_handler PATH_SUFFIXES bin PATHS ${SENTRY_NATIVE_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  find_path(SENTRY_CRASHPAD_DLL_PATH sentry.dll PATH_SUFFIXES bin PATHS ${SENTRY_NATIVE_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  find_path(SENTRY_INCLUDE_PATH sentry.h PATH_SUFFIXES include PATHS ${SENTRY_NATIVE_INSTALL_PATH_RELEASE})
  mark_as_advanced(SENTRY_CRASHPAD_PATH SENTRY_CRASHPAD_HANDLER_PATH SENTRY_CRASHPAD_DLL_PATH SENTRY_INCLUDE_PATH)
  target_compile_definitions(${NANOEM_EXECUTABLE_NAME} PRIVATE NANOEM_HAS_SENTRY $<$<BOOL:${NANOEM_SENTRY_DSN}>:NANOEM_SENTRY_DSN="${NANOEM_SENTRY_DSN}">)
  target_include_directories(${NANOEM_EXECUTABLE_NAME} PRIVATE ${SENTRY_INCLUDE_PATH})
  target_link_libraries(${NANOEM_EXECUTABLE_NAME} ${SENTRY_CRASHPAD_PATH})
  set_property(TARGET ${NANOEM_EXECUTABLE_NAME} APPEND PROPERTY LINK_OPTIONS "/DEBUG:FULL;/INCREMENTAL:NO")
  set_target_properties(${NANOEM_EXECUTABLE_NAME} PROPERTIES PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/sentry-pdb")
  get_filename_component(SENTRY_CRASHPAD_DIR ${SENTRY_CRASHPAD_PATH} DIRECTORY)
  list(APPEND LIBRARY_PATHS ${SENTRY_CRASHPAD_DIR})
  if(${NANOEM_ENABLE_TEST_RUNNER_WIN32})
    target_compile_definitions(${NANOEM_EXECUTABLE_NAME}_test PRIVATE NANOEM_HAS_SENTRY $<$<BOOL:${NANOEM_SENTRY_DSN}>:NANOEM_SENTRY_DSN="${NANOEM_SENTRY_DSN}">)
    target_include_directories(${NANOEM_EXECUTABLE_NAME}_test PRIVATE ${SENTRY_INCLUDE_PATH})
    target_link_libraries(${NANOEM_EXECUTABLE_NAME}_test ${SENTRY_CRASHPAD_PATH})
  endif()
  add_custom_command(TARGET ${NANOEM_EXECUTABLE_NAME} POST_BUILD
                     COMMAND ${CMAKE_COMMAND} ARGS -E make_directory "$<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>/plugins"
                     COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different "${SENTRY_CRASHPAD_DLL_PATH}/sentry.dll" "$<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>"
                     COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different "${SENTRY_CRASHPAD_HANDLER_PATH}" "$<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>/plugins")
endif()

# plugins
get_property(_plugins GLOBAL PROPERTY NANOEM_PROPERTY_INSTALL_PLUGINS)
if(NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN)
  target_link_libraries(${NANOEM_EXECUTABLE_NAME} ${_plugins})
else()
  foreach(_item ${_plugins})
    set(touch_file "${CMAKE_CURRENT_BINARY_DIR}/intermediate/${_item}.touch")
    add_custom_command(OUTPUT ${touch_file}
                       COMMAND ${CMAKE_COMMAND} ARGS -E touch ${touch_file}
                       COMMAND ${CMAKE_COMMAND} ARGS -E touch ${CMAKE_CURRENT_BINARY_DIR}/version.rc
                       DEPENDS ${_item})
    add_custom_target(${_item}_touch_win32 DEPENDS ${touch_file})
    set_property(TARGET ${_item}_touch_win32 PROPERTY FOLDER plugins/touch)
    add_dependencies(${NANOEM_EXECUTABLE_NAME} ${_item}_touch_win32)
    add_custom_command(TARGET ${NANOEM_EXECUTABLE_NAME} POST_BUILD
                       COMMAND ${CMAKE_COMMAND} ARGS -E make_directory "$<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>/plugins"
                       COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different "$<TARGET_FILE:${_item}>" "$<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>/plugins/$<TARGET_FILE_NAME:${_item}>")
    if(NANOEM_ENABLE_SENTRY_SDK)
      set_property(TARGET ${_item} APPEND PROPERTY LINK_OPTIONS "/DEBUG:FULL;/INCREMENTAL:NO")
      set_target_properties(${_item} PROPERTIES PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/sentry-pdb")
    endif()
  endforeach()
endif()
if(TARGET plugin_ffmpeg)
  set(_libraries "AVCODEC_LIBRARY_DLL;AVFORMAT_LIBRARY_DLL;AVUTIL_LIBRARY_DLL;SWRESAMPLE_LIBRARY_DLL;SWSCALE_LIBRARY_DLL")
  foreach(_item ${_libraries})
    get_filename_component(_filename ${${_item}} NAME)
    add_custom_command(TARGET ${NANOEM_EXECUTABLE_NAME} POST_BUILD DEPENDS plugin_ffmpeg
                       COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different "${${_item}}" "$<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>/plugins/${_filename}")
    install(FILES "${${_item}}" DESTINATION plugins)
  endforeach()
endif()
if(TARGET emapp_peplugin)
  add_custom_command(TARGET ${NANOEM_EXECUTABLE_NAME} POST_BUILD DEPENDS emapp_peplugin
                     COMMAND ${CMAKE_COMMAND} ARGS -E make_directory "$<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>/plugins/Managed"
                     COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different "${PROJECT_SOURCE_DIR}/emapp/plugins/PEPlugin/bin/Release/netstandard2.1/PEPlugin.dll" "$<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>/plugins/Managed")
endif()

# installation
install(TARGETS ${NANOEM_EXECUTABLE_NAME} DESTINATION .)
if(NANOEM_ENABLE_SENTRY_SDK)
  install(FILES ${SENTRY_CRASHPAD_DLL_PATH}/sentry.dll DESTINATION .)
  install(FILES ${SENTRY_CRASHPAD_HANDLER_PATH} DESTINATION plugins)
endif()
install(CODE "
  set(NANOEM_ENABLE_SENTRY_SDK ${NANOEM_ENABLE_SENTRY_SDK})
  set(NANOEM_ENABLE_UPLOAD_PDB ${NANOEM_ENABLE_UPLOAD_PDB})
  if(NANOEM_ENABLE_SENTRY_SDK AND NANOEM_ENABLE_UPLOAD_PDB)
    file(GLOB _plugins \"${CMAKE_CURRENT_BINARY_DIR}/Release/plugins/*.dll\")
    execute_process(COMMAND sentry-cli
                            --auth-token ${NANOEM_SENTRY_AUTH_TOKEN}
                            upload-dif
                            -o ${NANOEM_SENTRY_ORGANIZATION}
                            -p ${NANOEM_SENTRY_PROJECT}
                            \"${CMAKE_CURRENT_BINARY_DIR}/sentry-pdb/Release\"
                            \"${CMAKE_CURRENT_BINARY_DIR}/Release/nanoem.exe\")
    foreach(_item \${_plugins})
    execute_process(COMMAND sentry-cli
                            --auth-token ${NANOEM_SENTRY_AUTH_TOKEN}
                            upload-dif
                            -o ${NANOEM_SENTRY_ORGANIZATION}
                            -p ${NANOEM_SENTRY_PROJECT}
                            \${_item})
    endforeach()
  endif()
")

include(${PROJECT_SOURCE_DIR}/cmake/GetGitRevisionDescription.cmake)
git_describe(${NANOEM_EXECUTABLE_NAME}_VERSION)
set(CPACK_GENERATOR "ZIP")
set(CPACK_STRIP_FILES ON)
set(CPACK_PACKAGE_VENDOR "hkrn")
set(CPACK_PACKAGE_VERSION "${${NANOEM_EXECUTABLE_NAME}_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME} - MikuMikuDance compatible software works on multiple platforms")
set(CPACK_INSTALL_CMAKE_PROJECTS "${PROJECT_BINARY_DIR};${NANOEM_EXECUTABLE_NAME};ALL;/")
set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/license.md)
set(CPACK_NSIS_DISPLAY_NAME ${PROJECT_NAME})
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_NSIS_HELP_LINK "https://nanoem.readthedocs.io")
set(CPACK_NSIS_PACKAGE_NAME ${PROJECT_NAME})

# High-DPI configuration
if(MSVC)
  if("${NANOEM_TARGET_ARCHITECTURE}" STREQUAL "arm64")
    set(NANOEM_WIN32_ARCHITECTURE "arm64")
  elseif("${NANOEM_TARGET_ARCHITECTURE}" STREQUAL "x86_64")
    set(NANOEM_WIN32_ARCHITECTURE "amd64")
  elseif("${NANOEM_TARGET_ARCHITECTURE}" STREQUAL "i386")
    set(NANOEM_WIN32_ARCHITECTURE "x86")
  endif()
  set(NANOEM_WIN32_DPIAWARE "True/PM" CACHE STRING "Enable DPI-awareness for win32 application")
  set_property(CACHE NANOEM_WIN32_DPIAWARE PROPERTY STRINGS "True/PM;False")
  if("${NANOEM_WIN32_DPIAWARE}" STREQUAL "True/PM")
    set(NANOEM_WIN32_DPIAWARENESS "PerMonitorV2")
  else()
    set(NANOEM_WIN32_DPIAWARENESS "unaware")
  endif()
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/nanoem.manifest ${CMAKE_CURRENT_BINARY_DIR}/nanoem.manifest)
  add_custom_command(TARGET ${NANOEM_EXECUTABLE_NAME} POST_BUILD
                     COMMAND "mt.exe" ARGS -nologo
                                      -manifest \"${CMAKE_CURRENT_BINARY_DIR}/nanoem.manifest\"
                                      -inputresource:\"$<TARGET_FILE:${NANOEM_EXECUTABLE_NAME}>\"\;\#1
                                      -outputresource:\"$<TARGET_FILE:${NANOEM_EXECUTABLE_NAME}>\"\;\#1
                     MAIN_DEPENDENCY ${CMAKE_CURRENT_BINARY_DIR}/nanoem.manifest)
  if(${NANOEM_ENABLE_TEST_RUNNER_WIN32})
    add_custom_command(TARGET ${NANOEM_EXECUTABLE_NAME}_test POST_BUILD
                       COMMAND "mt.exe" ARGS -nologo
                                        -manifest \"${CMAKE_CURRENT_BINARY_DIR}/nanoem.manifest\"
                                        -inputresource:\"$<TARGET_FILE:${NANOEM_EXECUTABLE_NAME}_test>\"\;\#1
                                        -outputresource:\"$<TARGET_FILE:${NANOEM_EXECUTABLE_NAME}_test>\"\;\#1
                       MAIN_DEPENDENCY ${CMAKE_CURRENT_BINARY_DIR}/nanoem.manifest)
  endif()
endif()

include(CPack)
