function(nanoem_emapp_build_emarb_font)
  set(intermediate_path_prefix "${CMAKE_CURRENT_BINARY_DIR}/intermediate")
  set(output_path_prefix "${PROJECT_BINARY_DIR}/emapp/resources/include/emapp/private/resources")
  # font_awesome (v4.7)
  add_custom_command(OUTPUT ${output_path_prefix}/font_awesome.h
                     COMMAND ${CMAKE_COMMAND} ARGS -E copy ${CMAKE_CURRENT_SOURCE_DIR}/font_awesome.ttf ${intermediate_path_prefix}/font_awesome.ttf
                     COMMAND ${CMAKE_COMMAND} -E env ASAN_OPTIONS="detect_leaks=0"
                             $<TARGET_FILE:emarb_bin2c> --input=${intermediate_path_prefix}/font_awesome.ttf --symbol=font_awesome_ttf --compressed=true
                             > ${output_path_prefix}/font_awesome.h
                     MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/font_awesome.ttf
                     VERBATIM)
  # genei_gothic_p_semi_bold (v1.1)
  add_custom_command(OUTPUT ${output_path_prefix}/genei_gothic_p_semi_bold.h
                     COMMAND ${CMAKE_COMMAND} ARGS -E copy ${CMAKE_CURRENT_SOURCE_DIR}/genei_gothic_p_semi_bold.otf ${intermediate_path_prefix}/genei_gothic_p_semi_bold.otf
                     COMMAND ${CMAKE_COMMAND} -E env ASAN_OPTIONS="detect_leaks=0"
                             $<TARGET_FILE:emarb_bin2c> --input=${intermediate_path_prefix}/genei_gothic_p_semi_bold.otf --symbol=genei_gothic_p_semi_bold_otf --compressed=true
                             > ${output_path_prefix}/genei_gothic_p_semi_bold.h
                     MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/genei_gothic_p_semi_bold.otf
                     VERBATIM)
  add_custom_target(emarb_fonts DEPENDS ${output_path_prefix}/font_awesome.h ${output_path_prefix}/genei_gothic_p_semi_bold.h)
  set_property(TARGET emarb_fonts PROPERTY FOLDER nanoem/resources)
  add_dependencies(emarb_fonts emarb_bin2c)
  message(STATUS "[emapp/resources] setup emarb font")
endfunction()

nanoem_emapp_build_emarb_font()
