function(nanoem_emapp_build_emarb_translations)
  # translations
  set(input_path "${CMAKE_CURRENT_SOURCE_DIR}/translations.pb")
  set(intermediate_path "${CMAKE_CURRENT_BINARY_DIR}/translations.pb")
  set(output_path "${PROJECT_BINARY_DIR}/emapp/resources/include/emapp/private/resources/pb_translations.h")
  add_custom_command(OUTPUT ${output_path}
                     COMMAND ${CMAKE_COMMAND} ARGS -E copy ${input_path} ${intermediate_path}
                     COMMAND ${CMAKE_COMMAND} ARGS -E make_directory ${PROJECT_BINARY_DIR}/emapp/resources/include/emapp/private/resources
                     COMMAND $<TARGET_FILE:emarb_bin2c> ARGS --input=${intermediate_path} --symbol=pb_translations --compressed=true > ${output_path}
                     MAIN_DEPENDENCY ${input_path}
                     DEPENDS emarb_bin2c
                     VERBATIM)
  add_custom_target(emarb_translations DEPENDS ${output_path})
  set_property(TARGET emarb_translations PROPERTY FOLDER nanoem/resources)
  add_dependencies(emarb_translations emarb_bin2c)
  # YAML2CPP
  if(TARGET emarb_yaml2pb)
    add_custom_command(OUTPUT ${intermediate_path}
                       COMMAND $<TARGET_FILE:emarb_yaml2pb> ARGS ${CMAKE_CURRENT_SOURCE_DIR}/translations.yml ${intermediate_path}
                       COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${intermediate_path} ${input_path}
                       MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/translations.yml
                       DEPENDS emarb_yaml2pb
                       VERBATIM)
    add_custom_target(emarb_translations_intermediate SOURCES ${intermediate_path})
    set_property(TARGET emarb_translations_intermediate PROPERTY FOLDER nanoem/resources)
    add_dependencies(emarb_translations emarb_translations_intermediate)
  endif()
  message(STATUS "[emapp/resources] setup emarb translation")
endfunction()

nanoem_emapp_build_emarb_translations()
