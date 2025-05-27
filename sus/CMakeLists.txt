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

# Builds the Subspace library. Since the library depends on configuration macros,
# we need to rebuild the library any time those macros are used, so that we can
# test the modules build.
function(build_subspace_library)
    cmake_parse_arguments(SUS
        ""       # No-arg options
        "NAME"   # Single-arg options
        "DEFINE" # Multi-arg options
        ${ARGN}  # Parameters to parse
    )

    add_library(${SUS_NAME} STATIC
        "assertions/panic.cc"
        "env/var.cc"
        "lib/lib.cc"
    )

    target_link_libraries(${SUS_NAME}
        fmt::fmt
    )

    subspace_default_compile_options(${SUS_NAME})
    target_sources(${SUS_NAME} PUBLIC
        "assertions/check.h"
        "assertions/debug_check.h"
        "assertions/panic.h"
        "assertions/panic.cc"
        "assertions/unreachable.h"
        "boxed/box.h"
        "boxed/dyn.h"
        "boxed/macros.h"
        "choice/__private/all_values_are_unique.h"
        "choice/__private/index_of_value.h"
        "choice/__private/index_type.h"
        "choice/__private/nothing.h"
        "choice/__private/ops_concepts.h"
        "choice/__private/pack_index.h"
        "choice/__private/storage.h"
        "choice/__private/type_list.h"
        "choice/choice.h"
        "choice/choice_types.h"
        "choice/macros.h"
        "cmp/__private/void_concepts.h"
        "cmp/cmp.h"
        "cmp/eq.h"
        "cmp/ord.h"
        "cmp/reverse.h"
        "construct/__private/into_ref.h"
        "construct/construct.h"
        "construct/from.h"
        "construct/into.h"
        "construct/default.h"
        "construct/safe_from_reference.h"
        "construct/cast.h"
        "collections/__private/slice_methods_impl.inc"
        "collections/__private/slice_methods.inc"
        "collections/__private/slice_mut_methods.inc"
        "collections/__private/sort.h"
        "collections/iterators/array_iter.h"
        "collections/iterators/chunks.h"
        "collections/iterators/drain.h"
        "collections/iterators/slice_iter.h"
        "collections/iterators/vec_iter.h"
        "collections/iterators/windows.h"
        "collections/array.h"
        "collections/collections.h"
        "collections/compat_deque.h"
        "collections/compat_forward_list.h"
        "collections/compat_list.h"
        "collections/compat_map.h"
        "collections/compat_queue.h"
        "collections/compat_set.h"
        "collections/compat_stack.h"
        "collections/compat_unordered_map.h"
        "collections/compat_unordered_set.h"
        "collections/compat_vector.h"
        "collections/concat.h"
        "collections/join.h"
        "collections/slice.h"
        "collections/vec.h"
        "env/env.h"
        "env/var.cc"
        "env/var.h"
        "error/compat_error.h"
        "error/error.h"
        "fn/__private/signature.h"
        "fn/fn.h"
        "fn/fn_dyn.h"
        "iter/__private/into_iterator_archetype.h"
        "iter/__private/is_generator.h"
        "iter/__private/iter_compare.h"
        "iter/__private/iterator_end.h"
        "iter/__private/step.h"
        "iter/adaptors/by_ref.h"
        "iter/adaptors/chain.h"
        "iter/adaptors/cloned.h"
        "iter/adaptors/copied.h"
        "iter/adaptors/cycle.h"
        "iter/adaptors/enumerate.h"
        "iter/adaptors/filter.h"
        "iter/adaptors/filter_map.h"
        "iter/adaptors/flat_map.h"
        "iter/adaptors/flatten.h"
        "iter/adaptors/fuse.h"
        "iter/adaptors/inspect.h"
        "iter/adaptors/map.h"
        "iter/adaptors/map_while.h"
        "iter/adaptors/moved.h"
        "iter/adaptors/peekable.h"
        "iter/adaptors/reverse.h"
        "iter/adaptors/scan.h"
        "iter/adaptors/skip.h"
        "iter/adaptors/skip_while.h"
        "iter/adaptors/step_by.h"
        "iter/adaptors/take.h"
        "iter/adaptors/take_while.h"
        "iter/adaptors/zip.h"
        "iter/compat_ranges.h"
        "iter/extend.h"
        "iter/from_iterator.h"
        "iter/generator.h"
        "iter/into_iterator.h"
        "iter/iterator.h"
        "iter/iterator_concept.h"
        "iter/iterator_defn.h"
        "iter/iterator_impl.h"
        "iter/iterator_loop.h"
        "iter/iterator_ref.h"
        "iter/once.h"
        "iter/product.h"
        "iter/repeat.h"
        "iter/repeat_with.h"
        "iter/size_hint.h"
        "iter/size_hint_impl.h"
        "iter/successors.h"
        "iter/try_from_iterator.h"
        "iter/zip.h"
        "macros/__private/compiler_bugs.h"
        "macros/assume.h"
        "macros/arch.h"
        "macros/builtin.h"
        "macros/compiler.h"
        "macros/eval_macro.h"
        "macros/for_each.h"
        "macros/inline.h"
        "macros/lifetimebound.h"
        "macros/no_unique_address.h"
        "macros/nonnull.h"
        "macros/remove_parens.h"
        "marker/empty.h"
        "marker/unsafe.h"
        "mem/__private/data_size_finder.h"
        "mem/__private/ref_concepts.h"
        "mem/addressof.h"
        "mem/clone.h"
        "mem/copy.h"
        "mem/forward.h"
        "mem/move.h"
        "mem/never_value_macros.h"
        "mem/never_value.h"
        "mem/relocate_macros.h"
        "mem/relocate.h"
        "mem/remove_rvalue_reference.h"
        "mem/replace.h"
        "mem/size_of.h"
        "mem/swap.h"
        "mem/take.h"
        "num/__private/check_integer_overflow.h"
        "num/__private/float_consts.inc"
        "num/__private/float_methods.inc"
        "num/__private/float_methods_impl.inc"
        "num/__private/float_ordering.h"
        "num/__private/intrinsics.h"
        "num/__private/literals.h"
        "num/__private/primitive_type.h"
        "num/__private/signed_integer_consts.inc"
        "num/__private/signed_integer_methods.inc"
        "num/__private/signed_integer_methods_impl.inc"
        "num/__private/unsigned_integer_consts.inc"
        "num/__private/unsigned_integer_methods.inc"
        "num/__private/unsigned_integer_methods_impl.inc"
        "num/cast.h"
        "num/float.h"
        "num/float_concepts.h"
        "num/float_impl.h"
        "num/fp_category.h"
        "num/integer_concepts.h"
        "num/overflow_integer.h"
        "num/signed_integer.h"
        "num/signed_integer_impl.h"
        "num/try_from_int_error.h"
        "num/try_from_int_error_impl.h"
        "num/num_concepts.h"
        "num/unsigned_integer.h"
        "num/unsigned_integer_impl.h"
        "option/__private/is_option_type.h"
        "option/__private/is_tuple_type.h"
        "option/__private/marker.h"
        "option/__private/storage.h"
        "option/option_iter.h"
        "option/option.h"
        "option/compat_option.h"
        "option/state.h"
        "ops/range.h"
        "ops/range_literals.h"
        "ptr/copy.h"
        "ptr/nonnull.h"
        "ptr/as_ref.h"
        "ptr/subclass.h"
        "ptr/swap.h"
        "result/__private/is_result_type.h"
        "result/__private/marker.h"
        "result/__private/storage.h"
        "result/ok_void.h"
        "result/result.h"
        "string/__private/any_formatter.h"
        "string/__private/bytes_formatter.h"
        "string/__private/format_to_stream.h"
        "string/compat_string.h"
        "tuple/__private/storage.h"
        "tuple/tuple.h"
        "lib/lib.h"
        "lib/lib.cc"
    )
    target_sources(${SUS_NAME} PUBLIC
        FILE_SET CXX_MODULES
        FILES "sus.cc"
    )
    target_compile_definitions(${SUS_NAME} PRIVATE
        ${SUS_DEFINE}
    )
    add_library(${SUS_NAME}::lib ALIAS ${SUS_NAME})

    if(SUBSPACE_BUILD_TESTS)
        # Subspace test support
        add_library(${SUS_NAME}_test_support STATIC "")
        add_library(${SUS_NAME}::test_support ALIAS ${SUS_NAME}_test_support)
        target_sources(${SUS_NAME}_test_support PUBLIC
            "test/behaviour_types.h"
            "test/behaviour_types_unittest.cc"
            "test/ensure_use.cc"
            "test/ensure_use.h"
            "test/no_copy_move.h"
        )
        subspace_test_default_compile_options(${SUS_NAME}_test_support)
        target_link_libraries(${SUS_NAME}_test_support PRIVATE ${SUS_NAME}::lib)
    endif()
