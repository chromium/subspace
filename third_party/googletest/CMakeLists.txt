if(${SUBSPACE_BUILD_TESTS} OR ${SUBSPACE_BUILD_BENCHMARKS})
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        73a63ea05dc8ca29ec1d2c1d66481dd0de1950f1 # 2026-02-24
    SYSTEM
    OVERRIDE_FIND_PACKAGE
  )
endif()
