# Using Subspace

Subspace can be integrated into any other project built with CMake. [Git
submodules](https://git-scm.com/book/en/v2/Git-Tools-Submodules) can be
used to pull the Subspace code into your project.

## Add `subspace` as a submodule

```
git submodule add https://github.com/chromium/subspace third_party/subspace
```

## CMakeLists.txt

```
cmake_minimum_required(VERSION 3.25)

project(subspace-example VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(third_party/subspace)

add_executable(subspace-example src/main.cc)
target_link_libraries(subspace-example PRIVATE subspace::lib)
```

## src/main.cc
```
#include "sus/assertions/check.h"
#include "sus/prelude.h"  // Import core types.

int main() {
  i32 a = 6;
  i32 b = 9;
  sus::check(a < b);
  return 0;
}
```

## Building

Configure your build:
```
mkdir -p out/Debug
CXX=clang++ cmake -GNinja -B out/Debug -BCMAKE_BUILD_TYPE=Debug
```

The above uses Ninja, which requires it to be installed. Drop the `-GNinja` otherwise.

Compile:
```
cmake --build out/Debug
```
