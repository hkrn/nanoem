set(glslang_common_options -D -DGLSLANG=1 --target-env vulkan1.0)
set(glslang_vertex_shader_options ${glslang_common_options} -S vert -e nanoemVSMain)
set(glslang_pixel_shader_options ${glslang_common_options} -S frag -e nanoemPSMain)
set(glslang_compute_shader_options ${glslang_common_options} -S comp -e nanoemCSMain)

set(spirv_cross_common_options --remove-unused-variables --no-420pack-extension)
set(spirv_cross_glsl_core33_options ${spirv_cross_common_options} --flatten-ubo --version 330)
set(spirv_cross_glsl_core43_options ${spirv_cross_common_options} --version 430)
set(spirv_cross_glsl_es30_options ${spirv_cross_common_options} --flatten-ubo --es --version 300)
set(spirv_cross_glsl_es31_options ${spirv_cross_common_options} --es --version 310)
set(spirv_cross_msl_options ${spirv_cross_common_options} --msl)

if(NOT NANOEM_ENABLE_SHADER_OPTIMIZED)
  set(fxc_common_options /Od /Zi /nologo)
else()
  set(fxc_common_options /O3 /nologo /Qstrip_debug /Qstrip_reflect)
endif()
set(fxc_vertex_shader_options ${fxc_common_options} /T vs_4_0 /E nanoemVSMain)
set(fxc_pixel_shader_options ${fxc_common_options} /T ps_4_0 /E nanoemPSMain)
set(fxc_compute_shader_options ${fxc_common_options} /T cs_4_0 /E nanoemCSMain)
set(xcrun_metal_macos_options -arch air64 -emit-llvm -ffast-math -mmacosx-version-min=10.13 -std=osx-metal2.0 -c)
if(NOT NANOEM_ENABLE_SHADER_OPTIMIZED)
  set(xcrun_metal_macos_options ${xcrun_metal_macos_options} -gline-tables-only -MO)
endif()

function(nanoem_emapp_shader_compile_vertex_shaders_fxc _source _include_path _intermediate_path _output_path _generated_sources)
  get_filename_component(source_without_extension ${_source} NAME_WE)
  # vertex shader with HLSL
  set(intermediate_dxbc_vs_filename "${_intermediate_path}/${source_without_extension}_vs.asm")
  set(output_dxbc_vs_filename "${_output_path}/${source_without_extension}_vs_dxbc.h")
  add_custom_command(OUTPUT ${intermediate_dxbc_vs_filename}
                     COMMAND ${FXC_PATH} ARGS
                       ${fxc_vertex_shader_options}
                       /I${_include_path}
                       /Fo ${intermediate_dxbc_vs_filename}
                       ${source}
                     MAIN_DEPENDENCY ${_source}
                     VERBATIM)
  add_custom_command(OUTPUT ${output_dxbc_vs_filename}
                     COMMAND $<TARGET_FILE:emarb_bin2c> ARGS
                       --input=${intermediate_dxbc_vs_filename}
                       --symbol=g_nanoem_${source_without_extension}_vs_dxbc
                       --output=${output_dxbc_vs_filename}
                     MAIN_DEPENDENCY ${intermediate_dxbc_vs_filename}
                     VERBATIM)
  set(${_generated_sources} "${${_generated_sources}};${output_dxbc_vs_filename}" PARENT_SCOPE)
endfunction()

function(nanoem_emapp_shader_compile_pixel_shaders_fxc _source _include_path _intermediate_path _output_path _generated_sources)
  get_filename_component(source_without_extension ${_source} NAME_WE)
  # pixel shader with HLSL
  set(intermediate_dxbc_ps_filename "${_intermediate_path}/${source_without_extension}_ps.asm")
  set(output_dxbc_ps_filename "${_output_path}/${source_without_extension}_ps_dxbc.h")
  add_custom_command(OUTPUT ${intermediate_dxbc_ps_filename}
                     COMMAND ${FXC_PATH} ARGS
                       ${fxc_pixel_shader_options}
                       /I ${_include_path}
                       /Fo ${intermediate_dxbc_ps_filename}
                       ${source}
                     MAIN_DEPENDENCY ${_source}
                     VERBATIM)
  add_custom_command(OUTPUT ${output_dxbc_ps_filename}
                     COMMAND $<TARGET_FILE:emarb_bin2c> ARGS
                       --input=${intermediate_dxbc_ps_filename}
                       --symbol=g_nanoem_${source_without_extension}_ps_dxbc
                       --output=${output_dxbc_ps_filename}
                     MAIN_DEPENDENCY ${intermediate_dxbc_ps_filename}
                     VERBATIM)
  set(${_generated_sources} "${${_generated_sources}};${output_dxbc_ps_filename}" PARENT_SCOPE)
endfunction()

