include(CheckCSourceCompiles)

option(NANOEM_ENABLE_MUTABLE "Enable mutable API option." ON)
option(NANOEM_ENABLE_JSON "Enable JSON API with parson." ON)
option(NANOEM_ENABLE_BULLET "Enable linking Bullet Physics option." ON)
option(NANOEM_ENABLE_ICU "Enable linking ICU option." OFF)
option(NANOEM_ENABLE_NMD "Enable building NMD loader" ON)
option(NANOEM_ENABLE_DOCUMENT "Enable PMM API option" ON)
option(NANOEM_ENABLE_OPENMP "Enable building nanoem with OpenMP." OFF)
option(NANOEM_ENABLE_FUZZER "Enable building fuzzing program with libFuzzer" OFF)
mark_as_advanced(NANOEM_ENABLE_FUZZER NANOEM_ENABLE_LEFT_HANDED)

# NANOEM_COMPILE_DEFINITIONS
check_c_source_compiles("int main(void) { if (__builtin_expect(0, 0)) { return 1; } else { return 0; } }" NANOEM_ENABLE_BRANCH_PREDICTION)
if(NANOEM_ENABLE_BRANCH_PREDICTION)
  list(APPEND NANOEM_COMPILE_DEFINITIONS NANOEM_ENABLE_BRANCH_PREDICTION)
  message(STATUS "[nanoem] branch prediction is enabled")
endif()

# JSON extension
if(NANOEM_ENABLE_JSON)
  aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/ext/parson NANOEM_EXT_PARSON_SOURCES)
  list(APPEND NANOEM_COMPILE_DEFINITIONS NANOEM_ENABLE_JSON)
  list(APPEND NANOEM_EXTRA_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ext/json.c ${NANOEM_EXT_PARSON_SOURCES})
  message(STATUS "[nanoem] JSON extension is enabled")
endif()

# mutable extension
if(NANOEM_ENABLE_MUTABLE)
  list(APPEND NANOEM_COMPILE_DEFINITIONS NANOEM_ENABLE_MUTABLE)
  list(APPEND NANOEM_EXTRA_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ext/mutable.c
                                   ${CMAKE_CURRENT_SOURCE_DIR}/ext/converter.c)
  message(STATUS "[nanoem] mutable API extension is enabled")
endif()

# NMD extension
if(NANOEM_ENABLE_NMD)
  add_library(protobuf-c STATIC ${PROJECT_SOURCE_DIR}/dependencies/protobuf-c/protobuf-c/protobuf-c.c)
  set_property(TARGET protobuf-c PROPERTY FOLDER dependencies)
  list(APPEND NANOEM_COMPILE_DEFINITIONS NANOEM_ENABLE_NMD)
  list(APPEND NANOEM_INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/dependencies/protobuf-c)
  list(APPEND NANOEM_EXTRA_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ext/motion.c)
  list(APPEND NANOEM_LINK_LIBRARIES protobuf-c)
  message(STATUS "[nanoem] NMD motion extension is enabled")
endif()

# document extension
if(NANOEM_ENABLE_DOCUMENT)
  list(APPEND NANOEM_COMPILE_DEFINITIONS NANOEM_ENABLE_DOCUMENT)
  list(APPEND NANOEM_EXTRA_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ext/document.c)
  message(STATUS "[nanoem] PMM document extension is enabled")
endif()

