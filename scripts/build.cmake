cmake_minimum_required(VERSION 3.5)

function(rewrite_ninja_ub_workaround _build_path)
  if(EXISTS ${_build_path}/build.ninja)
    file(STRINGS ${_build_path}/build.ninja input_ninja NEWLINE_CONSUME)
    string(REPLACE "arm64\;-arch" "arm64 -arch" output_ninja "${input_ninja}")
    file(WRITE ${_build_path}/build.ninja ${output_ninja})
  endif()
endfunction()

function(rewrite_cmake_cache_for_win32 _build_path)
  if(WIN32)
    file(STRINGS ${_build_path}/CMakeCache.txt input_cmake_cache NEWLINE_CONSUME)
    # quote input_cmake_cache to prevent trailing semicolons
    string(REPLACE "/MD" "/MT" interm_cmake_cache "${input_cmake_cache}")
    string(REPLACE "-D_DLL" "" output_cmake_cache "${interm_cmake_cache}")
    file(WRITE ${_build_path}/CMakeCache.txt ${output_cmake_cache})
    execute_process(COMMAND ${CMAKE_COMMAND} -E chdir ${_build_path} ${CMAKE_COMMAND} ${_source_path})
  endif()
endfunction()

function(rewrite_cmake_cache_type _build_path)
  file(STRINGS ${_build_path}/CMakeCache.txt input_cmake_cache NEWLINE_CONSUME)
  # quote input_cmake_cache to prevent trailing semicolons
  string(REPLACE ":UNINITIALIZED=" ":STRING=" output_cmake_cache "${input_cmake_cache}")
  file(WRITE ${_build_path}/CMakeCache.txt ${output_cmake_cache})
  execute_process(COMMAND ${CMAKE_COMMAND} -E chdir ${_build_path} ${CMAKE_COMMAND} ${_source_path})
endfunction()

function(rewrite_cmake_cache _build_path)
  rewrite_cmake_cache_for_win32(${_build_path})
  rewrite_cmake_cache_type(${_build_path})
endfunction()

function(execute_build _build_path)
  execute_process(COMMAND ${CMAKE_COMMAND} --build ${_build_path}
                                           --clean-first
                                           --config ${_cmake_build_type}
                                           --target install
                                           RESULT_VARIABLE exit_code)
  if(NOT ${exit_code} EQUAL 0)
    message(FATAL_ERROR "Building failed with ${exit_code}: ${_build_path}")
  endif()
endfunction()

function(execute_build_no_failure _build_path)
  execute_process(COMMAND ${CMAKE_COMMAND} --build ${_build_path}
                                            --clean-first
                                            --config ${_cmake_build_type}
                                            --target install)
endfunction()

function(compile_zlib _cmake_build_type _generator _toolset_option _arch_option _triple_path)
  set(_source_path ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/zlib)
  set(_build_path ${base_build_path}/zlib/out/${_triple_path})
  file(MAKE_DIRECTORY ${_build_path})
  execute_process(COMMAND ${CMAKE_COMMAND} -E chdir ${_build_path}
                                           ${CMAKE_COMMAND}
                                           ${global_cmake_flags}
                                           -DCMAKE_BUILD_TYPE=${_cmake_build_type}
                                           -DCMAKE_CONFIGURATION_TYPES=${_cmake_build_type}
                                           -DCMAKE_INSTALL_PREFIX=${_build_path}/install-root
                                           -G "${_generator}" ${_arch_option} ${_toolset_option} ${_source_path})
  rewrite_cmake_cache(${_build_path})
  execute_build(${_build_path})
endfunction()

function(compile_libsoundio _cmake_build_type _generator _toolset_option _arch_option _triple_path)
  set(_source_path ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/libsoundio)
  if(EXISTS ${_source_path})
    set(_build_path ${base_build_path}/libsoundio/out/${_triple_path})
    file(MAKE_DIRECTORY ${_build_path})
    execute_process(COMMAND ${CMAKE_COMMAND} -E chdir ${_build_path}
                                             ${CMAKE_COMMAND}
                                             ${global_cmake_flags}
                                             -DCMAKE_BUILD_TYPE=${_cmake_build_type}
                                             -DCMAKE_CONFIGURATION_TYPES=${_cmake_build_type}
                                             -DCMAKE_INSTALL_PREFIX=${_build_path}/install-root
                                             -DBUILD_DYNAMIC_LIBS=OFF
                                             -DBUILD_EXAMPLE_PROGRAMS=OFF
                                             -DBUILD_TESTS=OFF
                                             -G "${_generator}" ${_arch_option} ${_toolset_option} ${_source_path})
    rewrite_cmake_cache(${_build_path})
    execute_build(${_build_path})
  endif()
endfunction()