function(nanoem_emapp_shader_compile_compute_shaders_fxc _source _include_path _intermediate_path _output_path _generated_sources)
  get_filename_component(source_without_extension ${_source} NAME_WE)
  # compute shader with HLSL
  set(intermediate_dxbc_cs_filename "${_intermediate_path}/${source_without_extension}_cs.asm")
  set(output_dxbc_cs_filename "${_output_path}/${source_without_extension}_cs_dxbc.h")
  add_custom_command(OUTPUT ${intermediate_dxbc_cs_filename}
                     COMMAND ${FXC_PATH} ARGS
                       ${fxc_compute_shader_options}
                       /I ${_include_path}
                       /Fo ${intermediate_dxbc_cs_filename}
                       ${source}
                     MAIN_DEPENDENCY ${_source}
                     VERBATIM)
  add_custom_command(OUTPUT ${output_dxbc_cs_filename}
                     COMMAND $<TARGET_FILE:emarb_bin2c> ARGS
                       --input=${intermediate_dxbc_cs_filename}
                       --symbol=g_nanoem_${source_without_extension}_ps_dxbc
                       --output=${output_dxbc_cs_filename}
                     MAIN_DEPENDENCY ${intermediate_dxbc_cs_filename}
                     VERBATIM)
  set(${_generated_sources} "${${_generated_sources}};${output_dxbc_cs_filename}" PARENT_SCOPE)
endfunction()

function(nanoem_emapp_shader_compile_vertex_shader_spirv _source _include_path _intermediate_path _output_path _generated_sources)
  get_filename_component(source_without_extension ${_source} NAME_WE)
  # vertex shader with SPIR-V
  set(intermediate_spirv_vs_filename "${_intermediate_path}/${source_without_extension}_vs.spv")
  set(output_spirv_vs_filename "${_output_path}/${source_without_extension}_vs_spirv.h")
  if(NANOEM_ENABLE_SHADER_OPTIMIZED AND SPIRV_OPT_PATH)
    add_custom_command(OUTPUT ${intermediate_spirv_vs_filename}
                       COMMAND ${GLSLANG_PATH} ARGS
                         -I${_include_path}
                         ${glslang_vertex_shader_options}
                         -o ${intermediate_spirv_vs_filename}
                         ${source}
                       COMMAND ${SPIRV_OPT_PATH}
                         -Os --strip-debug
                         ${intermediate_spirv_vs_filename}
                         -o ${intermediate_spirv_vs_filename}
                       MAIN_DEPENDENCY ${_source}
                       VERBATIM)
  else()
    add_custom_command(OUTPUT ${intermediate_spirv_vs_filename}
                       COMMAND ${GLSLANG_PATH} ARGS
                         -I${_include_path}
                         ${glslang_vertex_shader_options}
                         -o ${intermediate_spirv_vs_filename}
                         ${source}
                       MAIN_DEPENDENCY ${_source}
                       VERBATIM)
  endif()
  add_custom_command(OUTPUT ${output_spirv_vs_filename}
                     COMMAND $<TARGET_FILE:emarb_bin2c> ARGS
                       --input=${intermediate_spirv_vs_filename}
                       --symbol=g_nanoem_${source_without_extension}_vs_spirv
                       --output=${output_spirv_vs_filename}
                     MAIN_DEPENDENCY ${intermediate_spirv_vs_filename}
                     VERBATIM)
  set(${_generated_sources} "${${_generated_sources}};${output_spirv_vs_filename}" PARENT_SCOPE)
endfunction()

function(nanoem_emapp_shader_compile_vertex_shader_glsl_core33 _source _intermediate_path _output_path _generated_sources)
  get_filename_component(source_without_extension ${_source} NAME_WE)
  # vertex shader with GLSL
  set(intermediate_spirv_vs_filename "${_intermediate_path}/${source_without_extension}_vs.spv")
  set(intermediate_glsl_vs_filename "${_intermediate_path}/${source_without_extension}_vs_core33.vert")
  set(output_glsl_vs_filename "${_output_path}/${source_without_extension}_vs_glsl_core33.h")
  add_custom_command(OUTPUT ${intermediate_glsl_vs_filename}
                     COMMAND ${SPIRV_CROSS_PATH} ARGS
                       ${spirv_cross_glsl_core33_options}
                       --output ${intermediate_glsl_vs_filename}
                       --rename-interface-variable in 0 a_position
                       --rename-interface-variable in 1 a_normal
                       --rename-interface-variable in 2 a_texcoord0
                       --rename-interface-variable in 3 a_texcoord1
                       --rename-interface-variable in 4 a_texcoord2
                       --rename-interface-variable in 5 a_texcoord3
                       --rename-interface-variable in 6 a_texcoord4
                       --rename-interface-variable in 7 a_color0
                       --rename-interface-variable out 0 v_color0
                       --rename-interface-variable out 1 v_normal
                       --rename-interface-variable out 2 v_texcoord0
                       --rename-interface-variable out 3 v_texcoord1
                       --rename-interface-variable out 4 v_eye
                       --rename-interface-variable out 5 v_shadow0
                       ${intermediate_spirv_vs_filename}
                     MAIN_DEPENDENCY ${intermediate_spirv_vs_filename}
                     VERBATIM)
  add_custom_command(OUTPUT ${output_glsl_vs_filename}
                     COMMAND $<TARGET_FILE:emarb_bin2c> ARGS
                       --input=${intermediate_glsl_vs_filename}
                       --symbol=g_nanoem_${source_without_extension}_vs_glsl_core33
                       --as-string=true
                       --output=${output_glsl_vs_filename}
                     MAIN_DEPENDENCY ${intermediate_glsl_vs_filename}
                     VERBATIM)
  set(${_generated_sources} "${${_generated_sources}};${output_glsl_vs_filename}" PARENT_SCOPE)
