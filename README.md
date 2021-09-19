# nanoem

[![Build Status](https://github.com/hkrn/nanoem/workflows/CI/badge.svg)](https://github.com/hkrn/nanoem/actions) [![License: MPL 2.0](https://img.shields.io/badge/License-MPL%202.0-blue.svg)](https://opensource.org/licenses/MPL-2.0) [![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

nanoem is an [MMD (MikuMikuDance)](https://sites.google.com/view/vpvp/) compatible implementation and its like application mainly built for macOS.

- [All Releases](https://github.com/hkrn/nanoem/releases)
- [Usage Manual](https://nanoem.readthedocs.io) (Japanese)

## Background and Design Concept

MikuMikuDance (a.k.a MMD) is created on Windows via DirectX9. As such, it's unavailable to run on non-Windows deployment unless you use a virtual machine or emulation layer such as Wine.

nanoem was originally created to address the issue of non-Windows compatibility as well as the possibility it may not work in the future due to Windows reason. It was originally designed for macOS but now it's now designed to run on non-macOS as well and supports ARM CPU such as Apple M1 and RaspberryPi, multiple graphics backends (DirectX11/Metal/OpenGL).

It also has model editing feature similar to PMXEditor's one which allows for some model editing. This was also implemented for the same reason mentioned above.

nanoem is designed to achieve both of the following goals.

* Portability
* Lightweight startup
* Small size

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
git submodule update --init --recursive

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

## Contributors

<a href="https://github.com/hkrn/nanoem/graphs/contributors">
  <img src="https://contributors-img.web.app/image?repo=hkrn/nanoem" />
</a>
