# Subspace Library

An experimental take on a safer, simpler C++ standard library.

Please don't use this library. This is an experiment and we don't yet know where
it will take us. There will be breaking changes without warning, and there is no
stable version.

# Copyright

All source files must include this header:
```
// Copyright 2022 Google LLC
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

# Getting started

1. Install [LLVM](https://releases.llvm.org/download.html)
1. Install [CMake](https://cmake.org/install/)
1. Install [Python](https://www.python.org/downloads/)
1. `cmake -B out`
1. `cmake --build out`
1. `ctest --test-dir out`

## For Windows/VSCode.

1. Install Visual Studio 2022. Don't bother installing the CMake tools from it, they don't work as well
1. Set the "Windows SDK Version" in VSCode to `10.0.19041.0` (or the version installed with Visual Studio)
1. Set the "CMake: Generator" in VSCode to `Visual Studio 17 2022`, or add it to "CMake: Preferred Generators"