endfunction()

function(nanoem_emapp_shader_compile_vertex_shader_glsl_es3 _source _intermediate_path _output_path _generated_sources)
  get_filename_component(source_without_extension ${_source} NAME_WE)
  # vertex shader with GLSL
  set(intermediate_spirv_vs_filename "${_intermediate_path}/${source_without_extension}_vs.spv")
  set(intermediate_glsl_vs_filename "${_intermediate_path}/${source_without_extension}_vs_es3.vert")
  set(output_glsl_vs_filename "${_output_path}/${source_without_extension}_vs_glsl_es3.h")
  add_custom_command(OUTPUT ${intermediate_glsl_vs_filename}
                     COMMAND ${SPIRV_CROSS_PATH} ARGS
                       ${spirv_cross_glsl_es30_options}
                       --output ${intermediate_glsl_vs_filename}
                       --rename-interface-variable in 0 a_position
                       --rename-interface-variable in 1 a_normal
                       --rename-interface-variable in 2 a_texcoord0
                       --rename-interface-variable in 3 a_texcoord1
                       --rename-interface-variable in 4 a_texcoord2
                       --rename-interface-variable in 5 a_texcoord3
                       --rename-interface-variable in 6 a_texcoord4
                       --rename-interface-variable in 7 a_color0
                       --rename-interface-variable out 0 v_color0
                       --rename-interface-variable out 1 v_normal
                       --rename-interface-variable out 2 v_texcoord0
                       --rename-interface-variable out 3 v_texcoord1
                       --rename-interface-variable out 4 v_eye
                       --rename-interface-variable out 5 v_shadow0
                       ${intermediate_spirv_vs_filename}
                     MAIN_DEPENDENCY ${intermediate_spirv_vs_filename}
                     VERBATIM)
  add_custom_command(OUTPUT ${output_glsl_vs_filename}
                     COMMAND $<TARGET_FILE:emarb_bin2c> ARGS
                       --input=${intermediate_glsl_vs_filename}
                       --symbol=g_nanoem_${source_without_extension}_vs_glsl_es3
                       --as-string=true
                       --output=${output_glsl_vs_filename}
                     MAIN_DEPENDENCY ${intermediate_glsl_vs_filename}
                     VERBATIM)
  set(${_generated_sources} "${${_generated_sources}};${output_glsl_vs_filename}" PARENT_SCOPE)
endfunction()

function(nanoem_emapp_shader_compile_vertex_shader_msl _source _intermediate_path _output_path _generated_sources)
  get_filename_component(source_without_extension ${_source} NAME_WE)
  # vertex shader with MSL
  set(intermediate_spirv_vs_filename "${_intermediate_path}/${source_without_extension}_vs.spv")
  set(intermediate_msl_vs_filename "${_intermediate_path}/${source_without_extension}_vs_macos.metal")
  set(output_msl_vs_filename "${_output_path}/${source_without_extension}_vs_msl_macos.h")
  add_custom_command(OUTPUT ${intermediate_msl_vs_filename}
                     COMMAND ${SPIRV_CROSS_PATH} ARGS
                       ${spirv_cross_msl_options}
                       --output ${intermediate_msl_vs_filename}
                       --msl-version 10000
                       ${intermediate_spirv_vs_filename}
                     MAIN_DEPENDENCY ${intermediate_spirv_vs_filename}
                     VERBATIM)
  add_custom_command(OUTPUT ${intermediate_msl_vs_filename}.air
                     COMMAND ${XCRUN_PATH} ARGS
                       metal ${xcrun_metal_macos_options}
                       -o ${intermediate_msl_vs_filename}.air
                       ${intermediate_msl_vs_filename}
                     MAIN_DEPENDENCY ${intermediate_msl_vs_filename}
                     VERBATIM)
  add_custom_command(OUTPUT ${intermediate_msl_vs_filename}.ar
                     COMMAND ${XCRUN_PATH} ARGS
                       metal-ar r
                       ${intermediate_msl_vs_filename}.ar
                       ${intermediate_msl_vs_filename}.air
                     MAIN_DEPENDENCY ${intermediate_msl_vs_filename}.air
                     VERBATIM)
  add_custom_command(OUTPUT ${intermediate_msl_vs_filename}.lib
                     COMMAND ${XCRUN_PATH} ARGS
                       metallib
                       -o ${intermediate_msl_vs_filename}.lib
                       ${intermediate_msl_vs_filename}.ar
                     MAIN_DEPENDENCY ${intermediate_msl_vs_filename}.ar
                     VERBATIM)
  add_custom_command(OUTPUT ${output_msl_vs_filename}
                     COMMAND $<TARGET_FILE:emarb_bin2c> ARGS
                       --input=${intermediate_msl_vs_filename}.lib
                       --symbol=g_nanoem_${source_without_extension}_vs_msl_macos
                       --output=${output_msl_vs_filename}
                     MAIN_DEPENDENCY ${intermediate_msl_vs_filename}.lib
                     VERBATIM)
  set(${_generated_sources} "${${_generated_sources}};${output_msl_vs_filename}" PARENT_SCOPE)
