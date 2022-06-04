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

## Clang format

The latest LLVM release does not have sufficient C++20 support to format the
library correctly. To get a more bleeding edge clang-format executable, we get
it from the Chromium project:

1. Install [python3](https://www.python.org/downloads/) (or from the Windows
   Store) and add it to your path.
1. Download
   [depot_tools](https://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up),
   extract it, and put it in your path.
1. Run `gclient` in a terminal to set it up.
1. Make a directory to hold the clang-format binary.
1. Download the [sha1
   file](https://source.chromium.org/search?q=clang-format.*%5C.sha1&sq=&ss=chromium%2Fchromium%2Fsrc:buildtools%2F)
   for your platform and put it in that directory.
1. From in that same directory, in a terminal, run download_from_google_storage
   to pull down the clang-format binary.
    * On Windows: `python ..\depot_tools\download_from_google_storage.py --no_resume --no_auth --bucket chromium-clang-format -s clang-format.exe.sha1`
    * Elsewhere: `python ..\depot_tools\download_from_google_storage.py
     --no_resume --no_auth --bucket chromium-clang-format -s clang-format.sha1`
    * The command comes [from here](https://source.chromium.org/chromium/chromium/src/+/main:DEPS;l=3922-3930;drc=ea057e49017041bc61fb7dee82d08a4ca8078565).
1. Point your IDE to the downloaded clang-format executable.

## For Windows/VSCode.

1. Install Visual Studio 2022. Don't bother installing the CMake tools from it, they don't work as well
1. Set the "Windows SDK Version" in VSCode to `10.0.19041.0` (or the version installed with Visual Studio)
1. Set the "CMake: Generator" in VSCode to `Visual Studio 17 2022`, or add it to "CMake: Preferred Generators"