endfunction()

build_subspace_library(NAME subspace)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND
   CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
    # TODO: https://github.com/llvm/llvm-project/issues/59689
    # clang-cl requires this define in order to use offsetof() from constant
    # expressions, which is needed for the never-value optimization.
    target_compile_options(subspace PUBLIC /D_CRT_USE_BUILTIN_OFFSETOF)
    # See subdoc/lib/run.cc, these are needed to have clang-cl parse Subspace.
    target_compile_options(subspace PUBLIC /D__cpp_consteval)
    target_compile_options(subspace PUBLIC /w)
endif()

include(CMakeParseArguments)

function(add_subspace_test)
    cmake_parse_arguments(SUS
        ""                                 # No-arg options
        "NAME"                             # Single-arg options
        "SRCS;DEFINE;COMPILE_OPTIONS;DEPS" # Multi-arg options
        ${ARGN}                            # Parameters to parse
    )

    add_executable(${SUS_NAME} ${SUS_SRCS})
    subspace_test_default_compile_options(${SUS_NAME})
    target_compile_definitions(${SUS_NAME} PRIVATE ${SUS_DEFINE})
    target_compile_options(${SUS_NAME} PRIVATE ${SUS_COMPILE_OPTIONS})
    target_link_libraries(${SUS_NAME} PRIVATE
        subspace::lib
        subspace::test_support
        fmt::fmt
        gtest_main
    )

    set(test_module_name "${SUS_NAME}_module")
    add_executable(${test_module_name} ${SUS_SRCS})
    subspace_test_default_compile_options(${test_module_name})
    target_compile_definitions(${test_module_name} PRIVATE
        TEST_MODULE
        ${SUS_DEFINE}
    )
    target_compile_options(${test_module_name} PRIVATE ${SUS_COMPILE_OPTIONS})
    target_link_libraries(${test_module_name} PRIVATE
        fmt::fmt
        gtest_main
    )

    if(SUS_DEFINE)
        set(library_module_name "subspace_${SUS_NAME}")
        build_subspace_library(NAME ${library_module_name}
            DEFINE ${SUS_DEFINE}
        )
        target_link_libraries(${test_module_name} PRIVATE
            ${library_module_name}
            ${library_module_name}::test_support
        )
    else()
        target_link_libraries(${test_module_name} PRIVATE
            subspace::lib
            subspace::test_support
        )
    endif()

    gtest_discover_tests(${SUS_NAME})
    gtest_discover_tests(${test_module_name})
