cmake_minimum_required(VERSION 3.16)
project(subspace)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(subspace_include_dirs
  "${subspace_SOURCE_DIR}"
)
include_directories(${subspace_include_dirs})

# add the executable
add_library(subspace STATIC 
    "assertions/builtin.h"
    "assertions/check.h"
    "assertions/panic.h"
    "assertions/unreachable.h"
    "marker/unsafe.h"
    "mem/__private/relocate.h"
    "mem/replace.h"
    "mem/swap.h"
    "mem/take.h"
    "mem/addressof.h"
    "lib/lib.cc"
)
set_target_properties(subspace PROPERTIES LINKER_LANGUAGE CXX)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory("third_party/googletest")

enable_testing()

add_executable(subspace_unittests
    "assertions/check_unittest.cc"
    "assertions/panic_unittest.cc"
    "assertions/unreachable_unittest.cc"
    "mem/__private/relocate_unittest.cc"
    "mem/addressof_unittest.cc"
    "mem/replace_unittest.cc"
    "mem/swap_unittest.cc"
    "mem/take_unittest.cc"
)
set_target_properties(subspace_unittests PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(subspace_unittests
    subspace
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(
    subspace_unittests
)