# Bullet Physics for optional nanoem prerequisite
if(NANOEM_ENABLE_BULLET)
  set(__CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ${CMAKE_FIND_ROOT_PATH_MODE_INCLUDE})
  set(__CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ${CMAKE_FIND_ROOT_PATH_MODE_LIBRARY})
  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)
  set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
  nanoem_cmake_get_install_path("bullet3" BULLET_BASE_PATH BULLET_INSTALL_PATH_DEBUG BULLET_INSTALL_PATH_RELEASE)
  find_path(BULLET_INCLUDE_DIR NAMES btBulletCollisionCommon.h PATH_SUFFIXES include/bullet include PATHS ${BULLET_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH)
  find_library(BULLET_COLLISION_LIBRARY_DEBUG NAMES BulletCollision PATH_SUFFIXES lib PATHS ${BULLET_INSTALL_PATH_DEBUG} NO_DEFAULT_PATH)
  find_library(BULLET_DYNAMICS_LIBRARY_DEBUG NAMES BulletDynamics PATH_SUFFIXES lib PATHS ${BULLET_INSTALL_PATH_DEBUG} NO_DEFAULT_PATH)
  find_library(BULLET_LINEAR_MATH_LIBRARY_DEBUG NAMES LinearMath PATH_SUFFIXES lib PATHS ${BULLET_INSTALL_PATH_DEBUG} NO_DEFAULT_PATH)
  find_library(BULLET_SOFT_BODY_LIBRARY_DEBUG NAMES BulletSoftBody PATH_SUFFIXES lib PATHS ${BULLET_INSTALL_PATH_DEBUG} NO_DEFAULT_PATH)
  find_library(BULLET_COLLISION_LIBRARY_RELEASE NAMES BulletCollision PATH_SUFFIXES lib PATHS ${BULLET_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH)
  find_library(BULLET_DYNAMICS_LIBRARY_RELEASE NAMES BulletDynamics PATH_SUFFIXES lib PATHS ${BULLET_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH)
  find_library(BULLET_LINEAR_MATH_LIBRARY_RELEASE NAMES LinearMath PATH_SUFFIXES lib PATHS ${BULLET_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH)
  find_library(BULLET_SOFT_BODY_LIBRARY_RELEASE NAMES BulletSoftBody PATH_SUFFIXES lib PATHS ${BULLET_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH)
  mark_as_advanced(BULLET_INCLUDE_DIR BULLET_COLLISION_LIBRARY_DEBUG BULLET_DYNAMICS_LIBRARY_DEBUG BULLET_LINEAR_MATH_LIBRARY_DEBUG
                   BULLET_COLLISION_LIBRARY_RELEASE BULLET_DYNAMICS_LIBRARY_RELEASE BULLET_LINEAR_MATH_LIBRARY_RELEASE
                   BULLET_SOFT_BODY_LIBRARY_DEBUG BULLET_SOFT_BODY_LIBRARY_RELEASE)
  nanoem_cmake_find_glm()
  list(APPEND NANOEM_COMPILE_DEFINITIONS NANOEM_ENABLE_BULLET)
  list(APPEND NANOEM_INCLUDE_DIRECTORIES ${BULLET_INCLUDE_DIR} ${GLM_INCLUDE_DIR})
  list(APPEND NANOEM_EXTRA_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ext/physics_bullet.cc)
  list(APPEND NANOEM_LINK_LIBRARIES $<$<CONFIG:DEBUG>:${BULLET_SOFT_BODY_LIBRARY_DEBUG}>
                                    $<$<CONFIG:DEBUG>:${BULLET_DYNAMICS_LIBRARY_DEBUG}>
                                    $<$<CONFIG:DEBUG>:${BULLET_COLLISION_LIBRARY_DEBUG}>
                                    $<$<CONFIG:DEBUG>:${BULLET_LINEAR_MATH_LIBRARY_DEBUG}>
                                    $<$<NOT:$<CONFIG:DEBUG>>:${BULLET_SOFT_BODY_LIBRARY_RELEASE}>
                                    $<$<NOT:$<CONFIG:DEBUG>>:${BULLET_DYNAMICS_LIBRARY_RELEASE}>
                                    $<$<NOT:$<CONFIG:DEBUG>>:${BULLET_COLLISION_LIBRARY_RELEASE}>
                                    $<$<NOT:$<CONFIG:DEBUG>>:${BULLET_LINEAR_MATH_LIBRARY_RELEASE}>)
  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ${__CMAKE_FIND_ROOT_PATH_MODE_INCLUDE})
  set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ${__CMAKE_FIND_ROOT_PATH_MODE_LIBRARY})
  message(STATUS "[nanoem] bullet is located at ${BULLET_BASE_PATH}")
  message(STATUS "[nanoem] physics simulation using bullet extension is enabled")
endif()

# OpenMP for optional nanoem prerequisite
if(NANOEM_ENABLE_OPENMP)
  find_package(OpenMP REQUIRED)
  if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    list(APPEND NANOEM_COMPILE_DEFINITIONS NANOEM_ENABLE_OPENMP)
  endif()
  message(STATUS "[nanoem] OpenMP extension is enabled")
endif()

# Qt/ICU/OS encoding string conversion for nanoem prerequisite
if(NANOEM_ENABLE_QT)
  list(APPEND NANOEM_COMPILE_DEFINITIONS NANOEM_ENABLE_QT)
  list(APPEND NANOEM_EXTRA_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ext/qt.cc)
  list(APPEND NANOEM_LINK_LIBRARIES Qt5::Core)
  message(STATUS "[nanoem] NMD motion extension is enabled")
elseif(NANOEM_ENABLE_ICU)
  nanoem_cmake_get_install_path("icu" ICU4C_BASE_PATH ICU4C_INSTALL_PATH_DEBUG ICU4C_INSTALL_PATH_RELEASE)
  find_path(ICU4C_INCLUDE_DIR unicode/ucnv.h PATH_SUFFIXES include PATHS ${ICU4C_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH)
  find_library(ICU4C_I18N_LIBRARY_RELEASE NAMES icui18n icuin PATH_SUFFIXES lib PATHS ${ICU4C_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH)
  find_library(ICU4C_UC_LIBRARY_RELEASE NAMES icuuc PATH_SUFFIXES lib PATHS ${ICU4C_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH)
  find_library(ICU4C_DATA_LIBRARY_RELEASE NAMES icudata icudt PATH_SUFFIXES lib PATHS ${ICU4C_INSTALL_PATH_RELEASE} NO_DEFAULT_PATH)
  if(NOT ICU4C_INCLUDE_DIR)
    find_path(ICU4C_INCLUDE_DIR unicode/ucnv.h)
  endif()
  if(NOT ICU4C_I18N_LIBRARY_RELEASE)
    find_library(ICU4C_I18N_LIBRARY_RELEASE NAMES icui18n icuin)
  endif()
  if(NOT ICU4C_UC_LIBRARY_RELEASE)
    find_library(ICU4C_UC_LIBRARY_RELEASE NAMES icuuc)
  endif()
  if(NOT ICU4C_DATA_LIBRARY_RELEASE)
    find_library(ICU4C_DATA_LIBRARY_RELEASE NAMES icudata icudt)
  endif()
  mark_as_advanced(ICU4C_INCLUDE_DIR ICU4C_I18N_LIBRARY_RELEASE ICU4C_UC_LIBRARY_RELEASE ICU4C_DATA_LIBRARY_RELEASE)
  list(APPEND NANOEM_COMPILE_DEFINITIONS NANOEM_ENABLE_ICU)
  list(APPEND NANOEM_INCLUDE_DIRECTORIES ${ICU4C_INCLUDE_DIR})
  list(APPEND NANOEM_EXTRA_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ext/icu.c)
  list(APPEND NANOEM_LINK_LIBRARIES ${ICU4C_I18N_LIBRARY_RELEASE} ${ICU4C_UC_LIBRARY_RELEASE} ${ICU4C_DATA_LIBRARY_RELEASE})
  message(STATUS "[nanoem] ICU unicode string conversion is enabled")
elseif(WIN32)
  list(APPEND NANOEM_COMPILE_DEFINITIONS NANOEM_ENABLE_MBWC WIN32_LEAN_AND_MEAN VC_EXTRALEAN NOMINMAX)
  list(APPEND NANOEM_EXTRA_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ext/mbwc.c)
  message(STATUS "[nanoem] Win32 API unicode string conversion is enabled")
elseif(APPLE)
  list(APPEND NANOEM_COMPILE_DEFINITIONS NANOEM_ENABLE_CFSTRING)
  list(APPEND NANOEM_EXTRA_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ext/cfstring.c)
  list(APPEND NANOEM_LINK_LIBRARIES "-framework CoreFoundation")
  message(STATUS "[nanoem] CoreFoundation unicode string conversion is enabled")
elseif(EMSCRIPTEN)
  list(APPEND NANOEM_EXTRA_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ext/emscripten.cc)
  list(APPEND NANOEM_INCLUDE_DIRECTORIES "${CMAKE_FIND_ROOT_PATH}/include")
  add_definitions(-Drsize_t=size_t)
endif()

# nanoem
add_library(nanoem ${CMAKE_CURRENT_SOURCE_DIR}/nanoem.c
                   ${CMAKE_CURRENT_BINARY_DIR}/version.c
                   ${NANOEM_EXTRA_SOURCES})
nanoem_cmake_enable_lto(nanoem)
set_property(TARGET nanoem PROPERTY FOLDER nanoem)
if(NANOEM_ENABLE_DEBUG_ALLOCATOR)
  target_compile_definitions(nanoem PRIVATE NANOEM_ENABLE_DEBUG_ALLOCATOR)
  message(STATUS "[nanoem] debug allocator is enabled")
endif()

target_compile_definitions(nanoem PRIVATE ${NANOEM_COMPILE_DEFINITIONS})
target_include_directories(nanoem PRIVATE ${NANOEM_INCLUDE_DIRECTORIES})
target_link_libraries(nanoem  ${NANOEM_LINK_LIBRARIES})
if(BUILD_SHARED_LIBS)
  target_compile_definitions(nanoem PRIVATE NANOEM_DLL_EXPORTS)
  install(TARGETS nanoem DESTINATION $<IF:$<BOOL:${WIN32}>,bin,lib>)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/nanoem.h
                DESTINATION include/nanoem)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/document.h
                ${CMAKE_CURRENT_SOURCE_DIR}/ext/motion.h
                ${CMAKE_CURRENT_SOURCE_DIR}/ext/mutable.h
                ${CMAKE_CURRENT_SOURCE_DIR}/ext/physics.h
                DESTINATION include/nanoem/ext)
  message(STATUS "[nanoem] shared library configuration is enabled")
endif()

if(NANOEM_ENABLE_FUZZER)
  set(FUZZ_COMPILE_OPTIONS -fsanitize=fuzzer,address)
  set(FUZZ_LINK_FLAGS -fsanitize=fuzzer,address)
  add_library(nanoem_fuzz ${CMAKE_CURRENT_SOURCE_DIR}/nanoem.c
                          ${CMAKE_CURRENT_BINARY_DIR}/version.c
                          ${NANOEM_EXTRA_SOURCES})
  function(nanoem_build_fuzz_program name source)
    add_executable(${name} ${source})
    target_compile_definitions(${name} PRIVATE ${NANOEM_COMPILE_DEFINITIONS})
    target_compile_options(${name} PRIVATE ${FUZZ_COMPILE_OPTIONS})
    target_link_options(${name} PRIVATE ${FUZZ_LINK_FLAGS})
    target_include_directories(${name} PRIVATE ${NANOEM_INCLUDE_DIRECTORIES})
    target_link_libraries(${name} nanoem_fuzz ${NANOEM_LINK_LIBRARIES})
  endfunction()
  target_compile_definitions(nanoem_fuzz PRIVATE ${NANOEM_COMPILE_DEFINITIONS})
  target_compile_options(nanoem_fuzz PRIVATE ${FUZZ_COMPILE_OPTIONS})
  target_include_directories( nanoem_fuzz PRIVATE ${NANOEM_INCLUDE_DIRECTORIES})
  target_link_options(nanoem_fuzz PRIVATE ${FUZZ_LINK_FLAGS})
  target_link_libraries(nanoem_fuzz ${NANOEM_LINK_LIBRARIES})
  nanoem_build_fuzz_program(nanoem_fuzz_document ${CMAKE_CURRENT_SOURCE_DIR}/fuzz/fuzz_document.cc)
  nanoem_build_fuzz_program(nanoem_fuzz_model_pmd ${CMAKE_CURRENT_SOURCE_DIR}/fuzz/fuzz_model_pmd.cc)
  nanoem_build_fuzz_program(nanoem_fuzz_model_pmx ${CMAKE_CURRENT_SOURCE_DIR}/fuzz/fuzz_model_pmx.cc)
  nanoem_build_fuzz_program(nanoem_fuzz_motion_vmd ${CMAKE_CURRENT_SOURCE_DIR}/fuzz/fuzz_motion_vmd.cc)
  nanoem_build_fuzz_program(nanoem_fuzz_motion_nmd ${CMAKE_CURRENT_SOURCE_DIR}/fuzz/fuzz_motion_nmd.cc)
  message(STATUS "[nanoem] fuzzer extension is enabled")
endif()

# version.c
configure_file(version.c.in ${CMAKE_CURRENT_BINARY_DIR}/version.c @ONLY)
