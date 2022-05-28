function(nanoem_emapp_build_emarb_toon)
  set(intermediate_path_prefix "${CMAKE_CURRENT_BINARY_DIR}/intermediate")
  set(output_path_prefix "${PROJECT_BINARY_DIR}/emapp/resources/include/emapp/private/resources/toons")
  file(GLOB TOON_IMAGES ${CMAKE_CURRENT_SOURCE_DIR}/*.png)
  foreach(_item ${TOON_IMAGES})
    get_filename_component(_filename ${_item} NAME)
    get_filename_component(_name ${_item} NAME_WE)
    add_custom_command(OUTPUT ${output_path_prefix}/${_name}.h
                       COMMAND ${CMAKE_COMMAND} -E make_directory ${output_path_prefix}
                       COMMAND ${CMAKE_COMMAND} ARGS -E copy ${_item} ${intermediate_path_prefix}/${_filename}
                       COMMAND ${CMAKE_COMMAND} -E env ASAN_OPTIONS="detect_leaks=0"
                               $<TARGET_FILE:emarb_bin2c> --input=${intermediate_path_prefix}/${_filename} --symbol=${_name} --compressed=false
                               > ${output_path_prefix}/${_name}.h
                       MAIN_DEPENDENCY ${_item}
                       VERBATIM)
    list(APPEND _sources ${output_path_prefix}/${_name}.h)
  endforeach()
  add_custom_target(emarb_toon DEPENDS ${_sources})
  set_property(TARGET emarb_toon PROPERTY FOLDER nanoem/resources)
  add_dependencies(emarb_toon emarb_bin2c)
  message(STATUS "[emapp/resources] setup emarb toon")
endfunction()

nanoem_emapp_build_emarb_toon()
