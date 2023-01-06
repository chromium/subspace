# Copyright 2023 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

add_library(subspace STATIC
    "assertions/arch.h"
    "assertions/check.h"
    "assertions/endian.h"
    "assertions/panic.h"
    "assertions/panic.cc"
    "assertions/unreachable.h"
    "choice/__private/all_values_are_unique.h"
    "choice/__private/index_of_value.h"
    "choice/__private/index_type.h"
    "choice/__private/marker.h"
    "choice/__private/nothing.h"
    "choice/__private/ops_concepts.h"
    "choice/__private/pack_index.h"
    "choice/__private/storage.h"
    "choice/__private/type_list.h"
    "choice/choice.h"
    "choice/choice_types.h"
    "convert/subclass.h"
    "construct/__private/into_ref.h"
    "construct/from.h"
    "construct/into.h"
    "construct/default.h"
    "containers/__private/array_iter.h"
    "containers/__private/array_marker.h"
    "containers/__private/slice_iter.h"
    "containers/__private/vec_iter.h"
    "containers/__private/vec_marker.h"
    "containers/array.h"
    "containers/slice.h"
    "containers/vec.h"
    "fn/__private/fn_storage.h"
    "fn/callable.h"
    "fn/fn.h"
    "fn/fn_bind.h"
    "fn/fn_defn.h"
    "fn/fn_impl.h"
    "iter/__private/iterator_end.h"
    "iter/__private/iterator_loop.h"
    "iter/boxed_iterator.h"
    "iter/filter.h"
    "iter/from_iterator.h"
    "iter/iterator.h"
    "iter/iterator_defn.h"
    "iter/once.h"
    "iter/sized_iterator.h"
    "macros/__private/compiler_bugs.h"
    "macros/always_inline.h"
    "macros/builtin.h"
    "macros/compiler.h"
    "macros/eval_macro.h"
    "macros/for_each.h"
    "macros/no_unique_address.h"
    "macros/nonnull.h"
    "macros/remove_parens.h"
    "marker/unsafe.h"
    "mem/__private/data_size_finder.h"
    "mem/__private/relocatable_storage.h"
    "mem/addressof.h"
    "mem/clone.h"
    "mem/copy.h"
    "mem/forward.h"
    "mem/move.h"
    "mem/mref.h"
    "mem/never_value.h"
    "mem/nonnull.h"
    "mem/relocate.h"
    "mem/replace.h"
    "mem/size_of.h"
    "mem/swap.h"
    "mem/take.h"
    "num/__private/float_consts.h"
    "num/__private/float_macros.h"
    "num/__private/float_ordering.h"
    "num/__private/intrinsics.h"
    "num/__private/literals.h"
    "num/__private/ptr_type.h"
    "num/__private/signed_integer_macros.h"
    "num/__private/unsigned_integer_macros.h"
    "num/float.h"
    "num/float_concepts.h"
    "num/fp_category.h"
    "num/integer_concepts.h"
    "num/signed_integer.h"
    "num/try_from_int_error.h"
    "num/num_concepts.h"
    "num/unsigned_integer.h"
    "option/__private/is_option_type.h"
    "option/__private/is_tuple_type.h"
    "option/__private/marker.h"
    "option/__private/storage.h"
    "option/option.h"
    "option/state.h"
    "ops/eq.h"
    "ops/ord.h"
    "result/__private/is_result_type.h"
    "result/__private/marker.h"
    "result/__private/storage.h"
    "result/result.h"
    "tuple/__private/storage.h"
    "tuple/tuple.h"
    "lib/lib.cc"
)

add_library(subspace_test_support STATIC
    "test/from_i32.h"
    "test/behaviour_types.h"
    "test/behaviour_types_unittest.cc"
    "test/no_copy_move.h"
)

add_executable(subspace_unittests
    "assertions/check_unittest.cc"
    "assertions/endian_unittest.cc"
    "assertions/panic_unittest.cc"
    "assertions/unreachable_unittest.cc"
    "choice/choice_unittest.cc"
    "convert/subclass_unittest.cc"
    "containers/array_unittest.cc"
    "containers/slice_unittest.cc"
    "containers/vec_unittest.cc"
    "construct/from_unittest.cc"
    "construct/into_unittest.cc"
    "construct/default_unittest.cc"
    "fn/fn_unittest.cc"
    "iter/iterator_unittest.cc"
    "mem/addressof_unittest.cc"
    "mem/clone_unittest.cc"
    "mem/move_unittest.cc"
    "mem/nonnull_unittest.cc"
    "mem/nonnull_types_unittest.cc"
    "mem/relocate_unittest.cc"
    "mem/replace_unittest.cc"
    "mem/size_of_unittest.cc"
    "mem/swap_unittest.cc"
    "mem/take_unittest.cc"
    "num/__private/literals_unittest.cc"
    "num/f32_unittest.cc"
    "num/f64_unittest.cc"
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
    "ops/eq_unittest.cc"
    "ops/ord_unittest.cc"
    "result/result_unittest.cc"
    "result/result_types_unittest.cc"
    "tuple/tuple_unittest.cc"
)

# Subspace library
set_target_properties(subspace PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(subspace PUBLIC ..)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND
   CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
    # TODO: https://github.com/llvm/llvm-project/issues/59689
    # clang-cl requires this define in order to use offsetof() from constant
    # expressions, which is needed for the never-value optimization.
    target_compile_options(subspace PUBLIC /D_CRT_USE_BUILTIN_OFFSETOF)
endif()

# Subspace test support
set_target_properties(subspace_test_support PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(subspace_test_support PRIVATE .)
target_link_libraries(subspace_test_support
    subspace
)

# Subspace unittests
set_target_properties(subspace_unittests PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(subspace_unittests PRIVATE
    .
    ../third_party/googletest)
target_link_libraries(subspace_unittests
    subspace
    subspace_test_support
    gtest_main
)

# -Werror when building unittests to verify no warnings in the subspace
# library.
if(MSVC)
    target_compile_options(subspace_unittests PUBLIC /W4 /WX)
else()
    target_compile_options(subspace_unittests PUBLIC -Werror)
endif()

gtest_discover_tests(
    subspace_unittests
)