endfunction()

function(nanoem_emapp_shader_compile_pixel_shader_spirv _source _include_path _intermediate_path _output_path _generated_sources)
  get_filename_component(source_without_extension ${_source} NAME_WE)
  # pixel shader with SPIR-V
  set(intermediate_spirv_fs_filename "${_intermediate_path}/${source_without_extension}_fs.spv")
  set(output_spirv_fs_filename "${_output_path}/${source_without_extension}_fs_spirv.h")
  if(NANOEM_ENABLE_SHADER_OPTIMIZED AND SPIRV_OPT_PATH)
    add_custom_command(OUTPUT ${intermediate_spirv_fs_filename}
                       COMMAND ${GLSLANG_PATH} ARGS
                         -I${_include_path}
                         ${glslang_pixel_shader_options}
                         -o ${intermediate_spirv_fs_filename}
                         ${source}
                       COMMAND ${SPIRV_OPT_PATH} ARGS
                         -Os --strip-debug
                         ${intermediate_spirv_fs_filename}
                         -o ${intermediate_spirv_fs_filename}
                       MAIN_DEPENDENCY ${_source}
                       VERBATIM)
  else()
    add_custom_command(OUTPUT ${intermediate_spirv_fs_filename}
                       COMMAND ${GLSLANG_PATH} ARGS
                         -I${_include_path}
                         ${glslang_pixel_shader_options}
                         -o ${intermediate_spirv_fs_filename}
                         ${source}
                       MAIN_DEPENDENCY ${_source}
                       VERBATIM)
  endif()
  add_custom_command(OUTPUT ${output_spirv_fs_filename}
                     COMMAND $<TARGET_FILE:emarb_bin2c> ARGS
                       --input=${intermediate_spirv_fs_filename}
                       --symbol=g_nanoem_${source_without_extension}_fs_spirv
                       --output=${output_spirv_fs_filename}
                     MAIN_DEPENDENCY ${intermediate_spirv_fs_filename}
                     VERBATIM)
  set(${_generated_sources} "${${_generated_sources}};${output_spirv_fs_filename}" PARENT_SCOPE)
endfunction()

function(nanoem_emapp_shader_compile_pixel_shader_glsl_core33 _source _intermediate_path _output_path _generated_sources)
  get_filename_component(source_without_extension ${_source} NAME_WE)
  # pixel shader with GLSL
  set(intermediate_spirv_fs_filename "${_intermediate_path}/${source_without_extension}_fs.spv")
  set(intermediate_glsl_fs_filename "${_intermediate_path}/${source_without_extension}_fs_core33.frag")
  set(output_glsl_fs_filename "${_output_path}/${source_without_extension}_fs_glsl_core33.h")
  add_custom_command(OUTPUT ${intermediate_glsl_fs_filename}
                     COMMAND ${SPIRV_CROSS_PATH} ARGS
                       ${spirv_cross_glsl_core33_options}
                       --output ${intermediate_glsl_fs_filename}
                       --rename-interface-variable in 0 v_color0
                       --rename-interface-variable in 1 v_normal
                       --rename-interface-variable in 2 v_texcoord0
                       --rename-interface-variable in 3 v_texcoord1
                       --rename-interface-variable in 4 v_eye
                       --rename-interface-variable in 5 v_shadow0
                       ${intermediate_spirv_fs_filename}
                     MAIN_DEPENDENCY ${intermediate_spirv_fs_filename}
                     VERBATIM)
  add_custom_command(OUTPUT ${output_glsl_fs_filename}
                     COMMAND $<TARGET_FILE:emarb_bin2c> ARGS
                       --input=${intermediate_glsl_fs_filename}
                       --symbol=g_nanoem_${source_without_extension}_fs_glsl_core33
                       --as-string=true
                       --output=${output_glsl_fs_filename}
                     MAIN_DEPENDENCY ${intermediate_glsl_fs_filename}
                     VERBATIM)
  set(${_generated_sources} "${${_generated_sources}};${output_glsl_fs_filename}" PARENT_SCOPE)
