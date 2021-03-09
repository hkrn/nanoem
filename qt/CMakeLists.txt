set(NANOEM_EXECUTABLE_NAME ${PROJECT_NAME}_qt)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} QT_SOURCES)
add_executable(${NANOEM_EXECUTABLE_NAME} MACOSX_BUNDLE ${QT_SOURCES})
set_property(TARGET ${NANOEM_EXECUTABLE_NAME} APPEND PROPERTY COMPILE_DEFINITIONS QT_NO_KEYWORDS)
set_target_properties(${NANOEM_EXECUTABLE_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME} AUTOMOC ON)
target_link_libraries(${NANOEM_EXECUTABLE_NAME} Qt5::Core Qt5::Gui Qt5::Widgets)

include(${PROJECT_SOURCE_DIR}/cmake/GetGitRevisionDescription.cmake)
git_describe(NANOEM_BUNDLE_VERSION)
string(REGEX MATCH "^v([0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+)" VERSION_MATCH ${NANOEM_BUNDLE_VERSION})
set_target_properties(${NANOEM_EXECUTABLE_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME}
                      MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME}"
                      MACOSX_BUNDLE_ICON_FILE "${PROJECT_NAME}.icns"
                      MACOSX_BUNDLE_GUI_IDENTIFIER "com.github.nanoem"
                      MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_NAME} ${NANOEM_BUNDLE_VERSION}"
                      MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}"
                      MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_MATCH_1}"
                      MACOSX_BUNDLE_COPYRIGHT "Copyright 2015-2020 hkrn All Rights Reserved."
                      MACOSX_BUNDLE_BUNDLE_VERSION "${NANOEM_BUNDLE_VERSION}"
                      MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/macos/Info.plist.in")
nanoem_emapp_link_executable(${NANOEM_EXECUTABLE_NAME})

# plugins
set(_plugins "sokol_glcore33;sokol_metal_macos;plugin_avfoundation;plugin_effect;plugin_ffmpeg;plugin_lsmash;plugin_model;plugin_motion")
foreach(_item ${_plugins})
  if(TARGET ${_item})
    if(APPLE)
      add_custom_command(TARGET ${NANOEM_EXECUTABLE_NAME} POST_BUILD
                         COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>/../PlugIns
                         COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${_item}> $<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>/../PlugIns/$<TARGET_FILE_NAME:${_item}>)
    else()
      add_custom_command(TARGET ${NANOEM_EXECUTABLE_NAME} POST_BUILD
                         COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>/plugins
                         COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${_item}> $<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>/plugins/$<TARGET_FILE_NAME:${_item}>)
    endif()
  endif()
endforeach()

set(CPACK_GENERATOR "DragNDrop")
set(CPACK_STRIP_FILES ON)
set(CPACK_PACKAGE_VENDOR "hkrn")
set(CPACK_PACKAGE_VERSION "${NANOEM_BUNDLE_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME} - MikuMikuDance compatible software works on multiple platforms")
set(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};${NANOEM_EXECUTABLE_NAME};ALL;/")

include(CPack)
