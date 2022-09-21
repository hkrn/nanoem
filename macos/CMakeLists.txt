option(NANOEM_ENABLE_UPLOAD_DSYM "Enable uploading debug symbols to sentry.io" OFF)
option(NANOEM_ENABLE_ASAN_XCODE "Enable ASAN on Xcode (requires CMake 3.13 or greater)" OFF)
set(NANOEM_EXECUTABLE_NAME ${PROJECT_NAME}_macos)

if(NANOEM_ENABLE_ASAN_XCODE VERSION_GREATER_EQUAL "3.13.0")
  set(CMAKE_XCODE_GENERATE_SCHEME ON)
  set(CMAKE_XCODE_SCHEME_ADDRESS_SANITIZER ON)
  set(CMAKE_XCODE_SCHEME_ADDRESS_SANITIZER_USE_AFTER_RETURN ON)
endif()

set(NANOEM_MACOS_ENABLE_HIGH_RESOLUTION "true" CACHE STRING "Enable high resolution configuration for Retina display")
set_property(CACHE NANOEM_MACOS_ENABLE_HIGH_RESOLUTION PROPERTY STRINGS "true;false")

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/classes OSX_SOURCES)
add_executable(${NANOEM_EXECUTABLE_NAME} MACOSX_BUNDLE ${OSX_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/main.mm)
target_include_directories(${NANOEM_EXECUTABLE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/classes ${PROJECT_BINARY_DIR}/emapp/resources/include)

include(${PROJECT_SOURCE_DIR}/cmake/GetGitRevisionDescription.cmake)
git_describe(NANOEM_BUNDLE_VERSION)
string(REGEX MATCH "^v([0-9]+\\.[0-9]+\\.[0-9]+)" VERSION_MATCH ${NANOEM_BUNDLE_VERSION})
set_target_properties(${NANOEM_EXECUTABLE_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME}
                      MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME}"
                      MACOSX_BUNDLE_ICON_FILE "${PROJECT_NAME}.icns"
                      MACOSX_BUNDLE_GUI_IDENTIFIER "com.github.nanoem"
                      MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_NAME} ${NANOEM_BUNDLE_VERSION}"
                      MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}"
                      MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_MATCH_1}"
                      MACOSX_BUNDLE_COPYRIGHT "Copyright 2015-2020 hkrn All Rights Reserved."
                      MACOSX_BUNDLE_BUNDLE_VERSION "${NANOEM_BUNDLE_VERSION}"
                      MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
                      XCODE_ATTRIBUTE_CLANG_ENABLE_MODULES "YES"
                      XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME "NO")
nanoem_emapp_link_executable(${NANOEM_EXECUTABLE_NAME})

find_program(SW_VERS_PATH sw_vers)
mark_as_advanced(SW_VERS_PATH)
exec_program(${SW_VERS_PATH} ARGS -productVersion OUTPUT_VARIABLE MACOSX_VERSION_STRING)
string(REGEX REPLACE "(10\\.[0-9]+)(\\.[0-9]+)?" "\\1" MACOSX_VERSION "${MACOSX_VERSION_STRING}")
if(MACOSX_VERSION VERSION_GREATER "10.6")
  target_link_libraries(${NANOEM_EXECUTABLE_NAME} "-framework AVFoundation -framework CoreMedia -weak_framework MetalKit")
else()
  target_link_libraries(${NANOEM_EXECUTABLE_NAME} "-framework QTKit")
endif()
target_link_libraries(${NANOEM_EXECUTABLE_NAME} "-framework AudioUnit -framework AudioToolbox -framework CoreAudio -weak_framework Security")

# integration
option(NANOEM_ENABLE_TEST_RUNNER_OSX OFF)
if(${NANOEM_ENABLE_TEST_RUNNER_OSX})
  set(_executable ${NANOEM_EXECUTABLE_NAME}_integration)
  add_executable(${_executable} ${CMAKE_CURRENT_SOURCE_DIR}/test.mm ${OSX_SOURCES})
  target_include_directories(${_executable} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/classes ${PROJECT_SOURCE_DIR}/dependencies/plog/include)
  # force enabling NANOEM_ENABLE_LOGGING
  target_compile_definitions(${_executable} PRIVATE NANOEM_ENABLE_LOGGING)
  if(MACOSX_VERSION VERSION_GREATER "10.6")
    target_link_libraries(${_executable} "-framework AVFoundation -framework CoreMedia -weak_framework MetalKit")
  else()
    target_link_libraries(${_executable} "-framework QTKit")
  endif()
  nanoem_emapp_link_executable(${_executable})
  target_link_libraries(${_executable} "-framework AudioUnit -framework AudioToolbox -framework CoreAudio -weak_framework Security")
endif()