function(compile_minizip _cmake_build_type _generator _toolset_option _arch_option _triple_path)
  set(_source_path ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/minizip)
  set(_build_path ${base_build_path}/minizip/out/${_triple_path})
  file(MAKE_DIRECTORY ${_build_path})
  execute_process(COMMAND ${CMAKE_COMMAND} -E chdir ${_build_path}
                                           ${CMAKE_COMMAND}
                                           ${global_cmake_flags}
                                           -DCMAKE_BUILD_TYPE=${_cmake_build_type}
                                           -DCMAKE_CONFIGURATION_TYPES=${_cmake_build_type}
                                           -DCMAKE_INSTALL_PREFIX=${_build_path}/install-root
                                           -DMZ_BZIP2=OFF
                                           -DMZ_COMPAT=OFF
                                           -DMZ_LZMA=OFF
                                           -DMZ_PKCRYPT=OFF
                                           -DMZ_WZAES=OFF
                                           -DSKIP_INSTALL_FILES=ON
                                           -DZLIB_ROOT=${base_build_path}/zlib/out/${_triple_path}/install-root
                                           -G "${_generator}" ${_arch_option} ${_toolset_option} ${_source_path})
  rewrite_cmake_cache(${_build_path})
  execute_build(${_build_path})
endfunction()

function(compile_bullet _cmake_build_type _generator _toolset_option _arch_option _triple_path)
  set(_source_path ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/bullet3)
  set(_build_path ${base_build_path}/bullet3/out/${_triple_path})
  file(MAKE_DIRECTORY ${_build_path})
  execute_process(COMMAND ${CMAKE_COMMAND} -E chdir ${_build_path}
                                           ${CMAKE_COMMAND}
                                           ${global_cmake_flags}
                                           -DBUILD_DEMOS=OFF
                                           -DBUILD_EXTRAS=OFF
                                           -DINSTALL_EXTRA_LIBS=OFF
                                           -DINSTALL_LIBS=ON
                                           -DUSE_DOUBLE_PRECISION=OFF
                                           -DUSE_GLUT=OFF
                                           -DUSE_GRAPHICAL_BENCHMARK=OFF
                                           -DUSE_MSVC_INCREMENTAL_LINKING=OFF
                                           -DUSE_MSVC_RUNTIME_LIBRARY_DLL=OFF
                                           -DCMAKE_BUILD_TYPE=${_cmake_build_type}
                                           -DCMAKE_CONFIGURATION_TYPES=${_cmake_build_type}
                                           -DCMAKE_INSTALL_PREFIX=${_build_path}/install-root
                                           -G "${_generator}" ${_arch_option} ${_toolset_option} ${_source_path})
  rewrite_cmake_cache(${_build_path})
  execute_build(${_build_path})
endfunction()

function(compile_glslang _cmake_build_type _generator _toolset_option _arch_option _triple_path)
  set(_source_path ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glslang)
  set(_build_path ${base_build_path}/glslang/out/${_triple_path})
  file(MAKE_DIRECTORY ${_build_path})
  set(_extra_options "")
  execute_process(COMMAND ${CMAKE_COMMAND} -E chdir ${_build_path}
                                           ${CMAKE_COMMAND}
                                           ${global_cmake_flags}
                                           -DLLVM_USE_CRT_DEBUG=MTd
                                           -DLLVM_USE_CRT_MINSIZEREL=MT
                                           -DLLVM_USE_CRT_RELEASE=MT
                                           -DLLVM_USE_CRT_RELWITHDEBINFO=MT
                                           -DBUILD_TESTING=OFF
                                           -DENABLE_AMD_EXTENSIONS=OFF
                                           -DENABLE_GLSLANG_BINARIES=ON
                                           -DENABLE_HLSL=ON
                                           -DENABLE_NV_EXTENSIONS=OFF
                                           -DENABLE_OPT=ON
                                           -DENABLE_SPVREMAPPER=ON
                                           -DCMAKE_BUILD_TYPE=${_cmake_build_type}
                                           -DCMAKE_CONFIGURATION_TYPES=${_cmake_build_type}
                                           -DCMAKE_INSTALL_PREFIX=${_build_path}/install-root
                                           ${_extra_options}
                                           -G "${_generator}" ${_arch_option} ${_toolset_option} ${_source_path})
  rewrite_cmake_cache(${_build_path})
  execute_build(${_build_path})
endfunction()

function(compile_nanomsg _cmake_build_type _generator _toolset_option _arch_option _triple_path)
  set(_source_path ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/nanomsg)
  set(_build_path ${base_build_path}/nanomsg/out/${_triple_path})
  file(MAKE_DIRECTORY ${_build_path})
  execute_process(COMMAND ${CMAKE_COMMAND} -E chdir ${_build_path}
                                           ${CMAKE_COMMAND}
                                           ${global_cmake_flags}
                                           -DNN_STATIC_LIB=ON
                                           -DNN_ENABLE_GETADDRINFO_A=OFF
                                           -DNN_ENABLE_DOC=OFF
                                           -DNN_TESTS=OFF
                                           -DNN_TOOLS=OFF
                                           -DCMAKE_BUILD_TYPE=${_cmake_build_type}
                                           -DCMAKE_CONFIGURATION_TYPES=${_cmake_build_type}
                                           -DCMAKE_INSTALL_LIBDIR=lib
                                           -DCMAKE_INSTALL_PREFIX=${_build_path}/install-root
                                           -G "${_generator}" ${_arch_option} ${_toolset_option} ${_source_path})
  rewrite_cmake_cache(${_build_path})
  execute_build(${_build_path})
