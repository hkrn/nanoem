project(undo)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_MACOSX_RPATH 1)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/nanoem.cmake)
option(${PROJECT_NAME_PREFIX}_ENABLE_TEST_RUNNER "Enable building test runner option." ON)

add_library(${PROJECT_NAME} ${${PROJECT_NAME_PREFIX}_LIBRARY_TYPE} ${CMAKE_CURRENT_SOURCE_DIR}/undo.c)
set_target_properties(${PROJECT_NAME} PROPERTIES DEFINE_SYMBOL "${PROJECT_NAME_PREFIX}_DLL_EXPORTS")

if(${PROJECT_NAME_PREFIX}_ENABLE_TEST_RUNNER)
  add_executable(${PROJECT_NAME}_test ${CMAKE_CURRENT_SOURCE_DIR}/test.c)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps)
  target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME})
endif()

