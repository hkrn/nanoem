# configure
set(NANOEM_EXECUTABLE_NAME ${PROJECT_NAME}_sapp)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SAPP_SOURCES)
list(APPEND SAPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cc)
list(APPEND SAPP_SOURCES ${PROJECT_SOURCE_DIR}/dependencies/whereami/src/whereami.c)
if(APPLE)
  add_executable(${NANOEM_EXECUTABLE_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/sapp.m ${SAPP_SOURCES})
else()
  add_executable(${NANOEM_EXECUTABLE_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/sapp.c ${SAPP_SOURCES})
endif()
set_target_properties(${NANOEM_EXECUTABLE_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${NANOEM_EXECUTABLE_NAME})
nanoem_emapp_link_executable(${NANOEM_EXECUTABLE_NAME})
target_include_directories(${NANOEM_EXECUTABLE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
                           ${PROJECT_SOURCE_DIR}/dependencies/whereami/src)
if(WIN32)
  target_compile_definitions(${NANOEM_EXECUTABLE_NAME} PRIVATE SOKOL_D3D11)
  #target_compile_definitions(${NANOEM_EXECUTABLE_NAME} PRIVATE SOKOL_GLCORE33 SOKOL_EXTERNAL_GL_LOADER)
  if(TARGET sokol_glcore33)
    target_compile_definitions(${NANOEM_EXECUTABLE_NAME} PRIVATE NANOEM_WIN32_HAS_OPENGL)
  endif()
elseif(APPLE)
  target_compile_definitions(${NANOEM_EXECUTABLE_NAME} PRIVATE SOKOL_METAL)
  target_link_libraries(${NANOEM_EXECUTABLE_NAME} "-framework AudioToolbox -weak_framework MetalKit")
elseif(EMSCRIPTEN)
  target_compile_definitions(${NANOEM_EXECUTABLE_NAME} PRIVATE SOKOL_GLES3)
  target_link_options(${NANOEM_EXECUTABLE_NAME} PRIVATE "-s ALLOW_MEMORY_GROWTH=1 -s STRICT_JS=1 -s USE_WEBGL2=1")
  set_target_properties(${NANOEM_EXECUTABLE_NAME} PROPERTIES SUFFIX .html)
else()
  find_path(ASOUND_INCLUDE_DIR alsa/asoundlib.h)
  find_library(ASOUND_LIBRARY asound)
  mark_as_advanced(ASOUND_INCLUDE_DIR ASOUND_LIBRARY)
  target_compile_definitions(${NANOEM_EXECUTABLE_NAME} PRIVATE SOKOL_GLCORE33)
  target_include_directories(${NANOEM_EXECUTABLE_NAME} PRIVATE ${ASOUND_INCLUDE_DIR})
  target_link_libraries(${NANOEM_EXECUTABLE_NAME} ${ASOUND_LIBRARY})
endif()

# plugins
get_property(_plugins GLOBAL PROPERTY NANOEM_PROPERTY_INSTALL_PLUGINS)
if(NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN)
  target_link_libraries(${NANOEM_EXECUTABLE_NAME} ${_plugins})
else()
  foreach(_item ${_plugins})
    set(touch_file "${CMAKE_CURRENT_BINARY_DIR}/intermediate/${_item}.touch")
    add_custom_command(OUTPUT ${touch_file}
                       COMMAND ${CMAKE_COMMAND} -E touch ${touch_file}
                       COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_SOURCE_DIR}/main.cc
                       DEPENDS ${_item}
                       VERBATIM)
    add_custom_target(${_item}_touch_sapp DEPENDS ${touch_file})
    set_property(TARGET ${_item}_touch_sapp PROPERTY FOLDER plugins/touch)
    add_dependencies(${NANOEM_EXECUTABLE_NAME} ${_item}_touch_sapp)
    add_custom_command(TARGET ${NANOEM_EXECUTABLE_NAME} POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>/plugins"
                       COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:${_item}>" "$<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>/plugins/$<TARGET_FILE_NAME:${_item}>"
                       VERBATIM)
  endforeach()
endif()

if(WIN32 AND TARGET plugin_ffmpeg)
  set(_libraries "AVCODEC_LIBRARY_DLL;AVFORMAT_LIBRARY_DLL;AVUTIL_LIBRARY_DLL;SWRESAMPLE_LIBRARY_DLL;SWSCALE_LIBRARY_DLL")
  foreach(_item ${_libraries})
    get_filename_component(_filename ${${_item}} NAME)
    add_custom_command(TARGET ${NANOEM_EXECUTABLE_NAME} POST_BUILD DEPENDS plugin_ffmpeg
                       COMMAND ${CMAKE_COMMAND} -E copy "${${_item}}" "$<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>/plugins/${_filename}"
                       VERBATIM)
  endforeach()
endif()

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