endfunction()

function(nanoem_emapp_shader_compile_pixel_shader_glsl_es3 _source _intermediate_path _output_path _generated_sources)
  get_filename_component(source_without_extension ${_source} NAME_WE)
  # pixel shader with GLSL
  set(intermediate_spirv_fs_filename "${_intermediate_path}/${source_without_extension}_fs.spv")
  set(intermediate_glsl_fs_filename "${_intermediate_path}/${source_without_extension}_fs_es3.frag")
  set(output_glsl_fs_filename "${_output_path}/${source_without_extension}_fs_glsl_es3.h")
  add_custom_command(OUTPUT ${intermediate_glsl_fs_filename}
                     COMMAND ${SPIRV_CROSS_PATH} ARGS
                       ${spirv_cross_glsl_es30_options}
                       --output ${intermediate_glsl_fs_filename}
                       --rename-interface-variable in 0 v_color0
                       --rename-interface-variable in 1 v_normal
                       --rename-interface-variable in 2 v_texcoord0
                       --rename-interface-variable in 3 v_texcoord1
                       --rename-interface-variable in 4 v_eye
                       --rename-interface-variable in 5 v_shadow0
                       ${intermediate_spirv_fs_filename}
                     MAIN_DEPENDENCY ${intermediate_spirv_fs_filename}
                     VERBATIM)
  add_custom_command(OUTPUT ${output_glsl_fs_filename}
                     COMMAND $<TARGET_FILE:emarb_bin2c> ARGS
                       --input=${intermediate_glsl_fs_filename}
                       --symbol=g_nanoem_${source_without_extension}_fs_glsl_es3
                       --as-string=true
                       --output=${output_glsl_fs_filename}
                     MAIN_DEPENDENCY ${intermediate_glsl_fs_filename}
                     VERBATIM)
  set(${_generated_sources} "${${_generated_sources}};${output_glsl_fs_filename}" PARENT_SCOPE)
endfunction()

function(nanoem_emapp_shader_compile_pixel_shader_msl _source _intermediate_path _output_path _generated_sources)
  get_filename_component(source_without_extension ${_source} NAME_WE)
  # pixel shader with MSL
  set(intermediate_spirv_fs_filename "${_intermediate_path}/${source_without_extension}_fs.spv")
  set(intermediate_msl_fs_filename "${_intermediate_path}/${source_without_extension}_fs_macos.metal")
  set(output_msl_fs_filename "${_output_path}/${source_without_extension}_fs_msl_macos.h")
  add_custom_command(OUTPUT ${intermediate_msl_fs_filename}
                     COMMAND ${SPIRV_CROSS_PATH} ARGS
                       ${spirv_cross_msl_options}
                       --output ${intermediate_msl_fs_filename}
                       --msl-version 10000
                       ${intermediate_spirv_fs_filename}
                     MAIN_DEPENDENCY ${intermediate_spirv_fs_filename}
                     VERBATIM)
  add_custom_command(OUTPUT ${intermediate_msl_fs_filename}.air
                     COMMAND ${XCRUN_PATH} ARGS
                       metal ${xcrun_metal_macos_options}
                       -o ${intermediate_msl_fs_filename}.air
                       ${intermediate_msl_fs_filename}
                     MAIN_DEPENDENCY ${intermediate_msl_fs_filename}
                     VERBATIM)
  add_custom_command(OUTPUT ${intermediate_msl_fs_filename}.ar
                     COMMAND ${XCRUN_PATH} ARGS
                       metal-ar r
                       ${intermediate_msl_fs_filename}.ar
                       ${intermediate_msl_fs_filename}.air
                     MAIN_DEPENDENCY ${intermediate_msl_fs_filename}.air
                     VERBATIM)
  add_custom_command(OUTPUT ${intermediate_msl_fs_filename}.lib
                     COMMAND ${XCRUN_PATH} ARGS
                       metallib
                       -o ${intermediate_msl_fs_filename}.lib
                       ${intermediate_msl_fs_filename}.ar
                     MAIN_DEPENDENCY ${intermediate_msl_fs_filename}.ar
                     VERBATIM)
  add_custom_command(OUTPUT ${output_msl_fs_filename}
                     COMMAND $<TARGET_FILE:emarb_bin2c> ARGS
                       --input=${intermediate_msl_fs_filename}.lib
                       --symbol=g_nanoem_${source_without_extension}_fs_msl_macos
                       --output=${output_msl_fs_filename}
                     MAIN_DEPENDENCY ${intermediate_msl_fs_filename}.lib
                     VERBATIM)
  set(${_generated_sources} "${${_generated_sources}};${output_msl_fs_filename}" PARENT_SCOPE)
endfunction()

