# Set-ExecutionPolicy RemoteSigned -Scope Process

$env:NANOEM_DISABLE_BUILD_GLFW=1
$env:NANOEM_DISABLE_BUILD_SENTRY_NATIVE=1
$env:NANOEM_DISABLE_BUILD_TBB=1
$env:NANOEM_TARGET_ARCHITECTURES="x86_64"
$env:NANOEM_TARGET_COMPILER="clang"
$env:NANOEM_TARGET_GENERATOR="Ninja"
$build_cmake_path = (Join-Path $PSScriptRoot ../../build.cmake)

cmake -P $build_cmake_path
