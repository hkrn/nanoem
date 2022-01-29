project(fx9)
cmake_minimum_required(VERSION 3.5)

set(CMAKE_C_STANDARD 99)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(WIN32 AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(fx9_cxx_standard 14)
else()
  set(fx9_cxx_standard 11)
endif()

if(MSVC)
  if(BUILD_SHARED_LIBS)
    set(building_library_flag "/MD")
  else()
    set(building_library_flag "/MT")
  endif()
  foreach(type debug)
    string(TOUPPER ${type} type_toupper)
    set(CMAKE_C_FLAGS_${type_toupper} "${CMAKE_C_FLAGS_${type_toupper}} ${building_library_flag}d")
    set(CMAKE_CXX_FLAGS_${type_toupper} "${CMAKE_CXX_FLAGS_${type_toupper}} ${building_library_flag}d /EHsc")
   endforeach()
  foreach(type release relwithdebinfo minsizerel)
    string(TOUPPER ${type} type_toupper)
    set(CMAKE_C_FLAGS_${type_toupper} "${CMAKE_C_FLAGS_${type_toupper}} ${building_library_flag}")
    set(CMAKE_CXX_FLAGS_${type_toupper} "${CMAKE_CXX_FLAGS_${type_toupper}} ${building_library_flag} /EHsc")
  endforeach()
endif()

# fx9
get_filename_component(FX9_DEPENDENCIES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/.. ABSOLUTE)
get_filename_component(FX9_PROTOBUF_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../emapp/src/protoc ABSOLUTE)
set(FX9_PATH ${CMAKE_CURRENT_SOURCE_DIR})
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm64" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64")
  set(FX9_TARGET_ARCHITECTURE "arm64" CACHE STRING "Target architecture to build fx9")
else()
  set(FX9_TARGET_ARCHITECTURE "x86_64" CACHE STRING "Target architecture to build fx9")
endif()
set_property(CACHE FX9_TARGET_ARCHITECTURE PROPERTY STRINGS "x86_64;i386;arm64;arm32")
if(WIN32)
  set(FX9_TARGET_COMPILER "vs2019" CACHE STRING "Target architecture to build fx9")
  set_property(CACHE FX9_TARGET_COMPILER PROPERTY STRINGS "vs2019;vs2017;vs2015;vs2013")
else()
  if(APPLE)
    set(FX9_TARGET_COMPILER "clang" CACHE STRING "Target architecture to build fx9")
  else()
    set(FX9_TARGET_COMPILER "gcc" CACHE STRING "Target architecture to build fx9")
  endif()
  set_property(CACHE FX9_TARGET_COMPILER PROPERTY STRINGS "clang;gcc")
endif()
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/common.cmake)

# protobuf-c
add_library(protobuf-c STATIC ${FX9_DEPENDENCIES_PATH}/protobuf-c/protobuf-c/protobuf-c.c)
set_property(TARGET protobuf-c PROPERTY FOLDER dependencies)

