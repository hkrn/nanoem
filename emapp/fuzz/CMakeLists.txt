set(FUZZ_COMPILE_OPTIONS -fsanitize=fuzzer,address)
set(FUZZ_LINK_FLAGS -fsanitize=fuzzer,address)

function(nanoem_build_fuzz_program name source)
  get_property(_include_directories_emapp_fuzz TARGET emapp_fuzz PROPERTY INCLUDE_DIRECTORIES)
  get_property(_link_libraries_nanoem TARGET nanoem PROPERTY LINK_LIBRARIES)
  add_executable(${name} ${source})
  target_compile_options(${name} PRIVATE ${FUZZ_COMPILE_OPTIONS})
  target_link_options(${name} PRIVATE ${FUZZ_LINK_FLAGS})
  target_include_directories(${name} PRIVATE ${_include_directories_emapp_fuzz})
  target_link_libraries(${name} emapp_fuzz emapp_protoc_fuzz emarb_fuzz embundle_fuzz bx lz4 nanoem ${_link_libraries_nanoem})
  target_link_libraries(${name} optimized ${MINIZIP_LIBRARY_RELEASE}
                                optimized ${ZLIB_LIBRARY_RELEASE}
                                debug ${MINIZIP_LIBRARY_DEBUG}
                                debug ${ZLIB_LIBRARY_DEBUG})
  if(NANOEM_ENABLE_NANOMSG)
    target_link_libraries(${name} optimized ${NANOMSG_LIBRARY_RELEASE}
                                  debug ${NANOMSG_LIBRARY_DEBUG})
  endif()
  if(NANOEM_ENABLE_TBB)
    target_link_libraries(${name} optimized ${TBB_LIBRARY_RELEASE}
                                  debug ${TBB_LIBRARY_DEBUG})
  endif()
endfunction()

nanoem_emapp_build_embundle("_fuzz")
nanoem_emapp_build_emarb("_fuzz")
nanoem_emapp_build_emapp("_fuzz")
target_compile_options(emapp_fuzz PRIVATE ${FUZZ_COMPILE_OPTIONS})
target_compile_options(emarb_fuzz PRIVATE ${FUZZ_COMPILE_OPTIONS})
target_compile_options(embundle_fuzz PRIVATE ${FUZZ_COMPILE_OPTIONS})
target_link_options(emapp_fuzz PRIVATE ${FUZZ_LINK_FLAGS})
target_link_options(emarb_fuzz PRIVATE ${FUZZ_LINK_FLAGS})
target_link_options(embundle_fuzz PRIVATE ${FUZZ_LINK_FLAGS})

nanoem_build_fuzz_program(nanoem_fuzz_image_apng ${CMAKE_CURRENT_SOURCE_DIR}/fuzz_image_apng.cc)
nanoem_build_fuzz_program(nanoem_fuzz_image_dds ${CMAKE_CURRENT_SOURCE_DIR}/fuzz_image_dds.cc)
nanoem_build_fuzz_program(nanoem_fuzz_image_pfm ${CMAKE_CURRENT_SOURCE_DIR}/fuzz_image_pfm.cc)
