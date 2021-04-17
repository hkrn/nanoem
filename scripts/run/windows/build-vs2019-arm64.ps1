# Set-ExecutionPolicy RemoteSigned -Scope Process

$env:NANOEM_DISABLE_BUILD_SENTRY_NATIVE=1
$env:NANOEM_TARGET_ARCHITECTURES="arm64"
$env:NANOEM_TARGET_COMPILER="vs2019"
$env:NANOEM_TARGET_GENERATOR="Visual Studio 16 2019"
$build_cmake_path = (Join-Path $PSScriptRoot ../../build.cmake)

cmake -P $build_cmake_path