# fx9_runner
fx9_cmake_get_install_path("glfw" GLFW_BASE_PATH GLFW_INSTALL_PATH_DEBUG GLFW_INSTALL_PATH_RELEASE)
find_library(GLFW_LIBRARY_DEBUG NAMES glfw3 glfw PATH_SUFFIXES lib PATHS ${GLFW_INSTALL_PATH_DEBUG} NO_DEFAULT_PATH)
find_library(GLFW_LIBRARY_RELEASE NAMES glfw3 glfw PATH_SUFFIXES lib PATHS ${GLFW_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH)
find_path(GLFW_INCLUDE_DIR NAMES GLFW/glfw3.h PATH_SUFFIXES include PATHS ${GLFW_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH)
mark_as_advanced(GLFW_LIBRARY_DEBUG GLFW_LIBRARY_RELEASE GLFW_INCLUDE_DIR)

#mtlpp
if(APPLE)
  add_library(mtlpp ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/mtlpp/mtlpp.mm)
  set_property(TARGET mtlpp PROPERTY CXX_STANDARD ${fx9_cxx_standard})
endif()

if(GLFW_LIBRARY_RELEASE AND GLFW_INCLUDE_DIR)
  add_executable(fx9_runner ${CMAKE_CURRENT_SOURCE_DIR}/main.cc)
  set_property(TARGET fx9_runner PROPERTY CXX_STANDARD ${fx9_cxx_standard})
  set_property(TARGET fx9_runner APPEND PROPERTY COMPILE_DEFINITIONS $<$<BOOL:${WIN32}>:_CRT_SECURE_NO_WARNINGS=1>)
  set_property(TARGET fx9_runner APPEND PROPERTY
               INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
               ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
               ${FX9_DEPENDENCIES_PATH}/glslang
               ${FX9_DEPENDENCIES_PATH}/protobuf-c
               ${FX9_PROTOBUF_PATH}
               ${GLFW_INCLUDE_DIR})
  set_property(TARGET fx9_runner PROPERTY VS_STARTUP_PROJECT fx9_runner)
  target_link_libraries(fx9_runner protobuf-c fx9 fx9_protobuf)
  target_link_libraries(fx9_runner optimized ${GLFW_LIBRARY_RELEASE} debug ${GLFW_LIBRARY_DEBUG})
  if(APPLE)
    target_link_libraries(fx9_runner "-framework AppKit -framework CoreVideo -framework IOKit -framework OpenGL -framework Metal")
    target_link_libraries(fx9_runner mtlpp)
  elseif(UNIX)
    find_package(X11)
    target_link_libraries(fx9_runner ${X11_LIBRARIES} -ldl -lpthread ${X11_Xinerama_LIB} ${X11_Xcursor_LIB} ${X11_Xxf86vm_LIB} ${X11_Xrandr_LIB})
  endif()
  if(FX9_ENABLE_ASAN)
    set_target_properties(fx9_runner PROPERTIES COMPILE_FLAGS "-fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls" LINK_FLAGS "-fsanitize=address")
  elseif(FX9_ENABLE_UBSAN)
    set_target_properties(fx9_runner PROPERTIES COMPILE_FLAGS "-fsanitize=undefined -fno-omit-frame-pointer" LINK_FLAGS "-fsanitize=undefined")
  endif()
endif()

function(create_sokol_program name def)
  add_library("${name}_core" STATIC ${CMAKE_CURRENT_SOURCE_DIR}/sokol_impl.$<IF:$<BOOL:${APPLE}>,m,c>)
  set_property(TARGET "${name}_core" APPEND PROPERTY COMPILE_DEFINITIONS SOKOL_IMPL ${def}
               $<$<BOOL:${WIN32}>:_CRT_SECURE_NO_WARNINGS=1>)
  set_property(TARGET "${name}_core" APPEND PROPERTY INCLUDE_DIRECTORIES
               ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
               ${FX9_DEPENDENCIES_PATH}/sokol)
  if(WIN32)
    add_executable(${name} WIN32 ${CMAKE_CURRENT_SOURCE_DIR}/sokol.cc)
  else()
    add_executable(${name} ${CMAKE_CURRENT_SOURCE_DIR}/sokol.cc)
  endif()
  set_property(TARGET ${name} PROPERTY CXX_STANDARD ${fx9_cxx_standard})
  set_property(TARGET ${name} APPEND PROPERTY COMPILE_DEFINITIONS ${def})
  set_property(TARGET ${name} APPEND PROPERTY
               INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
               ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
               ${FX9_DEPENDENCIES_PATH}/glslang
               ${FX9_DEPENDENCIES_PATH}/protobuf-c
               ${FX9_DEPENDENCIES_PATH}/sokol
               ${FX9_PROTOBUF_PATH})
  target_link_libraries(${name} protobuf-c fx9 fx9_protobuf "${name}_core")
  if(WIN32)
    target_link_libraries(${name} opengl32)
  elseif(APPLE)
    target_link_libraries("${name}_core" "-framework AppKit -framework Quartz -framework CoreVideo -framework IOKit -framework Metal -framework MetalKit")
    set_property(TARGET "${name}_core" APPEND PROPERTY COMPILE_FLAGS "-fobjc-arc")
  elseif(UNIX)
    find_package(X11)
    target_link_libraries(${name} ${X11_LIBRARIES} -ldl -lpthread ${X11_Xinerama_LIB} ${X11_Xcursor_LIB} ${X11_Xxf86vm_LIB} ${X11_Xrandr_LIB})
  endif()
  if(FX9_ENABLE_ASAN)
    set_target_properties(${name} PROPERTIES COMPILE_FLAGS "-fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls" LINK_FLAGS "-fsanitize=address")
  elseif(FX9_ENABLE_UBSAN)
    set_target_properties(${name} PROPERTIES COMPILE_FLAGS "-fsanitize=undefined -fno-omit-frame-pointer" LINK_FLAGS "-fsanitize=undefined")
  endif()
endfunction()

if(APPLE)
  create_sokol_program(fx9_sokol_metal SOKOL_METAL)
  target_link_libraries(fx9_sokol_metal mtlpp)
else()
  create_sokol_program(fx9_sokol_gl SOKOL_GLCORE33)
  if(WIN32)
    create_sokol_program(fx9_sokol_d3d11 SOKOL_D3D11)
  endif()
endif()

# fx9_test
if(FALSE)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/test FX9_TEST_SOURCES)
add_executable(fx9_test ${FX9_TEST_SOURCES})
set_property(TARGET fx9_test APPEND PROPERTY CXX_STANDARD ${fx9_cxx_standard})
set_property(TARGET fx9_test APPEND PROPERTY
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
    ${FX9_DEPENDENCIES_PATH}/glslang
    ${FX9_DEPENDENCIES_PATH}/protobuf-c
    ${FX9_PROTOBUF_PATH})
set_property(TARGET fx9_test APPEND PROPERTY
    COMPILE_DEFINITIONS FX9_TEST_EFFECT_FIXTURES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/test/effects")
if(UNIX)
  target_link_libraries(fx9_test -ldl -lpthread)
endif()
endif()