endfunction()

function(compile_tbb _cmake_build_type _generator _toolset_option _arch_option _triple_path)
  set(_source_path ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/tbb)
  set(_build_path ${base_build_path}/tbb/out/${_triple_path})
  file(MAKE_DIRECTORY ${_build_path})
  execute_process(COMMAND ${CMAKE_COMMAND} -E chdir ${_build_path}
                                           ${CMAKE_COMMAND}
                                           ${global_cmake_flags}
                                           -DTBB_BUILD_SHARED=OFF
                                           -DTBB_BUILD_STATIC=ON
                                           -DTBB_BUILD_TESTS=OFF
                                           -DCMAKE_BUILD_TYPE=${_cmake_build_type}
                                           -DCMAKE_CONFIGURATION_TYPES=${_cmake_build_type}
                                           -DCMAKE_INSTALL_LIBDIR=lib
                                           -DCMAKE_INSTALL_PREFIX=${_build_path}/install-root
                                           -G "${_generator}" ${_arch_option} ${_toolset_option} ${_source_path})
  rewrite_cmake_cache(${_build_path})
  execute_build(${_build_path})
endfunction()

function(compile_spirv_cross _cmake_build_type _generator _toolset_option _arch_option _triple_path)
  set(_source_path ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/spirv-cross)
  set(_build_path ${base_build_path}/spirv-cross/out/${_triple_path})
  file(MAKE_DIRECTORY ${_build_path})
  execute_process(COMMAND ${CMAKE_COMMAND} -E chdir ${_build_path}
                                           ${CMAKE_COMMAND}
                                           ${global_cmake_flags}
                                           -DSPIRV_CROSS_ENABLE_C_API=OFF
                                           -DSPIRV_CROSS_ENABLE_REFLECT=ON
                                           -DSPIRV_CROSS_ENABLE_TESTS=OFF
                                           -DCMAKE_BUILD_TYPE=${_cmake_build_type}
                                           -DCMAKE_CONFIGURATION_TYPES=${_cmake_build_type}
                                           -DCMAKE_INSTALL_LIBDIR=lib
                                           -DCMAKE_INSTALL_PREFIX=${_build_path}/install-root
                                           -DCMAKE_POSITION_INDEPENDENT_CODE=ON
                                           -G "${_generator}" ${_arch_option} ${_toolset_option} ${_source_path})
  rewrite_cmake_cache(${_build_path})
  execute_build(${_build_path})
endfunction()