function(nanoem_emapp_shader_compile_compute_shader_spirv _source _include_path _intermediate_path _output_path _generated_sources)
  get_filename_component(source_without_extension ${_source} NAME_WE)
  # vertex shader with SPIR-V
  set(intermediate_spirv_cs_filename "${_intermediate_path}/${source_without_extension}_cs.spv")
  set(output_spirv_cs_filename "${_output_path}/${source_without_extension}_cs_spirv.h")
  if(NANOEM_ENABLE_SHADER_OPTIMIZED AND SPIRV_OPT_PATH)
    add_custom_command(OUTPUT ${intermediate_spirv_cs_filename}
                       COMMAND ${GLSLANG_PATH} ARGS
                         -I${_include_path}
                         $<$<BOOL:${NANOEM_ENABLE_SDEF}>:-DNANOEM_ENABLE_SDEF>
                         ${glslang_compute_shader_options}
                         -o ${intermediate_spirv_cs_filename}
                         ${source}
                       COMMAND ${SPIRV_OPT_PATH} ARGS
                         -Os --strip-debug
                         ${intermediate_spirv_cs_filename}
                         -o ${intermediate_spirv_cs_filename}
                       MAIN_DEPENDENCY ${_source}
                       VERBATIM)
  else()
    add_custom_command(OUTPUT ${intermediate_spirv_cs_filename}
                       COMMAND ${GLSLANG_PATH} ARGS
                         -I${_include_path}
                         $<$<BOOL:${NANOEM_ENABLE_SDEF}>:-DNANOEM_ENABLE_SDEF>
                         ${glslang_compute_shader_options}
                         -o ${intermediate_spirv_cs_filename}
                         ${source}
                       MAIN_DEPENDENCY ${_source}
                       VERBATIM)
  endif()
  add_custom_command(OUTPUT ${output_spirv_cs_filename}
                     COMMAND $<TARGET_FILE:emarb_bin2c> ARGS
                       --input=${intermediate_spirv_cs_filename}
                       --symbol=g_nanoem_${source_without_extension}_cs_spirv
                       --output=${output_spirv_cs_filename}
                     MAIN_DEPENDENCY ${intermediate_spirv_cs_filename}
                     VERBATIM)
  set(${_generated_sources} "${${_generated_sources}};${output_spirv_cs_filename}" PARENT_SCOPE)
endfunction()

function(nanoem_emapp_shader_compile_compute_shader_glsl_core43 _source _intermediate_path _output_path _generated_sources)
  get_filename_component(source_without_extension ${_source} NAME_WE)
  # compute shader with GLSL
  set(intermediate_spirv_cs_filename "${_intermediate_path}/${source_without_extension}_cs.spv")
  set(intermediate_glsl_cs_filename "${_intermediate_path}/${source_without_extension}_cs_core43.comp")
  set(output_glsl_cs_filename "${_output_path}/${source_without_extension}_cs_glsl_core43.h")
  add_custom_command(OUTPUT ${intermediate_glsl_cs_filename}
                     COMMAND ${SPIRV_CROSS_PATH} ARGS
                       ${spirv_cross_glsl_core43_options}
                       --output ${intermediate_glsl_cs_filename}
                       ${intermediate_spirv_cs_filename}
                     MAIN_DEPENDENCY ${intermediate_spirv_cs_filename}
                     VERBATIM)
  add_custom_command(OUTPUT ${output_glsl_cs_filename}
                     COMMAND $<TARGET_FILE:emarb_bin2c> ARGS
                       --input=${intermediate_glsl_cs_filename}
                       --symbol=g_nanoem_${source_without_extension}_cs_glsl_core43
                       --as-string=true
                       --output=${output_glsl_cs_filename}
                     MAIN_DEPENDENCY ${intermediate_glsl_cs_filename}
                     VERBATIM)
  set(${_generated_sources} "${${_generated_sources}};${output_glsl_cs_filename}" PARENT_SCOPE)
endfunction()

function(nanoem_emapp_shader_compile_compute_shader_glsl_es31 _source _intermediate_path _output_path _generated_sources)
  get_filename_component(source_without_extension ${_source} NAME_WE)
  # compute shader with GLSL
  set(intermediate_spirv_cs_filename "${_intermediate_path}/${source_without_extension}_cs.spv")
  set(intermediate_glsl_cs_filename "${_intermediate_path}/${source_without_extension}_cs_es31.comp")
  set(output_glsl_cs_filename "${_output_path}/${source_without_extension}_cs_glsl_es31.h")
  add_custom_command(OUTPUT ${intermediate_glsl_cs_filename}
                     COMMAND ${SPIRV_CROSS_PATH} ARGS
                       ${spirv_cross_glsl_es31_options}
                       --output ${intermediate_glsl_cs_filename}
                       ${intermediate_spirv_cs_filename}
                     MAIN_DEPENDENCY ${intermediate_spirv_cs_filename}
                     VERBATIM)
  add_custom_command(OUTPUT ${output_glsl_cs_filename}
                     COMMAND $<TARGET_FILE:emarb_bin2c> ARGS
                       --input=${intermediate_glsl_cs_filename}
                       --symbol=g_nanoem_${source_without_extension}_cs_glsl_es31
                       --as-string=true
                       --output=${output_glsl_cs_filename}
                     MAIN_DEPENDENCY ${intermediate_glsl_cs_filename}
                     VERBATIM)
  set(${_generated_sources} "${${_generated_sources}};${output_glsl_cs_filename}" PARENT_SCOPE)
