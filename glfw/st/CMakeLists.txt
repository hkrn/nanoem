# configure
set(NANOEM_EXECUTABLE_NAME ${PROJECT_NAME}_glfw_st)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src GLFW_SOURCES)
list(APPEND GLFW_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cc)
list(APPEND GLFW_SOURCES ${PROJECT_SOURCE_DIR}/dependencies/whereami/src/whereami.c)
add_executable(${NANOEM_EXECUTABLE_NAME} ${GLFW_SOURCES} ${COMMON_SOURCES})
set_target_properties(${NANOEM_EXECUTABLE_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${NANOEM_EXECUTABLE_NAME})
target_link_libraries(${NANOEM_EXECUTABLE_NAME} optimized ${GLFW_LIBRARY_RELEASE} debug ${GLFW_LIBRARY_DEBUG})
nanoem_emapp_link_executable(${NANOEM_EXECUTABLE_NAME})
target_include_directories(${NANOEM_EXECUTABLE_NAME} PRIVATE ${GLFW_INCLUDE_DIR} ${COMMON_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/dependencies/whereami/src)
glfw_link_sentry_sdk(${NANOEM_EXECUTABLE_NAME})
if(SOUNDIO_INCLUDE_DIR AND SOUNDIO_LIBRARY_DEBUG AND SOUNDIO_LIBRARY_RELEASE)
  target_compile_definitions(${NANOEM_EXECUTABLE_NAME} PRIVATE NANOEM_HAS_SOUNDIO)
  target_include_directories(${NANOEM_EXECUTABLE_NAME} PRIVATE ${SOUNDIO_INCLUDE_DIR})
  target_link_libraries(${NANOEM_EXECUTABLE_NAME} optimized ${SOUNDIO_LIBRARY_DEBUG} debug ${SOUNDIO_LIBRARY_RELEASE})
  if(APPLE)
    target_link_libraries(${NANOEM_EXECUTABLE_NAME} "-framework AudioUnit -framework CoreAudio")
  else()
    find_package(ALSA)
    if(ALSA_FOUND)
      target_link_libraries(${NANOEM_EXECUTABLE_NAME} ${ALSA_LIBRARIES})
    endif()
  endif()
endif()

# plugins
get_property(_plugins GLOBAL PROPERTY NANOEM_PROPERTY_INSTALL_PLUGINS)
foreach(_item ${_plugins})
  set(touch_file "${CMAKE_CURRENT_BINARY_DIR}/${_item}.touch")
  add_custom_command(OUTPUT ${touch_file}
                     COMMAND ${CMAKE_COMMAND} -E touch ${touch_file}
                     COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_SOURCE_DIR}/main.cc
                     DEPENDS ${_item}
                     VERBATIM)
  add_custom_target(${_item}_touch_glfw_st DEPENDS ${touch_file})
  set_property(TARGET ${_item}_touch_glfw_st PROPERTY FOLDER plugins/touch)
  add_dependencies(${NANOEM_EXECUTABLE_NAME} ${_item}_touch_glfw_st)
  add_custom_command(TARGET ${NANOEM_EXECUTABLE_NAME} POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>/plugins"
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:${_item}>" "$<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>/plugins/$<TARGET_FILE_NAME:${_item}>"
                     VERBATIM)
endforeach()

if(TARGET plugin_ffmpeg)
  get_property(ffmpeg_dependencies TARGET plugin_ffmpeg PROPERTY LINK_LIBRARIES)
  foreach(_item ${ffmpeg_dependencies})
    get_filename_component(_filename ${_item} NAME)
    if(NOT TARGET ${_item})
      get_filename_component(_directory ${_item} DIRECTORY)
      add_custom_command(TARGET ${NANOEM_EXECUTABLE_NAME} POST_BUILD DEPENDS plugin_ffmpeg
                         COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different "${_item}" "$<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>/plugins/${_filename}"
                         VERBATIM)
      list(APPEND LIBRARY_PATHS ${_directory})
      install(FILES "${_item}" DESTINATION plugins)
    endif()
  endforeach()
endif()

if(LIBRARY_PATHS)
  list(REMOVE_DUPLICATES LIBRARY_PATHS)
endif()
install(TARGETS ${NANOEM_EXECUTABLE_NAME} DESTINATION .)
install(CODE "
  include(BundleUtilities)
  # construct app
  set(app_path \"${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}\")
  file(GLOB_RECURSE INSTALLED_PLUGINS \"\${CMAKE_INSTALL_PREFIX}/plugins/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
  fixup_bundle(\"\${app_path}\" \"\${INSTALLED_PLUGINS}\" \"${LIBRARY_PATHS}\")
  if(CMAKE_INSTALL_DO_STRIP)
    execute_process(COMMAND strip \${app_path})
    foreach(item \${INSTALLED_PLUGINS})
      execute_process(COMMAND strip \${item})
    endforeach()
  endif()
" COMPONENT Runtime)

# cpack
include(${PROJECT_SOURCE_DIR}/cmake/GetGitRevisionDescription.cmake)
git_describe(${NANOEM_EXECUTABLE_NAME}_VERSION)
set(CPACK_GENERATOR "ZIP")
set(CPACK_STRIP_FILES ON)
set(CPACK_PACKAGE_VENDOR "hkrn")
set(CPACK_PACKAGE_VERSION "${${NANOEM_EXECUTABLE_NAME}_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME} - MikuMikuDance compatible software works on multiple platforms")
set(CPACK_INSTALL_CMAKE_PROJECTS "${PROJECT_BINARY_DIR};${NANOEM_EXECUTABLE_NAME};ALL;/")
set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/LICENSE.md)
set(CPACK_NSIS_DISPLAY_NAME ${PROJECT_NAME})
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_NSIS_HELP_LINK "https://nanoem.readthedocs.io")
set(CPACK_NSIS_PACKAGE_NAME ${PROJECT_NAME})
include(CPack)