function(compile_spirv_tools _cmake_build_type _generator _toolset_option _arch_option _triple_path)
  set(_source_path ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/spirv-tools)
  set(_build_path ${base_build_path}/spirv-tools/out/${_triple_path})
  file(MAKE_DIRECTORY ${_build_path})
  # checkout spirv-headers
  set(_branch_name "1.5.4.raytracing.fixed")
  execute_process(COMMAND ${GIT_EXECUTABLE} clone --branch ${_branch_name} https://github.com/KhronosGroup/SPIRV-Headers.git external/spirv-headers WORKING_DIRECTORY ${_source_path})
  execute_process(COMMAND ${GIT_EXECUTABLE} checkout ${_branch_name} WORKING_DIRECTORY ${_source_path}/external/spirv-headers)
  execute_process(COMMAND ${CMAKE_COMMAND} -E chdir ${_build_path}
                                           ${CMAKE_COMMAND}
                                           ${global_cmake_flags}
                                           -DSPIRV_COLOR_TERMINAL=OFF
                                           -DSPIRV_SKIP_TESTS=ON
                                           -DSPIRV_WARN_EVERYTHING=OFF
                                           -DSPIRV_WERROR=OFF
                                           -DCMAKE_BUILD_TYPE=${_cmake_build_type}
                                           -DCMAKE_CONFIGURATION_TYPES=${_cmake_build_type}
                                           -DCMAKE_INSTALL_LIBDIR=lib
                                           -DCMAKE_INSTALL_PREFIX=${_build_path}/install-root
                                           -G "${_generator}" ${_arch_option} ${_toolset_option} ${_source_path})
  rewrite_cmake_cache(${_build_path})
  execute_build(${_build_path})
endfunction()

function(compile_yamlcpp _cmake_build_type _generator _toolset_option _arch_option _triple_path)
  set(_source_path ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/yaml-cpp)
  if(EXISTS ${_source_path})
    set(_build_path ${base_build_path}/yaml-cpp/out/${_triple_path})
    file(MAKE_DIRECTORY ${_build_path})
    execute_process(COMMAND ${CMAKE_COMMAND} -E chdir ${_build_path}
                                             ${CMAKE_COMMAND}
                                             ${global_cmake_flags}
                                             -DAPPLE_UNIVERSAL_BIN=OFF
                                             -DBUILD_SHARED_LIBS=OFF
                                             -DBUILD_TESTING=OFF
                                             -DYAML_CPP_BUILD_CONTRIB=OFF
                                             -DYAML_CPP_BUILD_TESTS=OFF
                                             -DYAML_CPP_BUILD_TOOLS=OFF
                                             -DYAML_MSVC_SHARED_RT=OFF
                                             -DCMAKE_BUILD_TYPE=${_cmake_build_type}
                                             -DCMAKE_CONFIGURATION_TYPES=${_cmake_build_type}
                                             -DCMAKE_INSTALL_PREFIX=${_build_path}/install-root
                                             -G "${_generator}" ${_arch_option} ${_toolset_option} ${_source_path})
    execute_build(${_build_path})
  endif()
endfunction()

function(compile_glfw _cmake_build_type _generator _toolset_option _arch_option _triple_path)
  set(_source_path ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glfw)
  if(EXISTS ${_source_path})
    set(_build_path ${base_build_path}/glfw/out/${_triple_path})
    file(MAKE_DIRECTORY ${_build_path})
    execute_process(COMMAND ${CMAKE_COMMAND} -E chdir ${_build_path}
                                             ${CMAKE_COMMAND}
                                             ${global_cmake_flags}
                                             -DCMAKE_BUILD_TYPE=${_cmake_build_type}
                                             -DCMAKE_CONFIGURATION_TYPES=${_cmake_build_type}
                                             -DCMAKE_INSTALL_PREFIX=${_build_path}/install-root
                                             -DGLFW_BUILD_DOCS=OFF
                                             -DGLFW_BUILD_EXAMPLES=OFF
                                             -DGLFW_BUILD_TESTS=OFF
                                             -G "${_generator}" ${_arch_option} ${_toolset_option} ${_source_path})
    rewrite_cmake_cache(${_build_path})
    execute_build(${_build_path})
  endif()
endfunction()

function(compile_mimalloc _cmake_build_type _generator _toolset_option _arch_option _triple_path)
  set(_source_path ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/mimalloc)
  if(EXISTS ${_source_path} AND NOT WIN32)
    set(_build_path ${base_build_path}/mimalloc/out/${_triple_path})
    file(MAKE_DIRECTORY ${_build_path})
    execute_process(COMMAND ${CMAKE_COMMAND} -E chdir ${_build_path}
                                             ${CMAKE_COMMAND}
                                             ${global_cmake_flags}
                                             -DCMAKE_BUILD_TYPE=${_cmake_build_type}
                                             -DCMAKE_CONFIGURATION_TYPES=${_cmake_build_type}
                                             -DCMAKE_INSTALL_PREFIX=${_build_path}/install-root
                                             -DMI_BUILD_TESTS=OFF
                                             -DMI_DEBUG_FULL=OFF
                                             -DMI_INTERPOSE=OFF
                                             -DMI_LOCAL_DYNAMIC_TLS=OFF
                                             -DMI_OVERRIDE=OFF
                                             -DMI_SECURE=OFF
                                             -DMI_SEE_ASM=OFF
                                             -DMI_USE_CXX=OFF
                                             -G "${_generator}" ${_arch_option} ${_toolset_option} ${_source_path})
    rewrite_cmake_cache(${_build_path})
    execute_build(${_build_path})
  endif()
endfunction()

function(compile_sentry_native _cmake_build_type _generator _toolset_option _arch_option _triple_path)
  set(_source_path ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/sentry-native)
  if(EXISTS ${_source_path})
    set(_build_path ${base_build_path}/sentry-native/out/${_triple_path})
    file(MAKE_DIRECTORY ${_build_path})
    # force using vs2017 due to compilation failure
    set(_global_cmake_flags "${global_cmake_flags}")
    if(WIN32)
      set(_transport "winhttp")
    elseif(APPLE)
      set(_transport "none")
    else()
      set(_transport "curl")
    endif()
    if(WIN32 AND "${_toolset_option}" STREQUAL "-Tv140")
      set(_toolset_option "-Tv141")
    endif()
    set(_zlib_build_path ${base_build_path}/zlib/out/${_triple_path})
    execute_process(COMMAND ${CMAKE_COMMAND} -E chdir ${_build_path}
                                             ${CMAKE_COMMAND}
                                             ${_global_cmake_flags}
                                             -DCMAKE_BUILD_TYPE=${_cmake_build_type}
                                             -DCMAKE_CONFIGURATION_TYPES=${_cmake_build_type}
                                             -DCMAKE_INSTALL_PREFIX=${_build_path}/install-root
                                             -DCRASHPAD_ZLIB_SYSTEM_DEFAULT=OFF
                                             -DSENTRY_BUILD_EXAMPLES=OFF
                                             -DSENTRY_BUILD_TESTS=OFF
                                             -DSENTRY_TRANSPORT=${_transport}
                                             -DZLIB_ROOT=${base_build_path}/zlib/out/${_triple_path}/install-root
                                             -G "${_generator}" ${_arch_option} ${_toolset_option} ${_source_path})
    rewrite_cmake_cache(${_build_path})
    rewrite_ninja_ub_workaround(${_build_path})
    execute_build(${_build_path})
  endif()
endfunction()

function(compile_icu4c _cmake_build_type _generator _toolset_option _arch_option _triple_path)
  set(_source_path ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/icu)
  set(_source_path_icu4c ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/icu/icu4c)
  if(EXISTS ${_source_path} AND EXISTS ${_source_path_icu4c})
    set(_build_path ${base_build_path}/icu/out/${_triple_path})
    set(_branch_name "release-57-2")
    execute_process(COMMAND ${GIT_EXECUTABLE} checkout ${_branch_name} WORKING_DIRECTORY ${_source_path})
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/cmake/icudt57l.dat ${_source_path_icu4c}/source/data/in)
    set(_build_flags "-DUCONFIG_NO_BREAK_ITERATION -DUCONFIG_NO_COLLATION -DUCONFIG_NO_FORMATTING -DUCONFIG_NO_TRANSLITERATION -DUCONFIG_NO_REGULAR_EXPRESSIONS")
    if(APPLE)
      set(_macos_ver_min_flags "-mmacosx-version-min=10.9")
      set(_macos_arch_flags "-arch ${_arch}")
      if("${_arch}" STREQUAL "ub")
        set(_macos_arch_flags "-arch arm64 -arch x86_64")
      elseif("${_arch}" STREQUAL "arm64")
        set(_macos_ver_min_flags "-mmacosx-version-min=11.0")
      endif()
      if(DEFINED ENV{NANOEM_MACOS_SYSROOT})
        set(_macos_sysroot $ENV{NANOEM_MACOS_SYSROOT})
        set(_build_flags "${_build_flags} -isysroot ${_macos_sysroot}")
      endif()
      set(_build_flags "${_build_flags} ${_macos_ver_min_flags} ${_macos_arch_flags}")
    endif()
    set(_cflags "${_build_flags} -std=c99")
    set(_cxxflags "${_build_flags} -std=c++11")
    file(MAKE_DIRECTORY ${_build_path})
    execute_process(COMMAND
      ${CMAKE_COMMAND} -E env
        CFLAGS=${_cflags}
        CXXFLAGS=${_cxxflags}
      ${_source_path_icu4c}/source/configure
        --prefix=${_build_path}/install-root
        --with-data-packaging=static
        --enable-static
        --disable-dyload
        --disable-shared
        --disable-extras
        --disable-icuio
        --disable-layout
        --disable-layoutex
        --disable-tests
        --enable-tools=yes
        --disable-samples
      WORKING_DIRECTORY ${_build_path})
    execute_process(COMMAND make clean WORKING_DIRECTORY ${_build_path})
    execute_process(COMMAND
      ${CMAKE_COMMAND} -E env
        CFLAGS=${_cflags}
        CXXFLAGS=${_cxxflags}
      make install WORKING_DIRECTORY ${_build_path})
  endif()
