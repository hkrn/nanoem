option(NANOEM_ENABLE_SANDBOX "Enable building sandbox program option." OFF)

function(nanoem_sandbox_build_applications)
  nanoem_cmake_find_glm()
  get_property(_compile_definitions TARGET nanoem PROPERTY COMPILE_DEFINITIONS)
  get_property(_include_directories TARGET nanoem PROPERTY INCLUDE_DIRECTORIES)
  get_property(_link_libraries TARGET nanoem PROPERTY LINK_LIBRARIES)
  set(_name nanoem_sandbox_euler)
  add_executable(${_name} ${CMAKE_CURRENT_SOURCE_DIR}/euler.cc)
  set_property(TARGET ${_name} PROPERTY FOLDER sandbox)
  set_property(TARGET ${_name} APPEND PROPERTY COMPILE_DEFINITIONS ${_compile_definitions} $<$<BOOL:${WIN32}>:_CRT_SECURE_NO_WARNINGS=1>)
  set_property(TARGET ${_name} APPEND PROPERTY INCLUDE_DIRECTORIES ${_include_directories} ${GLM_INCLUDE_DIR})
  set(_name nanoem_sandbox_plugin)
  if(NANOEM_ENABLE_DOCUMENT)
    set(_name nanoem_sandbox_document)
    add_executable(${_name} ${CMAKE_CURRENT_SOURCE_DIR}/document.cc)
    target_link_libraries(${_name} nanoem ${_link_libraries})
    set_property(TARGET ${_name} PROPERTY FOLDER sandbox)
    set_property(TARGET ${_name} APPEND PROPERTY COMPILE_DEFINITIONS ${_compile_definitions} $<$<BOOL:${WIN32}>:_CRT_SECURE_NO_WARNINGS=1>)
    set_property(TARGET ${_name} APPEND PROPERTY INCLUDE_DIRECTORIES ${_include_directories})
  endif()
  if(NANOEM_ENABLE_MUTABLE)
    set(_name nanoem_sandbox_convert)
    add_executable(${_name} ${CMAKE_CURRENT_SOURCE_DIR}/convert.cc ${PROJECT_SOURCE_DIR}/dependencies/nanodxm/nanodxm.c)
    set_property(TARGET ${_name} PROPERTY FOLDER sandbox)
    set_property(TARGET ${_name} APPEND PROPERTY COMPILE_DEFINITIONS ${_compile_definitions} $<$<BOOL:${WIN32}>:_CRT_SECURE_NO_WARNINGS=1>)
    set_property(TARGET ${_name} APPEND PROPERTY INCLUDE_DIRECTORIES ${_include_directories})
    target_link_libraries(${_name} nanoem ${_link_libraries})
    set(_name nanoem_sandbox_motion)
    add_executable(${_name} ${CMAKE_CURRENT_SOURCE_DIR}/motion.cc)
    set_property(TARGET ${_name} PROPERTY FOLDER sandbox)
    set_property(TARGET ${_name} APPEND PROPERTY COMPILE_DEFINITIONS ${_compile_definitions} $<$<BOOL:${WIN32}>:_CRT_SECURE_NO_WARNINGS=1>)
    set_property(TARGET ${_name} APPEND PROPERTY INCLUDE_DIRECTORIES ${_include_directories} ${GLM_INCLUDE_DIR})
    target_link_libraries(${_name} nanoem ${_link_libraries})
    set(_name nanoem_sandbox_model)
    add_executable(${_name} ${CMAKE_CURRENT_SOURCE_DIR}/model.cc)
    set_property(TARGET ${_name} PROPERTY FOLDER sandbox)
    set_property(TARGET ${_name} APPEND PROPERTY COMPILE_DEFINITIONS ${_compile_definitions} $<$<BOOL:${WIN32}>:_CRT_SECURE_NO_WARNINGS=1>)
    set_property(TARGET ${_name} APPEND PROPERTY INCLUDE_DIRECTORIES ${_include_directories} ${GLM_INCLUDE_DIR})
    target_link_libraries(${_name} nanoem ${_link_libraries})
    set(_name nanoem_sandbox_strip)
    add_executable(${_name} ${CMAKE_CURRENT_SOURCE_DIR}/strip.cc)
    set_property(TARGET ${_name} PROPERTY FOLDER sandbox)
    set_property(TARGET ${_name} APPEND PROPERTY COMPILE_DEFINITIONS ${_compile_definitions} $<$<BOOL:${WIN32}>:_CRT_SECURE_NO_WARNINGS=1>)
    set_property(TARGET ${_name} APPEND PROPERTY INCLUDE_DIRECTORIES ${_include_directories} ${GLM_INCLUDE_DIR})
    target_link_libraries(${_name} nanoem ${_link_libraries})
    set(_name nanoem_sandbox_softbody)
    add_executable(${_name} ${CMAKE_CURRENT_SOURCE_DIR}/softbody.cc)
    set_property(TARGET ${_name} PROPERTY FOLDER sandbox)
    set_property(TARGET ${_name} APPEND PROPERTY COMPILE_DEFINITIONS ${_compile_definitions} $<$<BOOL:${WIN32}>:_CRT_SECURE_NO_WARNINGS=1>)
    set_property(TARGET ${_name} APPEND PROPERTY INCLUDE_DIRECTORIES ${_include_directories} ${GLM_INCLUDE_DIR})
    target_link_libraries(${_name} nanoem ${_link_libraries})
  endif()
  if(NANOEM_ENABLE_DOCUMENT AND NANOEM_ENABLE_MUTABLE)
    set(_name nanoem_sandbox_validate)
    add_executable(${_name} ${CMAKE_CURRENT_SOURCE_DIR}/validate.cc ${PROJECT_SOURCE_DIR}/dependencies/nanodxm/nanodxm.c)
    set_property(TARGET ${_name} PROPERTY FOLDER sandbox)
    set_property(TARGET ${_name} APPEND PROPERTY COMPILE_DEFINITIONS ${_compile_definitions} $<$<BOOL:${WIN32}>:_CRT_SECURE_NO_WARNINGS=1>)
    set_property(TARGET ${_name} APPEND PROPERTY INCLUDE_DIRECTORIES ${_include_directories} ${PROJECT_SOURCE_DIR}/dependencies)
    target_link_libraries(${_name} nanoem ${_link_libraries})
  endif()
  add_executable(nanoem_sandbox_plugin_audio ${CMAKE_CURRENT_SOURCE_DIR}/plugin_audio.cc)
  set_property(TARGET nanoem_sandbox_plugin_audio PROPERTY FOLDER sandbox)
  nanoem_emapp_link_executable(nanoem_sandbox_plugin_audio)
  add_executable(nanoem_sandbox_plugin_model ${CMAKE_CURRENT_SOURCE_DIR}/plugin_model.cc)
  set_property(TARGET nanoem_sandbox_plugin_model PROPERTY FOLDER sandbox)
  nanoem_emapp_link_executable(nanoem_sandbox_plugin_model)
  add_executable(nanoem_sandbox_plugin_motion ${CMAKE_CURRENT_SOURCE_DIR}/plugin_motion.cc)
  set_property(TARGET nanoem_sandbox_plugin_motion PROPERTY FOLDER sandbox)
  nanoem_emapp_link_executable(nanoem_sandbox_plugin_motion)
  add_executable(nanoem_sandbox_plugin_video ${CMAKE_CURRENT_SOURCE_DIR}/plugin_video.cc)
  set_property(TARGET nanoem_sandbox_plugin_video PROPERTY FOLDER sandbox)
  nanoem_emapp_link_executable(nanoem_sandbox_plugin_video)
  add_executable(nanoem_sandbox_project ${CMAKE_CURRENT_SOURCE_DIR}/project.cc)
  set_property(TARGET nanoem_sandbox_project PROPERTY FOLDER sandbox)
  nanoem_emapp_link_executable(nanoem_sandbox_project)
  add_executable(nanoem_sandbox_transformer ${CMAKE_CURRENT_SOURCE_DIR}/transformer.cc)
  set_property(TARGET nanoem_sandbox_transformer PROPERTY FOLDER sandbox)
  nanoem_emapp_link_executable(nanoem_sandbox_transformer)
  add_executable(nanoem_sandbox_image ${CMAKE_CURRENT_SOURCE_DIR}/image.cc)
  set_property(TARGET nanoem_sandbox_image PROPERTY FOLDER sandbox)
  nanoem_emapp_link_executable(nanoem_sandbox_image)
  find_library(XLSXWRITER_LIBRARY xlsxwriter)
  mark_as_advanced(XLSXWRITER_LIBRARY)
  find_path(XLSXWRITER_INCLUDE_DIR xlsxwriter.h PATH_SUFFIXES include)
  mark_as_advanced(XLSXWRITER_INCLUDE_DIR)
  if(XLSXWRITER_LIBRARY AND XLSXWRITER_INCLUDE_DIR)
    set(_name nanoem_sandbox_xlsx)
    add_executable(${_name} ${CMAKE_CURRENT_SOURCE_DIR}/xlsx.cc)
    set_property(TARGET ${_name} PROPERTY FOLDER sandbox)
    set_property(TARGET ${_name} APPEND PROPERTY COMPILE_DEFINITIONS ${_compile_definitions} $<$<BOOL:${WIN32}>:_CRT_SECURE_NO_WARNINGS=1>)
    set_property(TARGET ${_name} APPEND PROPERTY INCLUDE_DIRECTORIES ${XLSXWRITER_INCLUDE_DIR} ${_include_directories})
    target_link_libraries(${_name} nanoem ${XLSXWRITER_LIBRARY} ${_link_libraries})
  endif()
  if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
    find_library(DL_LIBRARY dl)
    mark_as_advanced(DL_LIBRARY)
    target_link_libraries(nanoem_sandbox_plugin_audio ${DL_LIBRARY})
    target_link_libraries(nanoem_sandbox_plugin_model ${DL_LIBRARY})
    target_link_libraries(nanoem_sandbox_plugin_motion ${DL_LIBRARY})
    target_link_libraries(nanoem_sandbox_plugin_video ${DL_LIBRARY})
  endif()
endfunction()

if(NANOEM_ENABLE_SANDBOX)
  nanoem_sandbox_build_applications()
endif()
