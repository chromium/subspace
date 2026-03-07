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
