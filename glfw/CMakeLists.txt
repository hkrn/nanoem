set(COMMON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common/include)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/common/src COMMON_SOURCES)

# GLFW
nanoem_cmake_get_install_path("glfw" GLFW_BASE_PATH GLFW_INSTALL_PATH_DEBUG GLFW_INSTALL_PATH_RELEASE)
find_path(GLFW_INCLUDE_DIR NAMES GLFW/glfw3.h AL/alure.h PATH_SUFFIXES include PATHS ${GLFW_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
find_library(GLFW_LIBRARY_DEBUG NAMES glfw3 PATH_SUFFIXES lib PATHS ${GLFW_INSTALL_PATH_DEBUG} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
find_library(GLFW_LIBRARY_RELEASE NAMES glfw3 PATH_SUFFIXES lib PATHS ${GLFW_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
mark_as_advanced(GLFW_BASE_PATH GLFW_INSTALL_PATH_DEBUG GLFW_INSTALL_PATH_RELEASE GLFW_LIBRARY_DEBUG GLFW_LIBRARY_RELEASE GLFW_INCLUDE_DIR)

# libsoundio
nanoem_cmake_get_install_path("libsoundio" SOUNDIO_BASE_PATH SOUNDIO_INSTALL_PATH_DEBUG SOUNDIO_INSTALL_PATH_RELEASE)
find_path(SOUNDIO_INCLUDE_DIR NAMES soundio/soundio.h PATH_SUFFIXES include PATHS ${SOUNDIO_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
find_library(SOUNDIO_LIBRARY_DEBUG NAMES soundio PATH_SUFFIXES lib PATHS ${SOUNDIO_INSTALL_PATH_DEBUG} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
find_library(SOUNDIO_LIBRARY_RELEASE NAMES soundio PATH_SUFFIXES lib PATHS ${SOUNDIO_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
mark_as_advanced(SOUNDIO_BASE_PATH SOUNDIO_INSTALL_PATH_DEBUG SOUNDIO_INSTALL_PATH_RELEASE SOUNDIO_LIBRARY_DEBUG SOUNDIO_LIBRARY_RELEASE SOUNDIO_INCLUDE_DIR)

# shaders
set(glslang_common_options -D -DGLSLANG=1 --target-env vulkan1.0)
set(glslang_vertex_shader_options ${glslang_common_options} -S vert -e nanoemVSMain)
set(spirv_cross_common_options --remove-unused-variables)
set(spirv_cross_glsl_core33_options ${spirv_cross_common_options} --no-420pack-extension --flatten-ubo --version 330)
set(spirv_cross_glsl_es30_options ${spirv_cross_common_options} --no-420pack-extension --flatten-ubo --es --version 300)

function(glfw_link_sentry_sdk _target)
  if(NANOEM_ENABLE_SENTRY_SDK)
    nanoem_cmake_get_install_path("sentry-native" SENTRY_NATIVE_BASE_PATH SENTRY_NATIVE_INSTALL_PATH_DEBUG SENTRY_NATIVE_INSTALL_PATH_RELEASE)
    find_library(SENTRY_LIBRARY_PATH sentry PATH_SUFFIXES lib PATHS ${SENTRY_NATIVE_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
    find_program(SENTRY_CRASHPAD_HANDLER_PATH crashpad_handler PATH_SUFFIXES bin PATHS ${SENTRY_NATIVE_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
    mark_as_advanced(SENTRY_LIBRARY_PATH SENTRY_CRASHPAD_HANDLER_PATH)
    set_property(TARGET ${_target} APPEND PROPERTY COMPILE_DEFINITIONS NANOEM_HAS_SENTRY $<$<BOOL:$ENV{NANOEM_SENTRY_DSN}>:NANOEM_SENTRY_DSN="$ENV{NANOEM_SENTRY_DSN}">)
    list(APPEND LIBRARY_PATHS ${SENTRY_LIBRARY_PATH})
    add_custom_command(TARGET ${_target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>/sentry"
                       COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SENTRY_LIBRARY_PATH}" "$<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>/sentry")
    install(FILES "${SENTRY_LIBRARY_PATH}" DESTINATION sentry)
    if(SENTRY_CRASHPAD_HANDLER_PATH)
      get_filename_component(SENTRY_LIBRARY_PATH ${SENTRY_CRASHPAD_PATH} DIRECTORY)
      add_custom_command(TARGET ${_target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SENTRY_CRASHPAD_HANDLER_PATH}" "$<TARGET_FILE_DIR:${NANOEM_EXECUTABLE_NAME}>/sentry")
      install(FILES "${SENTRY_CRASHPAD_HANDLER_PATH}" DESTINATION sentry)
    endif()
  endif()
endfunction()

function(compile_vertex_shader_spirv _source _include_path _intermediate_path _output_path _generated_sources)
  get_filename_component(source_without_extension ${_source} NAME_WE)
  # vertex shader with SPIR-V
  set(intermediate_spirv_vs_filename "${_intermediate_path}/${source_without_extension}_vs.spv")
  set(output_spirv_vs_filename "${_output_path}/${source_without_extension}_vs_spirv.h")
  if(NANOEM_ENABLE_SHADER_OPTIMIZED AND SPIRV_OPT_PATH)
    add_custom_command(OUTPUT ${intermediate_spirv_vs_filename}
                       COMMAND ${GLSLANG_PATH}
                         -I${_include_path}
                         $<$<BOOL:${NANOEM_ENABLE_SDEF}>:-DNANOEM_ENABLE_SDEF>
                         ${glslang_vertex_shader_options}
                         -o ${intermediate_spirv_vs_filename}
                         ${source}
                       COMMAND ${SPIRV_OPT_PATH}
                         -Os --strip-debug
                         ${intermediate_spirv_vs_filename}
                         -o ${intermediate_spirv_vs_filename}
                       MAIN_DEPENDENCY ${_source})
  else()
    add_custom_command(OUTPUT ${intermediate_spirv_vs_filename}
                       COMMAND ${GLSLANG_PATH}
                         -I${_include_path}
                         $<$<BOOL:${NANOEM_ENABLE_SDEF}>:-DNANOEM_ENABLE_SDEF>
                         ${glslang_vertex_shader_options}
                         -o ${intermediate_spirv_vs_filename}
                         ${source}
                       MAIN_DEPENDENCY ${_source})
  endif()
  add_custom_command(OUTPUT ${output_spirv_vs_filename}
                     COMMAND $<TARGET_FILE:emarb_bin2c>
                       --input=${intermediate_spirv_vs_filename}
                       --symbol=g_nanoem_${source_without_extension}_vs_spirv
                       > ${output_spirv_vs_filename}
                     MAIN_DEPENDENCY ${intermediate_spirv_vs_filename})
  set(${_generated_sources} "${${_generated_sources}};${output_spirv_vs_filename}" PARENT_SCOPE)
endfunction()

function(compile_vertex_shader_glsl_es3 _source _intermediate_path _output_path _generated_sources)
  get_filename_component(source_without_extension ${_source} NAME_WE)
  # pixel shader with GLSL
  set(intermediate_spirv_vs_filename "${_intermediate_path}/${source_without_extension}_vs.spv")
  set(intermediate_glsl_vs_filename "${_intermediate_path}/${source_without_extension}_vs_es3.vert")
  set(output_glsl_vs_filename "${_output_path}/${source_without_extension}_vs_glsl_es3.h")
  add_custom_command(OUTPUT ${intermediate_glsl_vs_filename}
                     COMMAND ${SPIRV_CROSS_PATH}
                       ${spirv_cross_glsl_es30_options}
                       --rename-interface-variable in  0 a_position
                       --rename-interface-variable in  1 a_normal
                       --rename-interface-variable in  2 a_texcoord
                       --rename-interface-variable in  3 a_edge
                       --rename-interface-variable in  4 a_uva1
                       --rename-interface-variable in  5 a_uva2
                       --rename-interface-variable in  6 a_uva3
                       --rename-interface-variable in  7 a_uva4
                       --rename-interface-variable in  8 a_weights
                       --rename-interface-variable in  9 a_indices
                       --rename-interface-variable in 10 a_info
                       --rename-interface-variable out  0 v_position
                       --rename-interface-variable out  1 v_normal
                       --rename-interface-variable out  2 v_texcoord
                       --rename-interface-variable out  3 v_edge
                       --rename-interface-variable out  4 v_uva1
                       --rename-interface-variable out  5 v_uva2
                       --rename-interface-variable out  6 v_uva3
                       --rename-interface-variable out  7 v_uva4
                       --rename-interface-variable out  8 v_weights
                       --rename-interface-variable out  9 v_indices
                       --rename-interface-variable out 10 v_info
                       --output ${intermediate_glsl_vs_filename}
                       ${intermediate_spirv_vs_filename}
                     MAIN_DEPENDENCY ${intermediate_spirv_vs_filename})
  add_custom_command(OUTPUT ${output_glsl_vs_filename}
                     COMMAND $<TARGET_FILE:emarb_bin2c>
                       --input=${intermediate_glsl_vs_filename}
                       --symbol=g_nanoem_${source_without_extension}_fs_glsl_es3
                       --as-string=true > ${output_glsl_vs_filename}
                     MAIN_DEPENDENCY ${intermediate_glsl_vs_filename})
  set(${_generated_sources} "${${_generated_sources}};${output_glsl_vs_filename}" PARENT_SCOPE)
endfunction()

set(source "${CMAKE_CURRENT_SOURCE_DIR}/model_skinning_tf.hlsl")
set(include_path "${PROJECT_SOURCE_DIR}/emapp/resources/shaders/include")
set(intermediate_path_prefix "${CMAKE_CURRENT_BINARY_DIR}/intermediate")
set(output_path_prefix "${PROJECT_BINARY_DIR}/emapp/resources/include/emapp/private/shaders")
compile_vertex_shader_spirv(${source} ${include_path} ${intermediate_path_prefix} ${output_path_prefix} generated_sources)
compile_vertex_shader_glsl_es3(${source} ${intermediate_path_prefix} ${output_path_prefix} generated_sources)
add_custom_target(${PROJECT_NAME}_glfw_shaders SOURCES ${generated_sources})
set_property(TARGET ${PROJECT_NAME}_glfw_shaders PROPERTY FOLDER nanoem/resources)

if(NANOEM_ENABLE_GLFW_MT)
  add_subdirectory("mt")
else()
  add_subdirectory("st")
endif()
