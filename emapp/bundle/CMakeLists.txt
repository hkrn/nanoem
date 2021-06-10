function(nanoem_emapp_build_embundle)
  add_library(embundle STATIC ${CMAKE_CURRENT_SOURCE_DIR}/bundle_c.c
                              ${CMAKE_CURRENT_SOURCE_DIR}/bundle.cc)
  nanoem_cmake_enable_lto(embundle)
  if(WIN32)
    target_link_libraries(embundle psapi imm32)
  endif()
  set_property(TARGET embundle PROPERTY FOLDER nanoem)
  target_compile_definitions(embundle PRIVATE
                            $<$<BOOL:${WIN32}>:_CRT_SECURE_NO_WARNINGS=1>
                            __STDC_LIMIT_MACROS
                            __STDC_CONSTANT_MACROS
                            __STDC_FORMAT_MACROS)
  target_include_directories(embundle PRIVATE
                            ${BX_COMPAT_INCLUDE_PATH}
                            ${BX_INCLUDE_DIR}
                            ${BX_THIRDPARTY_INCLUDE_DIR}
                            ${BIMG_INCLUDE_DIR}
                            ${GLM_INCLUDE_DIR}
                            ${PROJECT_SOURCE_DIR}/emapp/include
                            ${PROJECT_SOURCE_DIR}/dependencies
                            ${PROJECT_SOURCE_DIR}/dependencies/imgui)
  if(NANOEM_ENABLE_DEBUG_ALLOCATOR)
    target_compile_definitions(embundle PRIVATE NANODXM_ENABLE_DEBUG_ALLOCATOR UNDO_ENABLE_DEBUG_ALLOCATOR)
  endif()
  message(STATUS "[emapp/bundle] setup emapp bundle")
endfunction()

nanoem_emapp_build_embundle()
add_subdirectory(sokol)
