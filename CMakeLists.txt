cmake_minimum_required(VERSION 3.16)
project(subspace)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
    "containers/array.h"
    "concepts/make_default.h"
    "containers/array.h"
    "marker/unsafe.h"
    "mem/__private/relocatable_storage.h"
    "mem/__private/relocate.h"
    "mem/addressof.h"
    "mem/mref.h"
    "mem/pass.h"
    "mem/replace.h"
    "mem/swap.h"
    "mem/take.h"
    "option/__private/is_option_type.h"
    "option/option.h"
    "traits/iter/filter.h"
    "traits/iter/iterator.h"
    "traits/iter/iterator.cc"
    "traits/iter/iterator_defn.h"
    "traits/iter/iterator_fns.h"
    "traits/iter/once.h"
    "traits/iter/sized_iterator.h"
    "lib/lib.cc"
)
set_target_properties(subspace PROPERTIES LINKER_LANGUAGE CXX)

add_library(subspace_test_support STATIC
    "test/behaviour_types.h"
    "test/behaviour_types_unittest.cc"
)
set_target_properties(subspace_test_support PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(subspace_test_support
    subspace
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory("third_party/googletest")

enable_testing()

add_executable(subspace_unittests
    "assertions/check_unittest.cc"
    "assertions/panic_unittest.cc"
    "assertions/unreachable_unittest.cc"
    "containers/array_unittest.cc"
    "concepts/make_default_unittest.cc"
    "containers/array_unittest.cc"
    "mem/__private/relocate_unittest.cc"
    "mem/addressof_unittest.cc"
    "mem/mref_unittest.cc"
    "mem/replace_unittest.cc"
    "mem/swap_unittest.cc"
    "mem/take_unittest.cc"
    "option/option_unittest.cc"
    "option/option_types_unittest.cc"
    "traits/iter/iterator_unittest.cc"
)
set_target_properties(subspace_unittests PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(subspace_unittests
    subspace
    subspace_test_support
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(
    subspace_unittests
)
