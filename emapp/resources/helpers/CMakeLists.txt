if(DEFINED CMAKE_TOOLCHAIN_FILE OR DEFINED ENV{NANOEM_TOOLCHAIN_BIN2C_DIR})
  find_program(NANOEM_TOOLCHAIN_BIN2C_LOCATION NAMES bin2c PATHS $ENV{NANOEM_TOOLCHAIN_BIN2C_DIR})
  mark_as_advanced(NANOEM_TOOLCHAIN_BIN2C_LOCATION)
  add_executable(emarb_bin2c IMPORTED GLOBAL)
  set_target_properties(emarb_bin2c PROPERTIES IMPORTED_LOCATION ${NANOEM_TOOLCHAIN_BIN2C_LOCATION})
else()
  add_executable(emarb_bin2c ${CMAKE_CURRENT_SOURCE_DIR}/bin2c.c)
  set_property(TARGET emarb_bin2c PROPERTY FOLDER nanoem/resources)
  set_property(TARGET emarb_bin2c PROPERTY OUTPUT_NAME bin2c)
  target_include_directories(emarb_bin2c PRIVATE ${PROJECT_SOURCE_DIR}/dependencies)
  target_link_libraries(emarb_bin2c lz4)
  if(UNIX)
    target_link_libraries(emarb_bin2c m) # libmath
  endif()
endif()

nanoem_cmake_get_install_path("yaml-cpp" YAMLCPP_BASE_PATH YAML_INSTALL_PATH_DEBUG YAMLCPP_INSTALL_PATH_RELEASE)
find_library(YAMLCPP_LIBRARY_DEBUG NAMES yaml-cppd yaml-cpp yaml-cppmtd PATH_SUFFIXES lib PATHS ${YAML_INSTALL_PATH_DEBUG} NO_DEFAULT_PATH)
find_library(YAMLCPP_LIBRARY_RELEASE NAMES yaml-cpp yaml-cppmt PATH_SUFFIXES lib PATHS ${YAMLCPP_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH)
find_path(YAMLCPP_INCLUDE_DIR NAMES yaml-cpp/yaml.h PATH_SUFFIXES include PATHS ${YAMLCPP_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH)
mark_as_advanced(YAMLCPP_LIBRARY_DEBUG YAMLCPP_LIBRARY_RELEASE YAMLCPP_INCLUDE_DIR)
message(STATUS "[emapp/resources] yaml-cpp is located at ${YAMLCPP_BASE_PATH}")
if(DEFINED CMAKE_TOOLCHAIN_FILE OR DEFINED ENV{NANOEM_TOOLCHAIN_YAML2PB_DIR})
  find_program(NANOEM_TOOLCHAIN_YAML2PB_LOCATION NAMES yaml2pb PATHS $ENV{NANOEM_TOOLCHAIN_YAML2PB_DIR})
  mark_as_advanced(NANOEM_TOOLCHAIN_YAML2PB_LOCATION)
  add_executable(emarb_yaml2pb IMPORTED GLOBAL)
  set_target_properties(emarb_yaml2pb PROPERTIES IMPORTED_LOCATION ${NANOEM_TOOLCHAIN_YAML2PB_LOCATION})
elseif(YAMLCPP_LIBRARY_DEBUG AND YAMLCPP_LIBRARY_RELEASE AND YAMLCPP_INCLUDE_DIR)
  add_executable(emarb_yaml2pb ${CMAKE_CURRENT_SOURCE_DIR}/yaml2pb.cc
                               ${CMAKE_CURRENT_SOURCE_DIR}/../../src/protoc/common.pb-c.c
                               ${CMAKE_CURRENT_SOURCE_DIR}/../../src/protoc/translation.pb-c.c)
  target_link_libraries(emarb_yaml2pb protobuf-c)
  set_property(TARGET emarb_yaml2pb PROPERTY FOLDER nanoem/resources)
  set_property(TARGET emarb_yaml2pb PROPERTY OUTPUT_NAME yaml2pb)
  target_include_directories(emarb_yaml2pb PRIVATE
                            ${YAMLCPP_INCLUDE_DIR}
                            ${CMAKE_CURRENT_SOURCE_DIR}/../../src
                            ${YAMLCPP_INCLUDE_DIR}/include
                            ${PROJECT_SOURCE_DIR}/dependencies
                            ${PROJECT_SOURCE_DIR}/dependencies/protobuf-c)
  target_link_libraries(emarb_yaml2pb lz4 optimized ${YAMLCPP_LIBRARY_RELEASE} debug ${YAMLCPP_LIBRARY_DEBUG})
  message(STATUS "[emapp/resources] setup emarb automatic generation of translation binary")
endif()