# Sentry Native SDK
if(NANOEM_ENABLE_SENTRY_SDK)
  nanoem_cmake_get_install_path("sentry-native" SENTRY_NATIVE_BASE_PATH SENTRY_NATIVE_INSTALL_PATH_DEBUG SENTRY_NATIVE_INSTALL_PATH_RELEASE)
  find_library(SENTRY_LIBRARY_PATH sentry PATH_SUFFIXES lib PATHS ${SENTRY_NATIVE_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  find_program(SENTRY_CRASHPAD_HANDLER_PATH crashpad_handler PATH_SUFFIXES bin PATHS ${SENTRY_NATIVE_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  mark_as_advanced(SENTRY_LIBRARY_PATH SENTRY_CRASHPAD_HANDLER_PATH)
  target_compile_definitions(${NANOEM_EXECUTABLE_NAME} PRIVATE NANOEM_HAS_SENTRY $<$<BOOL:${NANOEM_SENTRY_DSN}>:NANOEM_SENTRY_DSN="${NANOEM_SENTRY_DSN}">)
  get_filename_component(SENTRY_CRASHPAD_DIR ${SENTRY_LIBRARY_PATH} DIRECTORY)
  list(APPEND LIBRARY_PATHS ${SENTRY_CRASHPAD_DIR})
  if(${NANOEM_ENABLE_TEST_RUNNER_OSX})
    target_compile_definitions(${NANOEM_EXECUTABLE_NAME}_integration PRIVATE NANOEM_HAS_SENTRY $<$<BOOL:${NANOEM_SENTRY_DSN}>:NANOEM_SENTRY_DSN="${NANOEM_SENTRY_DSN}">)
  endif()
  add_custom_command(TARGET ${NANOEM_EXECUTABLE_NAME} POST_BUILD
                     COMMAND ${CMAKE_COMMAND} ARGS -E make_directory "$<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>/../PlugIns"
                     COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different "${SENTRY_LIBRARY_PATH}" "$<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>/../PlugIns"
                     COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different "${SENTRY_CRASHPAD_HANDLER_PATH}" "$<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>/../PlugIns/crashpad_handler")
endif()

# tracking
if(NANOEM_GA_TRACKING_ID)
  target_compile_definitions(${NANOEM_EXECUTABLE_NAME} PRIVATE NANOEM_GA_TRACKING_ID="${NANOEM_GA_TRACKING_ID}")
endif()
if(DEFINED ENV{NANOEM_TRIAL_EXPIRATION_TIME})
  target_compile_definitions(${NANOEM_EXECUTABLE_NAME} PRIVATE NANOEM_TRIAL_EXPIRATION_TIME=$ENV{NANOEM_TRIAL_EXPIRATION_TIME})
endif()

# plugins
get_property(_plugins GLOBAL PROPERTY NANOEM_PROPERTY_INSTALL_PLUGINS)
if(NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN)
  target_link_libraries(${NANOEM_EXECUTABLE_NAME} ${_plugins})
  if(${NANOEM_ENABLE_TEST_RUNNER_OSX})
      target_link_libraries(${NANOEM_EXECUTABLE_NAME}_integration ${_plugins})
  endif()
else()
  foreach(_item ${_plugins})
    set(touch_file "${CMAKE_CURRENT_BINARY_DIR}/intermediate/${_item}.touch")
    add_custom_command(OUTPUT ${touch_file}
                       COMMAND ${CMAKE_COMMAND} ARGS -E touch ${touch_file}
                       COMMAND ${CMAKE_COMMAND} ARGS -E touch ${CMAKE_CURRENT_SOURCE_DIR}/main.mm
                       DEPENDS ${_item})
    add_custom_target(${_item}_touch_macos DEPENDS ${touch_file})
    set_property(TARGET ${_item}_touch_macos PROPERTY FOLDER plugins/touch)
    add_dependencies(${NANOEM_EXECUTABLE_NAME} ${_item}_touch_macos)
    add_custom_command(TARGET ${NANOEM_EXECUTABLE_NAME} POST_BUILD
                       COMMAND ${CMAKE_COMMAND} ARGS -E make_directory "$<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>/../PlugIns"
                       COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different "$<TARGET_FILE:${_item}>" "$<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>/../PlugIns/$<TARGET_FILE_NAME:${_item}>")
  endforeach()
endif()

# pandoc
find_program(PANDOC_PATH pandoc HINTS /opt/local/bin /usr/local/bin /usr/bin)
mark_as_advanced(PANDOC_PATH)
if(PANDOC_PATH)
  set(intermediate_path ${CMAKE_CURRENT_BINARY_DIR}/Credits.rtf)
  set(output_path ${CMAKE_CURRENT_SOURCE_DIR}/Resources/English.lproj/Credits.rtf)
  add_custom_command(OUTPUT ${intermediate_path}
                     COMMAND ${PANDOC_PATH} ARGS --standalone -f markdown -t rtf ${PROJECT_SOURCE_DIR}/emapp/resources/credits.md -o ${intermediate_path}
                     COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${intermediate_path} ${output_path}
                     MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/emapp/resources/credits.md)
  add_custom_target(${NANOEM_EXECUTABLE_NAME}_credits SOURCES ${intermediate_path})
  set_property(TARGET ${NANOEM_EXECUTABLE_NAME}_credits PROPERTY FOLDER nanoem/resources)
  if(TARGET emarb_credits)
    add_dependencies(${NANOEM_EXECUTABLE_NAME}_credits emarb_credits)
    add_dependencies(${NANOEM_EXECUTABLE_NAME} ${NANOEM_EXECUTABLE_NAME}_credits)
  endif()
endif()

# installation
install(TARGETS ${NANOEM_EXECUTABLE_NAME} BUNDLE DESTINATION . COMPONENT Runtime)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Resources/ DESTINATION ./${PROJECT_NAME}.app/Contents/Resources COMPONENT Runtime)
if(ALSOFT_LIBRARY_RELEASE)
  get_filename_component(ALSOFT_LIBRARY_DIR ${ALSOFT_LIBRARY_RELEASE} DIRECTORY)
  list(APPEND LIBRARY_PATHS ${ALSOFT_LIBRARY_DIR})
endif()
if(TARGET plugin_ffmpeg)
  get_property(ffmpeg_dependencies TARGET plugin_ffmpeg PROPERTY LINK_LIBRARIES)
  foreach(_item ${ffmpeg_dependencies})
    if(NOT TARGET ${_item})
      get_filename_component(_directory ${_item} DIRECTORY)
      list(APPEND LIBRARY_PATHS ${_directory})
    endif()
  endforeach()
endif()
list(REMOVE_DUPLICATES LIBRARY_PATHS)
install(CODE "
  include(BundleUtilities)
  function(gp_item_default_embedded_path_override item path)
    set(\${path} \"@executable_path/../Frameworks\" PARENT_SCOPE)
  endfunction()
  # construct app
  set(app_path \"${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app\")
  file(REMOVE_RECURSE \"\${CMAKE_CURRENT_BINARY_DIR}/Contents/PlugIns\")
  file(GLOB_RECURSE INSTALLED_PLUGINS \"\${app_path}/Contents/PlugIns/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
  fixup_bundle(\"\${app_path}\" \"\${INSTALLED_PLUGINS}\" \"${LIBRARY_PATHS}\")
  # generate debug info
  set(NANOEM_ENABLE_SENTRY_SDK ${NANOEM_ENABLE_SENTRY_SDK})
  if(NANOEM_ENABLE_SENTRY_SDK AND NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/sentry.touch)
    message(STATUS \"Generating \${app_path}/Contents/MacOS/${PROJECT_NAME} to ${PROJECT_NAME}.app.dSYM\")
    execute_process(COMMAND dsymutil \${app_path}/Contents/MacOS/${PROJECT_NAME} -o ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.app.dSYM)
    foreach(item \${INSTALLED_PLUGINS})
      message(STATUS \"Generating \${item} to ${PROJECT_NAME}.app.dSYM\")
      execute_process(COMMAND dsymutil \${item} -o ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.app.dSYM)
    endforeach()
    # upload dsym
    set(NANOEM_ENABLE_UPLOAD_DSYM ${NANOEM_ENABLE_UPLOAD_DSYM})
    if(NANOEM_ENABLE_UPLOAD_DSYM)
      execute_process(COMMAND sentry-cli
                              --auth-token ${NANOEM_SENTRY_AUTH_TOKEN}
                              upload-dif
                              -o ${NANOEM_SENTRY_ORGANIZATION}
                              -p ${NANOEM_SENTRY_PROJECT}
                              \"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.app.dSYM\")
    endif()
    file(TOUCH ${CMAKE_CURRENT_BINARY_DIR}/sentry.touch)
  endif()
  # strip
  if(CMAKE_INSTALL_DO_STRIP)
    execute_process(COMMAND xcrun strip \${app_path}/Contents/MacOS/nanoem)
    execute_process(COMMAND xcrun strip \${app_path}/Contents/PlugIns/crashpad_handler)
    foreach(item \${INSTALLED_PLUGINS})
      execute_process(COMMAND xcrun strip \${item})
    endforeach()
  endif()
" COMPONENT Runtime)

set(CPACK_GENERATOR "DragNDrop")
set(CPACK_STRIP_FILES ON)
set(CPACK_PACKAGE_VENDOR "hkrn")
set(CPACK_PACKAGE_VERSION "${NANOEM_BUNDLE_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME} - MikuMikuDance compatible software works on multiple platforms")
set(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};${NANOEM_EXECUTABLE_NAME};ALL;/")

include(CPack)
