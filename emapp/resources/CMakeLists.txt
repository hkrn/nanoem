function(nanoem_emapp_build_emarb_credits)
  # credits
  if(NANOEM_ENABLE_UPDATE_CREDITS)
    set(input_path "${CMAKE_CURRENT_SOURCE_DIR}/credits.md")
    set(intermediate_path "${CMAKE_CURRENT_BINARY_DIR}/credits.md")
    set(output_path "${PROJECT_BINARY_DIR}/emapp/resources/include/emapp/private/resources/credits.h")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/credits.md.in ${input_path} @ONLY LF)
    add_custom_command(OUTPUT ${output_path}
                       COMMAND ${CMAKE_COMMAND} ARGS -E copy ${CMAKE_CURRENT_SOURCE_DIR}/credits.md ${intermediate_path}
                       COMMAND $<TARGET_FILE:emarb_bin2c> ARGS --input=${intermediate_path} --symbol=credits_md > ${output_path}
                       MAIN_DEPENDENCY ${input_path}
                       VERBATIM)
    add_custom_target(emarb_credits SOURCES ${output_path} "${CMAKE_CURRENT_SOURCE_DIR}/credits.md.in")
    set_property(TARGET emarb_credits PROPERTY FOLDER nanoem/resources)
    add_dependencies(emarb_credits emarb_bin2c)
    message(STATUS "[emapp/resource] setup emarb credits")
  endif()
endfunction()

function(nanoem_emapp_build_emarb suffix)
  set(_emarb_name "emarb${suffix}")
  add_library(${_emarb_name} STATIC ${PROJECT_SOURCE_DIR}/emapp/resources/toon.cc
                                    ${PROJECT_SOURCE_DIR}/emapp/resources/font.cc
                                    ${PROJECT_SOURCE_DIR}/emapp/resources/translation.cc
                                   ${PROJECT_SOURCE_DIR}/emapp/resources/credits.cc)
  nanoem_cmake_enable_lto(emarb)
  set_property(TARGET ${_emarb_name} PROPERTY FOLDER nanoem)
  target_compile_definitions(${_emarb_name} PRIVATE
                             __STDC_LIMIT_MACROS
                             __STDC_CONSTANT_MACROS
                             __STDC_FORMAT_MACROS)
  target_include_directories(${_emarb_name} PRIVATE
                            ${PROJECT_BINARY_DIR}/emapp/resources/include
                            ${PROJECT_SOURCE_DIR}/emapp/include
                            ${PROJECT_SOURCE_DIR}/dependencies
                            ${BX_COMPAT_INCLUDE_PATH}
                            ${BX_INCLUDE_DIR}
                            ${BX_THIRDPARTY_INCLUDE_DIR}
                            ${BIMG_INCLUDE_DIR}
                            ${GLM_INCLUDE_DIR})
  add_dependencies(${_emarb_name} emarb_fonts)
  add_dependencies(${_emarb_name} emarb_translations)
  if(TARGET emarb_credits)
    add_dependencies(${_emarb_name} emarb_credits)
  endif()
  if(TARGET emarb_protoc)
    add_dependencies(${_emarb_name} emarb_protoc)
  endif()
  if(TARGET emarb_shader)
    add_dependencies(${_emarb_name} emarb_shader)
  endif()
  if(TARGET emarb_toon)
    add_dependencies(${_emarb_name} emarb_toon)
  endif()
endfunction()

if(XCODE)
  set(_prefix ${PROJECT_BINARY_DIR}/emapp/resources/include/emapp/private)
  execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${_prefix}/resources
                  COMMAND ${CMAKE_COMMAND} -E make_directory ${_prefix}/shaders)
endif()
add_subdirectory(helpers)
add_subdirectory(fonts)
add_subdirectory(protobuf)
add_subdirectory(shaders)
add_subdirectory(toons)
add_subdirectory(translations)
nanoem_emapp_build_emarb_credits()
