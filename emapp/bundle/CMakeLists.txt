function(nanoem_emapp_build_embundle suffix)
  set(_embundle_name "embundle${suffix}")
  add_library(${_embundle_name} STATIC ${PROJECT_SOURCE_DIR}/emapp/bundle/bundle_c.c
                                       ${PROJECT_SOURCE_DIR}/emapp/bundle/bundle.cc)
  nanoem_cmake_enable_lto(${_embundle_name})
  if(WIN32)
    target_link_libraries(${_embundle_name} psapi imm32)
  endif()
  set_property(TARGET ${_embundle_name} PROPERTY FOLDER nanoem)
  target_compile_definitions(${_embundle_name} PRIVATE
                            $<$<BOOL:${WIN32}>:_CRT_SECURE_NO_WARNINGS=1>
                            __STDC_LIMIT_MACROS
                            __STDC_CONSTANT_MACROS
                            __STDC_FORMAT_MACROS)
  target_include_directories(${_embundle_name} PRIVATE
                            ${BX_COMPAT_INCLUDE_PATH}
                            ${BX_INCLUDE_DIR}
                            ${BX_THIRDPARTY_INCLUDE_DIR}
                            ${BIMG_INCLUDE_DIR}
                            ${GLM_INCLUDE_DIR}
                            ${PROJECT_SOURCE_DIR}/emapp/include
                            ${PROJECT_SOURCE_DIR}/dependencies
                            ${PROJECT_SOURCE_DIR}/dependencies/imgui)
  if(NANOEM_ENABLE_DEBUG_ALLOCATOR)
    target_compile_definitions(${_embundle_name} PRIVATE NANODXM_ENABLE_DEBUG_ALLOCATOR UNDO_ENABLE_DEBUG_ALLOCATOR)
  endif()
  message(STATUS "[emapp/bundle] setup emapp bundle")
endfunction()

add_subdirectory(sokol)
