# Set-ExecutionPolicy RemoteSigned -Scope Process

$env:NANOEM_DISABLE_BUILD_SENTRY_NATIVE=1
$env:NANOEM_DISABLE_BUILD_TBB=1
$env:NANOEM_TARGET_ARCHITECTURES="arm64"
$env:NANOEM_TARGET_COMPILER="vs2022"
$env:NANOEM_TARGET_GENERATOR="Visual Studio 17 2022"
$build_cmake_path = (Join-Path $PSScriptRoot ../../build.cmake)

cmake -P $build_cmake_path