endfunction()

if(${SUBSPACE_BUILD_TESTS})
    set(subspace_unittest_list
        "assertions/check_unittest.cc"
        "assertions/panic_unittest.cc"
        "assertions/unreachable_unittest.cc"
        "boxed/box_unittest.cc"
        "boxed/dyn_unittest.cc"
        "choice/choice_types_unittest.cc"
        "choice/choice_unittest.cc"
        "cmp/eq_unittest.cc"
        "cmp/ord_unittest.cc"
        "cmp/reverse_unittest.cc"
        "construct/cast_unittest.cc"
        "collections/array_unittest.cc"
        "collections/compat_deque_unittest.cc"
        "collections/compat_forward_list_unittest.cc"
        "collections/compat_list_unittest.cc"
        "collections/compat_map_unittest.cc"
        "collections/compat_queue_unittest.cc"
        "collections/compat_set_unittest.cc"
        "collections/compat_stack_unittest.cc"
        "collections/compat_unordered_map_unittest.cc"
        "collections/compat_unordered_set_unittest.cc"
        "collections/compat_vector_unittest.cc"
        "collections/slice_unittest.cc"
        "collections/vec_unittest.cc"
        "construct/from_unittest.cc"
        "construct/into_unittest.cc"
        "construct/default_unittest.cc"
        "env/var_unittest.cc"
        "error/error_unittest.cc"
        "fn/fn_concepts_unittest.cc"
        "fn/fn_dyn_unittest.cc"
        "iter/compat_ranges_unittest.cc"
        "iter/empty_unittest.cc"
        "iter/generator_unittest.cc"
        "iter/iterator_unittest.cc"
        "iter/once_unittest.cc"
        "iter/once_with_unittest.cc"
        "iter/repeat_unittest.cc"
        "iter/repeat_with_unittest.cc"
        "iter/successors_unittest.cc"
        "marker/unsafe_unittest.cc"
        "mem/addressof_unittest.cc"
        "mem/clone_unittest.cc"
        "mem/move_unittest.cc"
        "mem/relocate_unittest.cc"
        "mem/replace_unittest.cc"
        "mem/size_of_unittest.cc"
        "mem/swap_unittest.cc"
        "mem/take_unittest.cc"
        "num/__private/literals_unittest.cc"
        "num/cast_unittest.cc"
        "num/f32_unittest.cc"
        "num/f64_unittest.cc"
        "num/i8_unittest.cc"
        "num/i16_unittest.cc"
        "num/i32_unittest.cc"
        "num/i64_unittest.cc"
        "num/isize_unittest.cc"
        "num/overflow_integer_unittest.cc"
        "num/u8_unittest.cc"
        "num/u16_unittest.cc"
        "num/u32_unittest.cc"
        "num/u64_unittest.cc"
        "num/uptr_unittest.cc"
        "num/usize_unittest.cc"
        "option/option_unittest.cc"
        "option/compat_option_unittest.cc"
        "option/option_types_unittest.cc"
        "ops/range_unittest.cc"
        "ptr/nonnull_unittest.cc"
        "ptr/nonnull_types_unittest.cc"
        "ptr/as_ref_unittest.cc"
        "ptr/subclass_unittest.cc"
        "ptr/swap_unittest.cc"
        "result/result_unittest.cc"
        "result/result_types_unittest.cc"
        "string/__private/format_to_stream_unittest.cc"
        "string/compat_string_unittest.cc"
        "tuple/tuple_types_unittest.cc"
        "tuple/tuple_unittest.cc"
    )

    add_subspace_test(NAME subspace_unittests
        SRCS ${subspace_unittest_list}
    )
    add_subspace_test(NAME subspace_iterator_invalidation_off_size_unittest
        SRCS "collections/invalidation_off_size_unittest.cc"
        DEFINE SUS_ITERATOR_INVALIDATION=0
    )
    add_subspace_test(NAME subspace_iterator_invalidation_on_size_unittest
        SRCS "collections/invalidation_on_size_unittest.cc"
        DEFINE SUS_ITERATOR_INVALIDATION=1
    )

    add_subspace_test(NAME subspace_overflow_unittests
        SRCS
            "num/i8_overflow_unittest.cc"
            "num/i16_overflow_unittest.cc"
            "num/i32_overflow_unittest.cc"
            "num/i64_overflow_unittest.cc"
            "num/isize_overflow_unittest.cc"
            "num/u8_overflow_unittest.cc"
            "num/u16_overflow_unittest.cc"
            "num/u32_overflow_unittest.cc"
            "num/u64_overflow_unittest.cc"
            "num/uptr_overflow_unittest.cc"
            "num/usize_overflow_unittest.cc"
        DEFINE SUS_CHECK_INTEGER_OVERFLOW=false
    )
endif()