endfunction()

function(compile_ffmpeg _cmake_build_type _generator _toolset_option _arch_option _triple_path)
  set(_source_path ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/ffmpeg)
  if(EXISTS ${_source_path})
    set(_build_path ${base_build_path}/ffmpeg/out/${_triple_path})
    set(_branch_name "n4.2.4")
    list(APPEND _ffmpeg_build_options "--disable-debug")
    list(APPEND _ffmpeg_build_options "--disable-asm")
    list(APPEND _ffmpeg_build_options "--disable-static")
    list(APPEND _ffmpeg_build_options "--disable-doc")
    list(APPEND _ffmpeg_build_options "--disable-htmlpages")
    list(APPEND _ffmpeg_build_options "--disable-manpages")
    list(APPEND _ffmpeg_build_options "--disable-podpages")
    list(APPEND _ffmpeg_build_options "--disable-txtpages")
    list(APPEND _ffmpeg_build_options "--disable-bzlib")
    list(APPEND _ffmpeg_build_options "--disable-iconv")
    list(APPEND _ffmpeg_build_options "--disable-lzo")
    list(APPEND _ffmpeg_build_options "--disable-sdl2")
    list(APPEND _ffmpeg_build_options "--disable-network")
    list(APPEND _ffmpeg_build_options "--disable-schannel")
    list(APPEND _ffmpeg_build_options "--disable-symver")
    list(APPEND _ffmpeg_build_options "--disable-xlib")
    list(APPEND _ffmpeg_build_options "--disable-zlib")
    list(APPEND _ffmpeg_build_options "--disable-securetransport")
    list(APPEND _ffmpeg_build_options "--disable-avdevice")
    list(APPEND _ffmpeg_build_options "--disable-avfilter")
    list(APPEND _ffmpeg_build_options "--disable-postproc")
    list(APPEND _ffmpeg_build_options "--disable-demuxers")
    list(APPEND _ffmpeg_build_options "--disable-muxers")
    list(APPEND _ffmpeg_build_options "--disable-decoders")
    list(APPEND _ffmpeg_build_options "--disable-encoders")
    list(APPEND _ffmpeg_build_options "--disable-bsfs")
    list(APPEND _ffmpeg_build_options "--disable-parsers")
    list(APPEND _ffmpeg_build_options "--disable-programs")
    list(APPEND _ffmpeg_build_options "--disable-hwaccels")
    list(APPEND _ffmpeg_build_options "--disable-filters")
    list(APPEND _ffmpeg_build_options "--disable-devices")
    list(APPEND _ffmpeg_build_options "--disable-protocols")
    list(APPEND _ffmpeg_build_options "--enable-shared")
    list(APPEND _ffmpeg_build_options "--enable-rpath")
    list(APPEND _ffmpeg_build_options "--enable-small")
    list(APPEND _ffmpeg_build_options "--enable-swresample")
    list(APPEND _ffmpeg_build_options "--enable-swscale")
    list(APPEND _ffmpeg_build_options "--enable-demuxer=avi,matroska")
    list(APPEND _ffmpeg_build_options "--enable-muxer=avi,matroska")
    list(APPEND _ffmpeg_build_options "--enable-decoder=flac,hq_hqa,hqx,pcm_s16le,rawvideo,utvideo")
    list(APPEND _ffmpeg_build_options "--enable-encoder=flac,pcm_s16le,rawvideo,utvideo")
    list(APPEND _ffmpeg_build_options "--enable-protocol=file")
    list(APPEND _ffmpeg_build_options "--enable-cross-compile")
    execute_process(COMMAND ${GIT_EXECUTABLE} checkout ${_branch_name} WORKING_DIRECTORY ${_source_path})
    if(APPLE)
      if("${_arch}" STREQUAL "ub")
        set(_macos_archs "arm64;x86_64")
      else()
        set(_macos_archs "${_arch}")
      endif()
      foreach(_item ${_macos_archs})
        set(_build_flags "-arch ${_item} -mmacosx-version-min=10.9")
        if(DEFINED ENV{NANOEM_MACOS_SYSROOT})
          set(_macos_sysroot $ENV{NANOEM_MACOS_SYSROOT})
          set(_build_flags "-isysroot ${_macos_sysroot}")
        endif()
        set(_interm_path "${_build_path}/interm/${_item}")
        file(MAKE_DIRECTORY ${_interm_path})
        set(_ffmpeg_built_options2 ${_ffmpeg_build_options})
        list(APPEND _ffmpeg_built_options2 "--prefix=\"${_interm_path}/install-root\"")
        string(JOIN ";" _full_build_options ${_ffmpeg_built_options2})
        execute_process(COMMAND
          ${CMAKE_COMMAND} -E env
            CFLAGS=${_build_flags}
            CXXFLAGS=${_build_flags}
            LDFLAGS=${_build_flags}
          ${_source_path}/configure ${_full_build_options} WORKING_DIRECTORY ${_interm_path})
        execute_process(COMMAND make install WORKING_DIRECTORY ${_interm_path})
      endforeach()
      if("${_arch}" STREQUAL "ub")
        set(_interm_arm64_path ${_build_path}/interm/arm64/install-root)
        set(_interm_x86_64_path ${_build_path}/interm/x86_64/install-root)
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${_interm_arm64_path}/include ${_build_path}/install-root/include)
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${_interm_arm64_path}/share ${_build_path}/install-root/share)
        execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${_build_path}/install-root/lib)
        file(GLOB _libraries ${_interm_arm64_path}/lib/lib*.dylib)
        foreach(_item ${_libraries})
          get_filename_component(_filename ${_item} NAME)
          execute_process(COMMAND lipo -create -output
            "${_build_path}/install-root/lib/${_filename}"
            "${_interm_arm64_path}/lib/${_filename}"
            "${_interm_x86_64_path}/lib/${_filename}")
        endforeach()
      endif()
    else()
      file(MAKE_DIRECTORY ${_build_path})
      list(APPEND _ffmpeg_build_options "--prefix=\"${_build_path}/install-root\"")
      string(JOIN ";" _full_build_options ${_ffmpeg_build_options})
      execute_process(COMMAND ${_source_path}/configure ${_full_build_options} WORKING_DIRECTORY ${_build_path})
      execute_process(COMMAND make install WORKING_DIRECTORY ${_build_path})
    endif()
  endif()