endfunction()

function(nanoem_emapp_shader_compile_compute_shader_msl _source _intermediate_path _output_path _generated_sources)
  get_filename_component(source_without_extension ${_source} NAME_WE)
  # pixel shader with MSL
  set(intermediate_spirv_cs_filename "${_intermediate_path}/${source_without_extension}_cs.spv")
  set(intermediate_msl_cs_filename "${_intermediate_path}/${source_without_extension}_cs_macos.metal")
  set(output_msl_cs_filename "${_output_path}/${source_without_extension}_cs_msl_macos.h")
  add_custom_command(OUTPUT ${intermediate_msl_cs_filename}
                     COMMAND ${SPIRV_CROSS_PATH} ARGS
                       ${spirv_cross_msl_options}
                       --output ${intermediate_msl_cs_filename}
                       --msl-version 20000
                       --msl-argument-buffers
                       ${intermediate_spirv_cs_filename}
                     MAIN_DEPENDENCY ${intermediate_spirv_cs_filename}
                     VERBATIM)
  add_custom_command(OUTPUT ${intermediate_msl_cs_filename}.air
                     COMMAND ${XCRUN_PATH} ARGS
                       metal ${xcrun_metal_macos_options}
                       -o ${intermediate_msl_cs_filename}.air
                       ${intermediate_msl_cs_filename}
                     MAIN_DEPENDENCY ${intermediate_msl_cs_filename}
                     VERBATIM)
  add_custom_command(OUTPUT ${intermediate_msl_cs_filename}.ar
                     COMMAND ${XCRUN_PATH} ARGS
                       metal-ar r
                       ${intermediate_msl_cs_filename}.ar
                       ${intermediate_msl_cs_filename}.air
                     MAIN_DEPENDENCY ${intermediate_msl_cs_filename}.air
                     VERBATIM)
  add_custom_command(OUTPUT ${intermediate_msl_cs_filename}.lib
                     COMMAND ${XCRUN_PATH} ARGS
                       metallib
                       -o ${intermediate_msl_cs_filename}.lib
                       ${intermediate_msl_cs_filename}.ar
                     MAIN_DEPENDENCY ${intermediate_msl_cs_filename}.ar
                     VERBATIM)
  add_custom_command(OUTPUT ${output_msl_cs_filename}
                     COMMAND $<TARGET_FILE:emarb_bin2c> ARGS
                       --input=${intermediate_msl_cs_filename}.lib
                       --symbol=g_nanoem_${source_without_extension}_cs_msl_macos
                       --output=${output_msl_cs_filename}
                     MAIN_DEPENDENCY ${intermediate_msl_cs_filename}.lib
                     VERBATIM)
  set(${_generated_sources} "${${_generated_sources}};${output_msl_cs_filename}" PARENT_SCOPE)
endfunction()

