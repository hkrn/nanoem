
function(nanoem_emapp_build_emarb_protoc)
  # protoc
  find_program(PROTOC_PATH protoc-c)
  mark_as_advanced(PROTOC_PATH)
  if(PROTOC_PATH)
    file(GLOB PROTO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.proto)
    set(intermediate_path_prefix "${CMAKE_CURRENT_BINARY_DIR}/intermediate")
    set(output_path_prefix "${PROJECT_SOURCE_DIR}/emapp/src/protoc")
    foreach(item ${PROTO_FILES})
      get_filename_component(filename ${item} NAME)
      get_filename_component(filename_without_extension ${item} NAME_WE)
      set(intermediate_source_path ${intermediate_path_prefix}/${filename_without_extension}.pb-c.c)
      set(intermediate_header_path ${intermediate_path_prefix}/${filename_without_extension}.pb-c.h)
      get_filename_component(intermediate_dir ${intermediate_source_path} DIRECTORY)
      set(output_source_path ${output_path_prefix}/${filename_without_extension}.pb-c.c)
      set(output_header_path ${output_path_prefix}/${filename_without_extension}.pb-c.h)
      get_filename_component(output_dir ${output_source_path} DIRECTORY)
      add_custom_command(OUTPUT ${intermediate_source_path} ${intermediate_header_path}
                         COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${intermediate_dir}
                         COMMAND ${PROTOC_PATH} ARGS --c_out=${intermediate_dir} ${filename}
                         COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${intermediate_source_path} ${output_source_path}
                         COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${intermediate_header_path} ${output_header_path}
                         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                         MAIN_DEPENDENCY ${item}
                         VERBATIM)
      list(APPEND generated_sources ${intermediate_source_path})
      list(APPEND generated_sources ${intermediate_header_path})
    endforeach()
    add_custom_target(emarb_protoc SOURCES ${generated_sources})
    set_property(TARGET emarb_protoc PROPERTY FOLDER nanoem/resources)
  endif()
  message(STATUS "[emapp/resources] setup emarb protobuf")
endfunction()

nanoem_emapp_build_emarb_protoc()