endfunction()

function(compile_all_repositories _generator _toolset_option _compiler _arch _config)
  if(CMAKE_TOOLCHAIN_FILE)
    set(global_cmake_flags "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE};${global_cmake_flags}")
  endif()
  if("${_config}" STREQUAL "debug")
    set(_cmake_build_type "Debug")
  elseif("${_config}" STREQUAL "release")
    set(_cmake_build_type "MinSizeRel")
  endif()
  # base
  if(DEFINED ENV{NANOEM_BUILD_DEPENDENCIES_DIRECTORY})
    set(base_build_path $ENV{NANOEM_BUILD_DEPENDENCIES_DIRECTORY})
  else()
    set(base_build_path ${CMAKE_CURRENT_SOURCE_DIR}/dependencies)
  endif()
  # system name
  if(DEFINED ENV{NANOEM_TARGET_SYSTEM_NAME})
    set(_platform $ENV{NANOEM_TARGET_SYSTEM_NAME})
  elseif(CMAKE_SYSTEM_NAME)
    string(TOLOWER CMAKE_SYSTEM_NAME _platform)
  elseif(WIN32)
    set(_platform "windows")
  elseif(APPLE)
    set(_platform "darwin")
    set(_macos_archs "${_arch}")
    if("${_arch}" STREQUAL "ub")
      set(_macos_archs "arm64\;x86_64")
    elseif("${_arch}" STREQUAL "arm64")
      set(OSX_TARGET "11.0")
    endif()
    set(global_cmake_flags "-DCMAKE_OSX_ARCHITECTURES=${_macos_archs};-DCMAKE_OSX_DEPLOYMENT_TARGET=${OSX_TARGET};${global_cmake_flags}")
    if(DEFINED ENV{NANOEM_MACOS_SYSROOT})
      set(_macos_sysroot $ENV{NANOEM_MACOS_SYSROOT})
      set(global_cmake_flags "-DCMAKE_OSX_SYSROOT=${_macos_sysroot};${global_cmake_flags}")
    endif()
  else()
    set(_platform "linux")
  endif()
  set(_triple_path ${_platform}/${_compiler}/${_arch}/${_config})
  set(_arch_option " ")
  if("${target_generator}" STREQUAL "Visual Studio 17 2022")
    if(${arch_item} STREQUAL "arm64")
      set(_arch_option "-AARM64")
    elseif(${arch_item} STREQUAL "x86_64")
      set(_arch_option "-Ax64")
    elseif(${arch_item} STREQUAL "i386")
      set(_arch_option "-AWin32")
    endif()
  endif()
  compile_zlib(${_cmake_build_type} ${_generator} ${_toolset_option} ${_arch_option} ${_triple_path})
  compile_minizip(${_cmake_build_type} ${_generator} ${_toolset_option} ${_arch_option} ${_triple_path})
  compile_bullet(${_cmake_build_type} ${_generator} ${_toolset_option} ${_arch_option} ${_triple_path})
  compile_glslang(${_cmake_build_type} ${_generator} ${_toolset_option} ${_arch_option} ${_triple_path})
  if(NOT DEFINED ENV{NANOEM_DISABLE_BUILD_NANOMSG})
    compile_nanomsg(${_cmake_build_type} ${_generator} ${_toolset_option} ${_arch_option} ${_triple_path})
  endif()
  if(NOT DEFINED ENV{NANOEM_DISABLE_BUILD_TBB})
    compile_tbb(${_cmake_build_type} ${_generator} ${_toolset_option} ${_arch_option} ${_triple_path})
  endif()
  if(NOT DEFINED ENV{NANOEM_DISABLE_BUILD_GLFW})
    compile_glfw(${_cmake_build_type} ${_generator} ${_toolset_option} ${_arch_option} ${_triple_path})
  endif()
  if(NOT DEFINED ENV{NANOEM_DISABLE_BUILD_MIMALLOC})
    compile_mimalloc(${_cmake_build_type} ${_generator} ${_toolset_option} ${_arch_option} ${_triple_path})
  endif()
  compile_spirv_cross(${_cmake_build_type} ${_generator} ${_toolset_option} ${_arch_option} ${_triple_path})
  # due to compiler error, disable compiling spirv-tools on CI (CircleCI)
  if(NOT DEFINED ENV{NANOEM_DISABLE_BUILD_SPIRV_TOOLS})
    compile_spirv_tools(${_cmake_build_type} ${_generator} ${_toolset_option} ${_arch_option} ${_triple_path})
  endif()
  if(NOT DEFINED ENV{NANOEM_DISABLE_ICU4C})
    compile_icu4c(${_cmake_build_type} ${_generator} ${_toolset_option} ${_arch_option} ${_triple_path})
  endif()
  if(NOT DEFINED ENV{NANOEM_DISABLE_BUILD_SENTRY_NATIVE})
    compile_sentry_native(${_cmake_build_type} ${_generator} ${_toolset_option} ${_arch_option} ${_triple_path})
  endif()
  if(NOT DEFINED ENV{NANOEM_DISABLE_BUILD_LIBSOUNDIO})
    compile_libsoundio(${_cmake_build_type} ${_generator} ${_toolset_option} ${_arch_option} ${_triple_path})
  endif()
  if(NOT DEFINED ENV{NANOEM_DISABLE_BUILD_YAMLCPP})
    compile_yamlcpp(${_cmake_build_type} ${_generator} ${_toolset_option} ${_arch_option} ${_triple_path})
  endif()
  if(NOT DEFINED ENV{NANOEM_DISABLE_BUILD_FFMPEG})
    compile_ffmpeg(${_cmake_build_type} ${_generator} ${_toolset_option} ${_arch_option} ${_triple_path})
  endif()
  if("${_arch}" STREQUAL "ub")
    file(GLOB _libraries ${base_build_path}/*)
    foreach(_item ${_libraries})
      set(_path "${_item}/out/${_platform}/${_compiler}")
      if(EXISTS ${_path})
        file(CREATE_LINK ${_path}/ub ${_path}/arm64 SYMBOLIC)
        file(CREATE_LINK ${_path}/ub ${_path}/x86_64 SYMBOLIC)
      endif()
    endforeach()
  endif()
endfunction()

function(cleanup_all_repositories)
  set(all_repositories "bullet3;glslang;minizip;zlib")
  foreach(item ${all_repositories})
    string(STRIP "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/${item}" _path)
    execute_process(COMMAND ${GIT_EXECUTABLE} checkout -- . WORKING_DIRECTORY ${_path})
    execute_process(COMMAND ${GIT_EXECUTABLE} clean -df -e out WORKING_DIRECTORY ${_path})
  endforeach()
endfunction()

set(ARCH_LIST $ENV{NANOEM_TARGET_ARCHITECTURES})
if(NOT ARCH_LIST)
  if(APPLE)
    # ub = "arm64;x86_64"
    set(ARCH_LIST "ub")
  else()
    set(ARCH_LIST "x86_64")
  endif()
endif()
set(CONFIG_LIST $ENV{NANOEM_TARGET_CONFIGURATIONS})
if(NOT CONFIG_LIST)
  set(CONFIG_LIST "debug;release")
endif()
set(OSX_TARGET $ENV{NANOEM_TARGET_MACOS_VERSION})
if(NOT OSX_TARGET)
  set(OSX_TARGET "10.9")
endif()
set(CMAKE_FIND_LIBRARY_PREFIXES "lib;")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".so;.a;.lib")
find_package(Git REQUIRED)
execute_process(COMMAND ${GIT_EXECUTABLE} checkout -- .
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/bullet3
                RESULT_VARIABLE result)
execute_process(COMMAND ${GIT_EXECUTABLE} apply --ignore-space-change ${CMAKE_CURRENT_SOURCE_DIR}/cmake/bullet-2.76.diff
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/bullet3
                RESULT_VARIABLE result)
execute_process(COMMAND ${GIT_EXECUTABLE} checkout -- .
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glslang)
file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glslang/glslang/Include/InfoSink.h input_info_sink_h NEWLINE_CONSUME)
# due to error detection of LLVM for windows, replaccde "UNKNOWN ERROR :" to "UNKNOWN :"
string(REPLACE "UNKNOWN ERROR:" "UNKNOWN:" output_info_sink_h "${input_info_sink_h}")
file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glslang/glslang/Include/InfoSink.h ${output_info_sink_h})

foreach(arch_item ${ARCH_LIST})
  set(target_generator $ENV{NANOEM_TARGET_GENERATOR})
  set(target_compiler $ENV{NANOEM_TARGET_COMPILER})
  set(target_toolset $ENV{NANOEM_TARGET_TOOLSET})
  set(make_program $ENV{NANOEM_MAKE_PROGRAM})
  if(make_program)
    set(toolset_option "-DCMAKE_MAKE_PROGRAM=${make_program}")
  endif()
  if(WIN32)
    # https://docs.microsoft.com/en-us/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8
    if(NOT "${arch_item}" STREQUAL "arm64" AND NOT "${target_compiler}" STREQUAL "clang")
      set(global_cmake_flags "-DCMAKE_CXX_FLAGS='-utf-8'")
    endif()
    if(NOT target_compiler)
      set(target_compiler "vs2022")
    endif()
    if(NOT target_generator)
      set(target_generator "Visual Studio 17 2022")
    endif()
    if(target_toolset)
      set(target_toolset "-T${target_toolset}")
    elseif(NOT target_toolset)
      if("${target_compiler}" STREQUAL "vs2017")
        set(target_toolset "-Tv141")
      elseif("${target_compiler}" STREQUAL "vs2015")
        set(target_toolset "-Tv140")
      else()
        set(target_toolset " ")
      endif()
    endif()
    # MinGW
    if("${target_compiler}" STREQUAL "gcc")
      list(APPEND _gcc_flags "-DCMAKE_C_FLAGS='-std=c99'")
      list(APPEND _gcc_flags "-DCMAKE_CXX_FLAGS='-std=c++14 -fpermissive'")
      set(global_cmake_flags "${_gcc_flags}")
    endif()
  else()
    if(NOT target_generator)
      set(target_generator "Ninja")
    endif()
    if(target_compiler)
      set(global_cmake_flags "-DCMAKE_CXX_FLAGS='-std=c++14'")
    else()
      if(APPLE)
        set(target_compiler "clang")
        list(APPEND _clang_flags "-DCMAKE_C_FLAGS='-std=c99'")
        list(APPEND _clang_flags "-DCMAKE_CXX_FLAGS='-std=c++14'")
        set(global_cmake_flags "${_clang_flags}")
      else()
        set(target_compiler "gcc")
        list(APPEND _gcc_flags "-DCMAKE_C_FLAGS='-std=c99 -fPIC'")
        list(APPEND _gcc_flags "-DCMAKE_CXX_FLAGS='-std=c++14 -fPIC'")
        set(global_cmake_flags "${_gcc_flags}")
      endif()
    endif()
    if(NOT target_toolset)
      set(target_toolset " ")
    endif()
  endif()
  foreach(config_item ${CONFIG_LIST})
    compile_all_repositories(${target_generator} ${target_toolset} ${target_compiler} ${arch_item} ${config_item})
  endforeach()
endforeach()
cleanup_all_repositories()
