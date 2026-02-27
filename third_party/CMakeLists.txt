include(FetchContent)

# Note: The last `FetchContent_Declare` must go before the first `FetchContent_MakeAvailable` so
# CMake can manage common subdependencies. To help catch accidental misorderings, we put each
# `FetchContent_Declare` in a subdirectory named after the dependency, and the corresponding
# `FetchContent_MakeAvailable` in this file.
#
# See https://cmake.org/cmake/help/latest/module/FetchContent.html#command:fetchcontent_makeavailable
# for more.

add_subdirectory(buildtools)
add_subdirectory(fmt)
add_subdirectory(googletest)
add_subdirectory(md4c)
add_subdirectory(nanobench)

FetchContent_MakeAvailable(buildtools)
FetchContent_MakeAvailable(fmt)

if(${SUBSPACE_BUILD_TESTS} OR ${SUBSPACE_BUILD_BENCHMARKS})
  FetchContent_MakeAvailable(googletest)
endif()

if(SUBSPACE_BUILD_SUBDOC)
  FetchContent_MakeAvailable(md4c)
endif()

if(SUBSPACE_BUILD_BENCHMARKS)
  FetchContent_MakeAvailable(nanobench)
  set_property(TARGET nanobench PROPERTY CXX_STANDARD 20)
endif()
