macro(nanoem_emapp_plugin_set_library_type)
  if(NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN)
    set(_library_type STATIC)
  else()
    set(_library_type SHARED)
  endif()
endmacro()

function(nanoem_emapp_plugin_install target)
  if(NOT NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN)
    if(APPLE)
      set(PLUGIN_INSTALL_DIR ${PROJECT_NAME}.app/Contents/PlugIns)
    else()
      set(PLUGIN_INSTALL_DIR plugins)
    endif()
    if(WIN32)
      target_compile_definitions(${target} PRIVATE NANOEM_BUILDING_DLL NANOEM_DLL_EXPORTS)
      if(MINGW)
        set_property(TARGET ${target} APPEND PROPERTY LINK_FLAGS -Wl,--kill-at)
      endif()
      install(TARGETS ${target} RUNTIME DESTINATION ${PLUGIN_INSTALL_DIR})
    else()
      install(TARGETS ${target} LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR})
    endif()
  endif()
  set_property(GLOBAL APPEND PROPERTY NANOEM_PROPERTY_INSTALL_PLUGINS ${target})
  message(STATUS "[emapp] setup \"${target}\" as installing plugin to ${PLUGIN_INSTALL_DIR}")
endfunction()

function(nanoem_emapp_link_executable item)
  target_compile_definitions(${item} PRIVATE BX_CONFIG_ALLOCATOR_CRT=0
                             BX_CONFIG_ALLOCATOR_DEBUG=$<BOOL:${NANOEM_ENABLE_DEBUG_ALLOCATOR}>)
  set_property(TARGET ${item} emapp APPEND PROPERTY COMPILE_DEFINITIONS
               $<$<BOOL:${WIN32}>:_CRT_SECURE_NO_WARNINGS=1>
               $<$<BOOL:${NANOEM_ENABLE_DEBUG_LABEL}>:NANOEM_ENABLE_DEBUG_LABEL=1>
               $<$<BOOL:${SOKOL_ENABLE_DEBUG}>:SOKOL_DEBUG=1>
               __STDC_LIMIT_MACROS
               __STDC_CONSTANT_MACROS
               __STDC_FORMAT_MACROS)
  target_include_directories(${item} PRIVATE
                             ${PROJECT_SOURCE_DIR}/dependencies
                             ${PROJECT_SOURCE_DIR}/dependencies/protobuf-c
                             ${PROJECT_SOURCE_DIR}/emapp/include
                             ${PROJECT_SOURCE_DIR}/emapp/resources
                             ${BX_COMPAT_INCLUDE_PATH}
                             ${BX_INCLUDE_DIR}
                             ${BX_THIRDPARTY_INCLUDE_DIR}
                             ${BIMG_INCLUDE_DIR}
                             ${GLM_INCLUDE_DIR})
  get_property(_compile_definitions_nanoem TARGET nanoem PROPERTY COMPILE_DEFINITIONS)
  set_property(TARGET ${item} emapp APPEND PROPERTY COMPILE_DEFINITIONS ${_compile_definitions_nanoem})
  get_property(_include_directories_nanoem TARGET nanoem PROPERTY INCLUDE_DIRECTORIES)
  set_property(TARGET ${item} emapp APPEND PROPERTY INCLUDE_DIRECTORIES ${_include_directories_nanoem})
  get_property(_compile_definitions_embundle TARGET embundle PROPERTY COMPILE_DEFINITIONS)
  set_property(TARGET ${item} emapp APPEND PROPERTY COMPILE_DEFINITIONS ${_compile_definitions_embundle})
  get_property(_link_libraries TARGET nanoem PROPERTY LINK_LIBRARIES)
  target_link_libraries(${item} emapp embundle emarb lz4 bx nanoem ${_link_libraries})
  target_link_libraries(${item} optimized ${MINIZIP_LIBRARY_RELEASE}
                                optimized ${ZLIB_LIBRARY_RELEASE}
                                debug ${MINIZIP_LIBRARY_DEBUG}
                                debug ${ZLIB_LIBRARY_DEBUG})
  if(UNIX AND NOT APPLE)
    # for minizip
    find_package(Iconv)
    target_link_libraries(${item} ${Iconv_LIBRARIES})
  endif()
  if(NANOEM_ENABLE_TBB)
    target_link_libraries(${item} optimized ${TBB_LIBRARY_RELEASE}
                                  debug ${TBB_LIBRARY_DEBUG})
    target_include_directories(${item} PRIVATE ${TBB_INCLUDE_DIR})
    target_compile_definitions(${item} PRIVATE NANOEM_ENABLE_TBB __TBB_NO_IMPLICIT_LINKAGE=1)
  endif()
  if(NANOEM_ENABLE_OPTICK)
    find_library(OPTICK_LIBRARY_PATH OptickCore PATHS ${PROJECT_SOURCE_DIR}/dependencies/optick/build NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
    target_link_libraries(${item} ${OPTICK_LIBRARY_PATH})
    target_compile_definitions(${item} PRIVATE NANOEM_ENABLE_OPTICK)
  endif()
  if(NANOEM_ENABLE_NANOMSG)
    target_link_libraries(${item} optimized ${NANOMSG_LIBRARY_RELEASE}
                                  debug ${NANOMSG_LIBRARY_DEBUG})
  endif()
  if(WIN32)
    set_target_properties(${item} PROPERTIES WIN32_EXECUTABLE ON)
    if(TARGET sokol_glcore33)
      target_link_libraries(${item} opengl32)
    endif()
    if(NANOEM_ENABLE_ASAN)
      target_link_libraries(${item} clang_rt.asan-${NANOEM_TARGET_ARCHITECTURE} clang_rt.asan_cxx-${NANOEM_TARGET_ARCHITECTURE})
    endif()
  elseif(APPLE)
    target_link_libraries(${item} "-framework $<IF:$<BOOL:${IOS}>,UIKit,AppKit> -framework Security -framework IOKit -framework $<IF:$<BOOL:${IOS}>,OpenGLES,OpenGL>")
    find_program(SW_VERS_PATH sw_vers)
    mark_as_advanced(SW_VERS_PATH)
    exec_program(${SW_VERS_PATH} ARGS -productVersion OUTPUT_VARIABLE MACOSX_VERSION_STRING)
    string(REGEX REPLACE "(10\\.[0-9]+)(\\.[0-9]+)?" "\\1" MACOSX_VERSION "${MACOSX_VERSION_STRING}")
    if(MACOSX_VERSION VERSION_GREATER "10.10")
      target_link_libraries(${item} "-framework QuartzCore -weak_framework IOSurface -weak_framework Metal")
    endif()
  elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
    target_link_libraries(${item} dl)
    find_package(X11)
    find_package(OpenGL)
    if(X11_FOUND)
      target_link_libraries(${item} ${X11_LIBRARIES} ${X11_Xi_LIB} ${X11_Xinerama_LIB} ${X11_Xcursor_LIB} ${X11_Xxf86vm_LIB} ${X11_Xrandr_LIB})
    endif()
    if(OPENGL_FOUND)
      target_link_libraries(${item} ${OPENGL_gl_LIBRARY})
    endif()
  endif()
  if(MSVC)
    set_property(TARGET ${item} APPEND PROPERTY LINK_FLAGS "/NODEFAULTLIB:vcruntime")
  endif()
  nanoem_cmake_enable_lto(${item})
  message(STATUS "[emapp] setup linkage of \"${item}\"")
endfunction()

function(nanoem_emapp_build_emapp suffix)
  set(_emapp_name "emapp${suffix}")
  set(_emapp_protoc_name "emapp_protoc${suffix}")
  set(_emarb_name "emarb${suffix}")
  set(_embundle_name "embundle${suffix}")
  aux_source_directory(${PROJECT_SOURCE_DIR}/emapp/src/protoc EMAPP_PROTOC_SOURCES)
  aux_source_directory(${PROJECT_SOURCE_DIR}/emapp/src EMAPP_CORE_SOURCES)
  aux_source_directory(${PROJECT_SOURCE_DIR}/emapp/src/command EMAPP_NS_COMMAND_SOURCES)
  aux_source_directory(${PROJECT_SOURCE_DIR}/emapp/src/effect EMAPP_NS_EFFECT_SOURCES)
  aux_source_directory(${PROJECT_SOURCE_DIR}/emapp/src/model EMAPP_NS_MODEL_SOURCES)
  aux_source_directory(${PROJECT_SOURCE_DIR}/emapp/src/internal EMAPP_NS_INTERNAL_SOURCES)
  aux_source_directory(${PROJECT_SOURCE_DIR}/emapp/src/internal/imgui EMAPP_NS_INTERNAL_IMGUI_SOURCES)
  aux_source_directory(${PROJECT_SOURCE_DIR}/emapp/src/internal/project EMAPP_NS_INTERNAL_PROJECT_SOURCES)
  aux_source_directory(${PROJECT_SOURCE_DIR}/emapp/src/plugin EMAPP_NS_PLUGIN_SOURCES)
  aux_source_directory(${PROJECT_SOURCE_DIR}/emapp/include EMAPP_CORE_HEADERS)
  add_library(${_emapp_protoc_name} STATIC ${EMAPP_PROTOC_SOURCES})
  nanoem_cmake_enable_lto(${_emapp_protoc_name})
  set_property(TARGET ${_emapp_protoc_name} PROPERTY FOLDER nanoem/resources)
  target_include_directories(${_emapp_protoc_name} PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/protobuf-c)
  add_library(${_emapp_name} STATIC ${EMAPP_CORE_SOURCES} ${EMAPP_NS_COMMAND_SOURCES} ${EMAPP_NS_EFFECT_SOURCES}
                                    ${EMAPP_NS_INTERNAL_SOURCES} ${EMAPP_NS_INTERNAL_IMGUI_SOURCES} ${EMAPP_NS_INTERNAL_PROJECT_SOURCES} ${EMAPP_NS_MODEL_SOURCES}
                                    ${EMAPP_NS_PLUGIN_SOURCES} ${EMAPP_CORE_HEADERS})
  nanoem_cmake_enable_lto(${_emapp_name})
  add_dependencies(${_emapp_name} emarb)
  target_link_libraries(${_emapp_name} ${_emapp_protoc_name})
  if(WIN32)
    target_link_libraries(${_emapp_name} ws2_32 mswsock advapi32 crypt32)
  endif()
  set_property(TARGET ${_emapp_name} PROPERTY FOLDER nanoem)
  set_property(TARGET lz4 PROPERTY FOLDER dependencies)
  get_property(_compile_definition_nanoem TARGET nanoem PROPERTY COMPILE_DEFINITIONS)
  get_property(_compile_definition_embundle TARGET ${_embundle_name} PROPERTY COMPILE_DEFINITIONS)
  target_compile_definitions(${_emapp_name} PRIVATE
               BX_CONFIG_ALLOCATOR_CRT=0
               BX_CONFIG_ALLOCATOR_DEBUG=$<BOOL:${NANOEM_ENABLE_DEBUG_ALLOCATOR}>
               $<$<BOOL:${NANOEM_ENABLE_SDEF}>:NANOEM_ENABLE_SDEF>
               $<$<BOOL:${NANOEM_ENABLE_OPTICK}>:NANOEM_ENABLE_OPTICK>
               $<$<BOOL:${NANOEM_ENABLE_SHADER_OPTIMIZED}>:NANOEM_ENABLE_SHADER_OPTIMIZED>
               $<$<BOOL:${NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN}>:NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN>
               $<$<BOOL:${NANOEM_ENABLE_BLENDOP_MINMAX}>:NANOEM_ENABLE_BLENDOP_MINMAX>
               $<$<BOOL:${NANOEM_ENABLE_SOFTBODY}>:NANOEM_ENABLE_SOFTBODY>
               ${_compile_definition_nanoem}
               ${_compile_definition_embundle})
  set_property(TARGET ${_emapp_name} ${_emarb_name} ${_embundle_name} APPEND PROPERTY COMPILE_DEFINITIONS
               IMGUI_DISABLE_OBSOLETE_FUNCTIONS
               IMGUI_DISABLE_INCLUDE_IMCONFIG_H)
  if(NANOEM_ENABLE_IMGUI_INDEX32)
    set_property(TARGET ${_emapp_name} ${_emarb_name} ${_embundle_name} APPEND PROPERTY COMPILE_DEFINITIONS
                 $<$<BOOL:${NANOEM_ENABLE_DEBUG_LABEL}>:NANOEM_ENABLE_DEBUG_LABEL>
                 IMGUI_USER_CONFIG="emapp/private/imgui/config.h")
  endif()
  if(NANOEM_ENABLE_IMGUI_FILE_DIALOG)
    set_property(TARGET ${_emapp_name} ${_embundle_name} APPEND PROPERTY COMPILE_DEFINITIONS NANOEM_ENABLE_IMGUI_FILE_DIALOG)
  endif()
  get_property(_include_directories TARGET nanoem PROPERTY INCLUDE_DIRECTORIES)
  target_include_directories(${_emapp_name} PRIVATE
                             ${BX_COMPAT_INCLUDE_PATH}
                             ${BX_INCLUDE_DIR}
                             ${BIMG_INCLUDE_DIR}
                             ${MINIZIP_INCLUDE_DIR}
                             ${PROJECT_SOURCE_DIR}/dependencies
                             ${PROJECT_SOURCE_DIR}/dependencies/imgui
                             ${PROJECT_SOURCE_DIR}/dependencies/protobuf-c
                             ${PROJECT_SOURCE_DIR}/emapp/bundle
                             ${PROJECT_SOURCE_DIR}/emapp/include
                             ${PROJECT_SOURCE_DIR}/emapp/resources
                             ${PROJECT_SOURCE_DIR}/emapp/plugins/include
                             ${PROJECT_BINARY_DIR}/emapp/resources/include
                             ${GLM_INCLUDE_DIR}
                             ${ZLIB_INCLUDE_DIR}
                             ${_include_directories})
  if(NANOEM_ENABLE_MIMALLOC)
    target_link_libraries(${_emapp_name} optimized ${MIMALLOC_LIBRARY_RELEASE} debug ${MIMALLOC_LIBRARY_DEBUG})
    target_include_directories(${_emapp_name} PRIVATE ${MIMALLOC_INCLUDE_DIR})
    target_compile_definitions(${_emapp_name} PRIVATE NANOEM_ENABLE_MIMALLOC)
  endif()
  if(NANOEM_ENABLE_TBB)
    nanoem_emapp_find_tbb()
    target_compile_definitions(${_emapp_name} PRIVATE NANOEM_ENABLE_TBB __TBB_NO_IMPLICIT_LINKAGE=1)
    target_include_directories(${_emapp_name} PRIVATE ${TBB_INCLUDE_DIR})
    message(STATUS "[emapp] TBB is enabled")
  endif()
  if(NANOEM_ENABLE_NANOMSG)
    target_compile_definitions(${_emapp_name} PRIVATE NANOEM_ENABLE_NANOMSG)
    target_include_directories(${_emapp_name} PRIVATE ${NANOMSG_INCLUDE_DIR})
  endif()
  if(EMSCRIPTEN)
    target_link_libraries(${_emapp_name} sokol_gles3)
  endif()
  # ${PROJECT_NAME} for bullet
  # set_property(TARGET ${PROJECT_NAME} emapp APPEND PROPERTY COMPILE_DEFINITIONS GLM_FORCE_RADIANS)
endfunction()

function(nanoem_emapp_organize_executables executables)
  nanoem_emapp_build_embundle("")
  nanoem_emapp_build_emarb("")
  nanoem_emapp_build_emapp("")
  set(${executables} ${_executables} PARENT_SCOPE)
endfunction()

function(nanoem_emapp_find_bx)
  add_library(bx STATIC ${PROJECT_SOURCE_DIR}/dependencies/bx/src/amalgamated.cpp)
  nanoem_cmake_enable_lto(bx)
  find_path(BX_INCLUDE_DIR bx/bx.h PATH_SUFFIXES include PATHS ${PROJECT_SOURCE_DIR}/dependencies/bx NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  find_path(BX_THIRDPARTY_INCLUDE_DIR ini/ini.h PATH_SUFFIXES include PATHS ${PROJECT_SOURCE_DIR}/dependencies/bx/3rdparty NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  if(MSVC OR (WIN32 AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
    find_path(BX_COMPAT_INCLUDE_PATH alloca.h PATH_SUFFIXES include/compat/msvc PATHS ${PROJECT_SOURCE_DIR}/dependencies/bx NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  elseif(MINGW)
    find_path(BX_COMPAT_INCLUDE_PATH alloca.h PATH_SUFFIXES include/compat/mingw PATHS ${PROJECT_SOURCE_DIR}/dependencies/bx NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  elseif(APPLE)
    find_path(BX_COMPAT_INCLUDE_PATH malloc.h PATH_SUFFIXES include/compat/osx PATHS ${PROJECT_SOURCE_DIR}/dependencies/bx NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  endif()
  mark_as_advanced(BX_INCLUDE_DIR BX_COMPAT_INCLUDE_PATH BX_THIRDPARTY_INCLUDE_DIR)
  set_property(TARGET bx PROPERTY FOLDER dependencies)
  target_compile_definitions(bx PRIVATE __STDC_LIMIT_MACROS __STDC_CONSTANT_MACROS __STDC_FORMAT_MACROS)
  target_include_directories(bx PRIVATE ${BX_COMPAT_INCLUDE_PATH} ${BX_INCLUDE_DIR} ${BX_THIRDPARTY_INCLUDE_DIR})
  if(WIN32)
    target_link_libraries(bx psapi)
  elseif(APPLE)
    target_link_libraries(bx "-framework Foundation")
  endif()
endfunction()

function(nanoem_emapp_find_minizip)
  nanoem_cmake_get_install_path("minizip" MINIZIP_BASE_PATH MINIZIP_INSTALL_PATH_DEBUG MINIZIP_INSTALL_PATH_RELEASE)
  find_path(MINIZIP_INCLUDE_DIR NAMES mz.h PATH_SUFFIXES include PATHS ${MINIZIP_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  find_library(MINIZIP_LIBRARY_DEBUG NAMES minizip libminizip minizipd libminizipd PATH_SUFFIXES lib PATHS ${MINIZIP_INSTALL_PATH_DEBUG} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  find_library(MINIZIP_LIBRARY_RELEASE NAMES minizip libminizip PATH_SUFFIXES lib PATHS ${MINIZIP_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  mark_as_advanced(MINIZIP_INCLUDE_DIR MINIZIP_LIBRARY_DEBUG MINIZIP_LIBRARY_RELEASE)
  message(STATUS "[emapp] minizip is located at ${MINIZIP_BASE_PATH}")
endfunction()

function(nanoem_emapp_find_nanomsg)
  nanoem_cmake_get_install_path("nanomsg" NANOMSG_BASE_PATH NANOMSG_INSTALL_PATH_DEBUG NANOMSG_INSTALL_PATH_RELEASE)
  if(NANOEM_ENABLE_NANOMSG)
    find_path(NANOMSG_INCLUDE_DIR NAMES nanomsg/nn.h PATH_SUFFIXES include PATHS ${NANOMSG_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
    find_library(NANOMSG_LIBRARY_DEBUG NAMES nanomsg PATH_SUFFIXES lib PATHS ${NANOMSG_INSTALL_PATH_DEBUG} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
    find_library(NANOMSG_LIBRARY_RELEASE NAMES nanomsg PATH_SUFFIXES lib PATHS ${NANOMSG_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
    mark_as_advanced(NANOMSG_INCLUDE_DIR NANOMSG_LIBRARY_DEBUG NANOMSG_LIBRARY_RELEASE)
    message(STATUS "[emapp] nanomsg is located at ${NANOMSG_BASE_PATH}")
  endif()
endfunction()

# mimalloc
function(nanoem_emapp_find_mimalloc)
  set(MIMALLOC_VERSION "1.7")
  nanoem_cmake_get_install_path("mimalloc" MIMALLOC_BASE_PATH MIMALLOC_INSTALL_PATH_DEBUG MIMALLOC_INSTALL_PATH_RELEASE)
  find_path(MIMALLOC_INCLUDE_DIR NAMES mimalloc.h PATH_SUFFIXES lib/mimalloc-${MIMALLOC_VERSION}/include PATHS ${MIMALLOC_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  find_library(MIMALLOC_LIBRARY_DEBUG NAMES libmimalloc-debug.a mimalloc-debug PATH_SUFFIXES lib/mimalloc-${MIMALLOC_VERSION} PATHS ${MIMALLOC_INSTALL_PATH_DEBUG} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  find_library(MIMALLOC_LIBRARY_RELEASE NAMES libmimalloc.a mimalloc PATH_SUFFIXES lib/mimalloc-${MIMALLOC_VERSION} PATHS ${MIMALLOC_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  mark_as_advanced(MIMALLOC_INCLUDE_DIR MIMALLOC_LIBRARY_DEBUG MIMALLOC_LIBRARY_RELEASE)
endfunction()

function(nanoem_emapp_find_tbb)
  nanoem_cmake_get_install_path("tbb" TBB_BASE_PATH TBB_INSTALL_PATH_DEBUG TBB_INSTALL_PATH_RELEASE)
  find_path(TBB_INCLUDE_DIR NAMES tbb/tbb.h PATH_SUFFIXES include PATHS ${TBB_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  find_library(TBB_LIBRARY_DEBUG NAMES tbb_static PATH_SUFFIXES lib PATHS ${TBB_INSTALL_PATH_DEBUG} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  find_library(TBB_LIBRARY_RELEASE NAMES tbb_static PATH_SUFFIXES lib PATHS ${TBB_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  mark_as_advanced(TBB_INCLUDE_DIR TBB_LIBRARY_DEBUG TBB_LIBRARY_RELEASE)
  message(STATUS "[emapp] tbb is located at ${TBB_BASE_PATH}")
endfunction()

# revisions
function(get_git_dependency_revision _location _output)
  if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND NANOEM_ENABLE_UPDATE_CREDITS)
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule status dependencies/${_location} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE revision_full)
    string(REGEX MATCH " *([0-9a-f]+)" _variable_unused ${revision_full})
    set(${_output} ${CMAKE_MATCH_1} PARENT_SCOPE)
  endif()
endfunction()
get_git_dependency_revision(bullet3 BULLET_REVISION)
get_git_dependency_revision(bx BX_REVISION)
get_git_dependency_revision(debug-draw DEBUG_DRAW_REVISION)
get_git_dependency_revision(imgui IMGUI_REVISION)
get_git_dependency_revision(imguizmo IMGUIZMO_REVISION)
get_git_dependency_revision(l-smash LSMASH_REVISION)
get_git_dependency_revision(lz4 LZ4_REVISION)
get_git_dependency_revision(mimalloc MIMALLOC_REVISION)
get_git_dependency_revision(minizip MINIZIP_REVISION)
get_git_dependency_revision(nanomsg NANOMSG_REVISION)
get_git_dependency_revision(par PAR_REVISION)
get_git_dependency_revision(protobuf-c PROTOBUF_C_REVISION)
get_git_dependency_revision(sokol SOKOL_REVISION)
get_git_dependency_revision(spirv-cross SPIRV_CROSS_REVISION)
get_git_dependency_revision(spirv-tools SPIRV_TOOLS_REVISION)
get_git_dependency_revision(stb STB_REVISION)
get_git_dependency_revision(tbb TBB_REVISION)
get_git_dependency_revision(zlib ZLIB_REVISION)

nanoem_cmake_find_glm()
nanoem_cmake_find_zlib()
nanoem_emapp_find_bx()
nanoem_emapp_find_minizip()
nanoem_emapp_find_nanomsg()
nanoem_emapp_find_mimalloc()

# lz4
set(LZ4_SUBMODULE_PATH "${PROJECT_SOURCE_DIR}/dependencies/lz4")
add_library(lz4 STATIC ${LZ4_SUBMODULE_PATH}/lib/lz4.c
                       ${LZ4_SUBMODULE_PATH}/lib/lz4frame.c
                       ${LZ4_SUBMODULE_PATH}/lib/lz4hc.c
                       ${LZ4_SUBMODULE_PATH}/lib/xxhash.c)

add_subdirectory(bundle)
add_subdirectory(resources)

nanoem_emapp_organize_executables(NANOEM_EXECUTABLES)
foreach(item ${NANOEM_EXECUTABLES})
  nanoem_emapp_link_executable(${item})
endforeach()

if(NANOEM_ENABLE_FUZZER)
  add_subdirectory(fuzz)
endif()
