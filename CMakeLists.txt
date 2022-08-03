cmake_minimum_required(VERSION 3.16)
project(subspace)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(MSVC)
    add_compile_options(/Zc:preprocessor /wd5105)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Werror)
endif()

set(subspace_include_dirs
    "${subspace_SOURCE_DIR}"
)
include_directories(${subspace_include_dirs})

# add the executable
add_library(subspace STATIC
    "assertions/arch.h"
    "assertions/builtin.h"
    "assertions/check.h"
    "assertions/endian.h"
    "assertions/nonnull.h"
    "assertions/panic.h"
    "assertions/panic.cc"
    "assertions/unreachable.h"
    "assertions/windows.h"
    "containers/array.h"
    "concepts/from.h"
    "concepts/into.h"
    "concepts/make_default.h"
    "concepts/callable.h"
    "containers/array.h"
    "fn/__private/fn_storage.h"
    "fn/fn.h"
    "fn/fn_bind.h"
    "fn/fn_defn.h"
    "fn/fn_impl.h"
    "iter/filter.h"
    "iter/iterator.h"
    "iter/iterator.cc"
    "iter/iterator_defn.h"
    "iter/iterator_impl.h"
    "iter/once.h"
    "iter/sized_iterator.h"
    "macros/always_inline.h"
    "macros/for_each.h"
    "macros/remove_parens.h"
    "marker/unsafe.h"
    "mem/__private/relocatable_storage.h"
    "mem/__private/relocate.h"
    "mem/addressof.h"
    "mem/forward.h"
    "mem/move.h"
    "mem/mref.h"
    "mem/pass.h"
    "mem/replace.h"
    "mem/swap.h"
    "mem/take.h"
    "num/__private/intrinsics.h"
    "num/__private/literals.h"
    "num/__private/ptr_type.h"
    "num/__private/signed_integer_macros.h"
    "num/__private/unsigned_integer_macros.h"
    "num/signed_integer.h"
    "num/integer_concepts.h"
    "num/num_concepts.h"
    "num/unsigned_integer.h"
    "option/__private/is_option_type.h"
    "option/option.h"
    "tuple/__private/storage.h"
    "tuple/tuple.h"
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
    "assertions/endian_unittest.cc"
    "assertions/panic_unittest.cc"
    "assertions/unreachable_unittest.cc"
    "containers/array_unittest.cc"
    "concepts/from_unittest.cc"
    "concepts/into_unittest.cc"
    "concepts/make_default_unittest.cc"
    "containers/array_unittest.cc"
    "fn/fn_unittest.cc"
    "iter/iterator_unittest.cc"
    "mem/__private/relocate_unittest.cc"
    "mem/addressof_unittest.cc"
    "mem/move_unittest.cc"
    "mem/mref_unittest.cc"
    "mem/replace_unittest.cc"
    "mem/swap_unittest.cc"
    "mem/take_unittest.cc"
    "num/__private/literals_unittest.cc"
    "num/i8_unittest.cc"
    "num/i16_unittest.cc"
    "num/i32_unittest.cc"
    "num/i64_unittest.cc"
    "num/isize_unittest.cc"
    "num/u8_unittest.cc"
    "num/u16_unittest.cc"
    "num/u32_unittest.cc"
    "num/u64_unittest.cc"
    "num/usize_unittest.cc"
    "option/option_unittest.cc"
    "option/option_types_unittest.cc"
    "tuple/tuple_unittest.cc"
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