function(nanoem_emapp_shader_compile_all_shaders)
  set(intermediate_path_prefix "${CMAKE_CURRENT_BINARY_DIR}/intermediate")
  set(output_path_prefix "${PROJECT_BINARY_DIR}/emapp/resources/include/emapp/private/shaders")
  set(include_path "${CMAKE_CURRENT_SOURCE_DIR}/include")
  file(GLOB VERTEX_PIXEL_SHADER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.hlsl)
  file(GLOB COMPUTE_SHADER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/compute/*.hlsl)
  find_program(FXC_PATH fxc)
  nanoem_cmake_get_install_path("glslang" GLSLANG_INSTALL_BASE_PATH GLSLANG_INSTALL_PATH_DEBUG GLSLANG_INSTALL_PATH_RELEASE)
  nanoem_cmake_get_install_path("spirv-cross" SPIRV_CROSS_INSTALL_BASE_PATH SPIRV_CROSS_INSTALL_PATH_DEBUG SPIRV_CROSS_INSTALL_PATH_RELEASE)
  nanoem_cmake_get_install_path("spirv-tools" SPIRV_TOOLS_INSTALL_BASE_PATH SPIRV_TOOLS_INSTALL_PATH_DEBUG SPIRV_TOOLS_INSTALL_PATH_RELEASE)
  find_program(GLSLANG_PATH glslangValidator PATH_SUFFIXES bin PATHS ${GLSLANG_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  find_program(SPIRV_CROSS_PATH spirv-cross PATH_SUFFIXES bin PATHS ${SPIRV_CROSS_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  find_program(SPIRV_OPT_PATH spirv-opt PATH_SUFFIXES bin PATHS ${SPIRV_TOOLS_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  mark_as_advanced(FXC_PATH GLSLANG_PATH SPIRV_CROSS_PATH SPIRV_OPT_PATH)
  # DirectX
  if(FXC_PATH)
    foreach(source ${VERTEX_PIXEL_SHADER_SOURCES})
      nanoem_emapp_shader_compile_vertex_shaders_fxc(${source} ${include_path} ${intermediate_path_prefix} ${output_path_prefix} generated_sources)
      nanoem_emapp_shader_compile_pixel_shaders_fxc(${source} ${include_path} ${intermediate_path_prefix} ${output_path_prefix} generated_sources)
    endforeach()
    foreach(source ${COMPUTE_SHADER_SOURCES})
      nanoem_emapp_shader_compile_compute_shaders_fxc(${source} ${include_path} ${intermediate_path_prefix} ${output_path_prefix} generated_sources)
    endforeach()
  endif()
  if(GLSLANG_PATH AND SPIRV_CROSS_PATH)
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${intermediate_path_prefix})
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${output_path_prefix})
    # SPIRV and GLSL
    foreach(source ${VERTEX_PIXEL_SHADER_SOURCES})
      nanoem_emapp_shader_compile_vertex_shader_spirv(${source} ${include_path} ${intermediate_path_prefix} ${output_path_prefix} generated_sources)
      nanoem_emapp_shader_compile_vertex_shader_glsl_core33(${source} ${intermediate_path_prefix} ${output_path_prefix} generated_sources)
      nanoem_emapp_shader_compile_vertex_shader_glsl_es3(${source} ${intermediate_path_prefix} ${output_path_prefix} generated_sources)
      nanoem_emapp_shader_compile_pixel_shader_spirv(${source} ${include_path} ${intermediate_path_prefix} ${output_path_prefix} generated_sources)
      nanoem_emapp_shader_compile_pixel_shader_glsl_core33(${source} ${intermediate_path_prefix} ${output_path_prefix} generated_sources)
      nanoem_emapp_shader_compile_pixel_shader_glsl_es3(${source} ${intermediate_path_prefix} ${output_path_prefix} generated_sources)
    endforeach()
    foreach(source ${COMPUTE_SHADER_SOURCES})
      nanoem_emapp_shader_compile_compute_shader_spirv(${source} ${include_path} ${intermediate_path_prefix} ${output_path_prefix} generated_sources)
      nanoem_emapp_shader_compile_compute_shader_glsl_core43(${source} ${intermediate_path_prefix} ${output_path_prefix} generated_sources)
      nanoem_emapp_shader_compile_compute_shader_glsl_es31(${source} ${intermediate_path_prefix} ${output_path_prefix} generated_sources)
    endforeach()
    # Metal
    find_program(XCRUN_PATH xcrun)
    mark_as_advanced(XCRUN_PATH)
    if(XCRUN_PATH)
      foreach(source ${VERTEX_PIXEL_SHADER_SOURCES})
        nanoem_emapp_shader_compile_vertex_shader_msl(${source} ${intermediate_path_prefix} ${output_path_prefix} generated_sources)
        nanoem_emapp_shader_compile_pixel_shader_msl(${source} ${intermediate_path_prefix} ${output_path_prefix} generated_sources)
      endforeach()
      foreach(source ${COMPUTE_SHADER_SOURCES})
        nanoem_emapp_shader_compile_compute_shader_msl(${source} ${intermediate_path_prefix} ${output_path_prefix} generated_sources)
      endforeach()
    endif()
  endif()
  if(generated_sources)
    add_custom_target(emarb_shader SOURCES ${generated_sources})
    set_property(TARGET emarb_shader PROPERTY FOLDER nanoem/resources)
    option(NANOEM_ENABLE_COPY_ALL_SHADERS OFF "Enable copying all shader binaries to emapp/private/shaders directory")
    mark_as_advanced(NANOEM_ENABLE_COPY_ALL_SHADERS)
    if(NANOEM_ENABLE_COPY_ALL_SHADERS)
      set(content "")
      foreach(source ${generated_sources})
        get_filename_component(filename ${source} NAME)
        set(dest "${PROJECT_SOURCE_DIR}/emapp/include/emapp/private/shaders")
        string(APPEND content "file(COPY \"${source}\" DESTINATION \"${dest}\")\n")
      endforeach()
      file(GENERATE OUTPUT ${intermediate_path_prefix}/copy.cmake CONTENT ${content})
      add_custom_command(TARGET emarb_shader
                         POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -P ${intermediate_path_prefix}/copy.cmake
                         VERBATIM)
    endif()
  endif()
  message(STATUS "[emapp/resources] setup emarb shaders")
endfunction()

nanoem_emapp_shader_compile_all_shaders()
