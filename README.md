# nanoem

[![Build Status](https://github.com/hkrn/nanoem/workflows/CI/badge.svg)](https://github.com/hkrn/nanoem/actions) [![License: MPL 2.0](https://img.shields.io/badge/License-MPL%202.0-blue.svg)](https://opensource.org/licenses/MPL-2.0) [![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

nanoem is an [MMD (MikuMikuDance)](https://sites.google.com/view/vpvp/) compatible implementation and its like application mainly built for macOS.

- [Binary Distribution of macOS](https://bowlroll.net/file/71328)
- [Binary Distribution of Windows](https://bowlroll.net/file/122592)
- [Usage Manual](https://nanoem.readthedocs.io)

## How to build?

### Prerequisites

- [cmake](https://cmake.org) (>= 3.5)
- C++14 compliant compiler
  - confirmed on Clang and Visual Studio 2017
- [git](https://git-scm.com)
- [ninja-build](https://ninja-build.org/)
  - Optional but recommend on macOS/Linux

### Minimum build instruction

See also [GitHub Action Workflow](.github/workflows/main.yml).

```bash
git submodule update --init --recurse

# needs setting NANOEM_TARGET_COMPILER explicitly when the compiler is clang (default is gcc on Linux)
export NANOEM_TARGET_COMPILER=clang
cmake -P scripts/build.cmake
mkdir out
cd out
cmake -G Ninja ..
cmake --build .

# OpenGL 3.3 Core Profile or higher is required to run on Linux
cd sapp && ./nanoem
```

## Architecture

See [Architecture Document](docs/architecture.rst) (Japanese)

## About License

- nanoem component is licensed under [MIT/X11 License](LICENSE.MIT).
- emapp/win32/macos/glfw/sapp components are licensed under [Mozilla Public License](LICENSE.MPL).

## Authors

 - hkrn
