include(FetchContent)

# Thirds-party projects should be built with warnings disabled to limit noise and build breaks.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -Wno-error")

# This directory handles third-party dependency package management.
#
# Packages that use CMake are handled using `FetchContent`. `FetchContent` is split into two
# function calls: `FetchContent_Declare` and `FetchContent_MakeAvailable`. `FetchContent_Declare`
# tells CMake how to download the package, how to apply any local patches, and how the CMake project
# should be configured. `FetchContent_Declare` is lazy: it tells CMake what needs to be done, but
# doesn't execute any of that work (see below for why). `FetchContent_MakeAvailable` tells CMake to
# execute the work declared by `FetchContent_Declare`.
#
# Optimally using `FetchContent` requires sequencing all calls to `FetchContent_Declare` before any
# calls to `FetchContent_MakeAvailable`. CMake is able to manage common subdependencies when calls
# are correctly sequenced. For example, a project that depends on libfmt, GoogleTest, and Subspace
# only needs to download libfmt and GoogleTest once, even though they're also dependencies for
# Subspace.
#
# To ensure that Subspace has optimal sequencing, we split the calls to `FetchContent_Declare` into
# two files: `FetchContent_Declare.cmake` and `FetchContent_MakeAvailable.cmake`, which handle the
# logic for their respective namesakes.
#
# See https://cmake.org/cmake/help/latest/module/FetchContent.html#command:fetchcontent_makeavailable
# for more.
include("${CMAKE_CURRENT_SOURCE_DIR}/FetchContent_Declare.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/FetchContent_MakeAvailable.cmake")
