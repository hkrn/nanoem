# Set-ExecutionPolicy RemoteSigned -Scope Process

$env:NANOEM_TARGET_ARCHITECTURES="x86_64"
$env:NANOEM_TARGET_COMPILER="vs2017"
$env:NANOEM_TARGET_GENERATOR="Visual Studio 16 2019"
$env:NANOEM_TARGET_TOOLSET="v141"
$build_cmake_path = (Join-Path $PSScriptRoot ../../build.cmake)

cmake -P $build_cmake_path
$env:NANOEM_TARGET_ARCHITECTURES="i386"
cmake -P $build_cmake_path
