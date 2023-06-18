[![CI](https://github.com/chromium/subspace/actions/workflows/ci.yml/badge.svg)](https://github.com/chromium/subspace/actions/workflows/ci.yml)
[![hdoc](https://github.com/chromium/subspace/actions/workflows/hdoc.yml/badge.svg)](https://docs.hdoc.io/danakj/subspace/)
[![sub-doc](https://github.com/chromium/subspace/actions/workflows/subdoc.yml/badge.svg)](https://danakj.github.io/subspace-docs/sus.html)
<!---
[![clang-doc](https://github.com/chromium/subspace/actions/workflows/clang-doc.yml/badge.svg)](https://danakj.github.io/subspace-docs/sus/#Namespaces)
-->
# Subspace Library

An [experimental take](https://danakj.github.io/2022/12/31/why-subspace.html)
on a safer, simpler C++ standard library.

Please don't use this library. This is an experiment and we don't yet know where
it will take us. There will be breaking changes without warning, and there is no
stable version.

## Getting started

1. Install [depot_tools](https://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up)
1. Install [CMake](https://cmake.org/install/)
1. Install [python3](https://www.python.org/downloads/)
1. `gclient sync --jobs=16`
1. `cmake -B out -DSUBSPACE_BUILD_CIR=OFF -DSUBSPACE_BUILD_SUBDOC=OFF`
1. `cmake --build out`
1. `ctest --test-dir out`

## CMake options
The CMake files define the following options:

* `SUBSPACE_BUILD_CIR`, defaults to `ON`, set to `OFF` to disable CIR build
* `SUBSPACE_BUILD_SUBDOC`, defaults to `ON`, set to `OFF` to disable Subdoc build
* `SUBSPACE_BUILD_TESTS`, defaults to `ON`, set to `OFF` to disable building unit tests

To include Subdoc or CIR in your build, re-run `cmake -B out` and change the value for
them from `OFF` to `ON`. You will need LLVM and Clang libraries available. Set the
`LLVM_DIR` and `Clang_DIR` environment variables when running `cmake` to point it to
your installation. See [Building CIR and Subdoc](#Building-CIR-and-Subdoc) for more.

## Clang format

The latest LLVM release does not have sufficient C++20 support to format the
library correctly. To get a more bleeding edge clang-format executable, we get
it from the Chromium project. The binary is downloaded by gclient and
resides in `third_party/buildtools/<platform>/clang-format`. Point your IDE at that
executable.

## For Windows/VSCode.

1. Install Visual Studio 2022. Don't bother installing the CMake tools from it, they don't work as well
1. Set the "Windows SDK Version" in VSCode to `10.0.19041.0` (or the version installed with Visual Studio)
1. Set the "CMake: Generator" in VSCode to `Visual Studio 17 2022`, or add it to "CMake: Preferred Generators"

# Building CIR and Subdoc

CIR is a mid-level representation of C++ built on Clang in the spirit of [Rust's
MIR](https://kanishkarj.github.io/rust-internals-mir).

Subdoc is a documentation generator from inline C++ documentation in the spirit of
[Rustdoc](https://doc.rust-lang.org/rustdoc/what-is-rustdoc.html).

These both require an installation of LLVM from Git HEAD, including the LLVM/Clang
headers and libraries.

By default, in VSCode it looks for LLVM and clang in your system paths, but you can override
this to point to your own installation from your VSCode user settings:
```json
"cmake.environment": {
    "LLVM_DIR": "path/to/llvm/install/lib/cmake/llvm",
    "Clang_DIR": "path/to/llvm/install/lib/cmake/clang",
},
```

To build LLVM from VSCode, the following can be put in LLVM's
`.vscode/settings.json`:
```json
{
    "cmake.configureOnOpen": false,
    "cmake.sourceDirectory": "${workspaceFolder}/llvm",
    "cmake.buildDirectory": "${workspaceFolder}/build/${buildType}/${variant:platform}",
    "cmake.installPrefix": "${workspaceFolder}/build/install",
    "cmake.generator": "Ninja",
    "cmake.configureArgs": [
        "-DLLVM_ENABLE_PROJECTS=clang",
    ],
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"
}
```
Then use VSCode to choose a build configuration and run the "CMake: Build" and
"CMake: Install" tasks.

## Windows

On windows, set the environment variable `LLVM_DEBUG=1` if the LLVM build was a
debug build in order for CIR to link the appropriate runtime.

# Using Subspace with CMake

If you're looking to use Subspace in a project, CMake and `git submodules` can be used to help
integrate the subspace library.

## Add `subspace` as a submodule
* `git submodule add https://github.com/chromium/subspace third_party/subspace`

## CMakeLists.txt

```
cmake_minimum_required(VERSION 3.25)

project(subspace-example VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

SET(SUBSPACE_BUILD_CIR OFF)
SET(SUBSPACE_BUILD_SUBDOC OFF)
SET(SUBSPACE_BUILD_TESTS OFF)
add_subdirectory(third_party/subspace)

add_executable(subspace-example src/main.cc)
target_link_libraries(subspace-example PRIVATE subspace::lib)
```

## src/main.cc
```
#include "subspace/assertions/check.h"

int main() {
  sus::check(false);
  return 0;
}
```

## Building
```
mkdir -p out/Debug
cd out/Debug
CXX=clang++ cmake -GNinja ../..
ninja
```

# Copyright

All source files must include this header:
```
// Copyright 2023 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
```
