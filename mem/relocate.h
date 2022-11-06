// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <type_traits>

#include "macros/builtin.h"
#include "marker/unsafe.h"

namespace sus::mem {

namespace __private {

template <class T>
struct relocatable_tag final {
  static constexpr bool value(...) { return false; }

  static constexpr bool value(int)
    requires(std::is_same_v<
             std::remove_cv_t<decltype(T::SusUnsafeTrivialRelocate)>, bool>)
  {
    return T::SusUnsafeTrivialRelocate;
  };
};

// Tests if the type T can be relocated with memcpy(). Checking for trivially
// movable and destructible is not sufficient - this also honors the
// [[trivial_abi]] clang attribute, as types annotated with the attribute are
// now considered "trivially relocatable" in https://reviews.llvm.org/D114732.
//
// TODO: @ssbr has pointed out that this must also verify that the type has no
// padding at the end which can be used by an outer type. If so, either
// - You can't memcpy it, or
// - You must memcpy without including the padding bytes.
//
// If type `T` has padding at its end, such as:
// ```
// class T { i64 a; i32 b; };
// ```
//
// Then there are two ways for another type to place a field inside the padding
// adjacent to `b` and inside area allocated for `sizeof(T)`:
//
// 1. A subclass of a non-POD type can insert its fields into the padding of the
//    base class.
//
// So a subclass of `T` may have its first field inside the padding adjacent to
// `b`:
// ```
// class S : T { i32 c; };
// ```
// In this example, `sizeof(S) == sizeof(T)` because `c` sits inside the
// trailing padding of `T`.
//
// 2. A class with a `[[no_unique_address]]` field may insert other fields below
//    it into the padding of the `[[no_unique_address]]` field.
//
// So a class that contains `T` as a field can insert another field into `T`:
// ```
// class S {
//   [[no_unique_address]] T t;
//   i32 c;
// };
// ```
// In this example, `sizeof(S) == sizeof(T)` because `c` sits inside the
// trailing padding of `T`.
//
// From @ssbr:
//
// So the dsizeof(T) algorithm [to determine how much to memcpy safely] is
// something like:
//
// - A: find out how many bytes fit into the padding via inheritance (`struct S
//   : T { bytes }` for all `bytes` until `sizeof(T) != sizeof(S)`).
// - B: find out how many bytes fit into the padding via no_unique_address
//   (struct S { [[no_unique_address]] T x; bytes } for all `bytes` until
//   `sizeof(T) != sizeof(S)`).
//
// ```
// return sizeof(T) - max(A, B)
// ```
//
// And I think on every known platform, A == B. It might even be guaranteed by
// the standard, but I wouldn't know how to check
//
// clang-format off
template <class... T>
struct relocate_one_by_memcpy_helper final
    : public std::integral_constant<
        bool,
        (... && 
         (relocatable_tag<T>::value(0)
#if __has_extension(trivially_relocatable)
          || __is_trivially_relocatable(T)
#else
          || (std::is_trivially_move_constructible_v<T> &&
              std::is_trivially_destructible_v<T>)
#endif
         )
        )
      > {};
// clang-format on

// Tests if an array of type T[] can be relocated with memcpy(). Checking for
// trivially movable and destructible is not sufficient - this also honors the
// [[trivial_abi]] clang attribute, as types annotated with the attribute are
// now considered "trivially relocatable" in https://reviews.llvm.org/D114732.
//
// Tests against `std::remove_all_extents_t<T>` so that the same answer is
// returned for `T` or `T[]` or `T[][][]` etc.
//
// Volatile types are excluded, since if we have a range of volatile Foo, then
// the user is probably expecting us to follow the abstract machine and copy the
// Foo objects one by one, instead of byte-by-byte (possible tearing). See:
// https://reviews.llvm.org/D61761?id=198907#inline-548830
//
// clang-format off
template <class T>
struct relocate_array_by_memcpy_helper final
    : public std::integral_constant<
        bool,
        (relocatable_tag<T>::value(0)
#if __has_extension(trivially_relocatable)
         || __is_trivially_relocatable(std::remove_all_extents_t<T>)
#else
         || (std::is_trivially_move_constructible_v<std::remove_all_extents_t<T>>
             && std::is_trivially_destructible_v<std::remove_all_extents_t<T>>)
#endif
        )
        && !std::is_volatile_v<std::remove_all_extents_t<T>>
      > {};
// clang-format on

}  // namespace __private

template <class T>
concept relocate_array_by_memcpy =
    __private::relocate_array_by_memcpy_helper<T>::value;

template <class... T>
concept relocate_one_by_memcpy =
    __private::relocate_one_by_memcpy_helper<T...>::value;

}  // namespace sus::mem

#if defined(__clang__)
/// An attribute to allow a class to be passed in registers.
///
/// This should only be used when the class is also marked as unconditionally
/// relocatable with `sus_class_trivial_relocatable()`.
///
/// This also enables trivial relocation in libc++ if compiled with clang.
#define sus_trivial_abi clang::trivial_abi
#else
/// An attribute to allow a class to be passed in registers.
///
/// This should only be used when the class is also marked as unconditionally
/// relocatable with `sus_class_trivial_relocatable()`.
///
/// This also enables trivial relocation in libc++ if compiled with clang.
#define sus_trivial_abi
#endif

/// Mark a class as trivially relocatable.
///
/// To additionally allow the class to be passed in registers, the class can be
/// marked with the `sus_trivial_abi` attribute.
#define sus_class_trivial_relocatable(unsafe_fn)                      \
  static_assert(std::is_same_v<decltype(unsafe_fn),                   \
                               const ::sus::marker::UnsafeFnMarker>); \
  template <class SusOuterClassTypeForTriviallyReloc>                 \
  friend struct ::sus::mem::__private::relocatable_tag;               \
  static constexpr bool SusUnsafeTrivialRelocate = true

/// Mark a class as trivially relocatable based on a compile-time condition.
#define sus_class_trivial_relocatable_value(unsafe_fn, is_trivially_reloc)   \
  static_assert(std::is_same_v<decltype(unsafe_fn),                          \
                               const ::sus::marker::UnsafeFnMarker>);        \
  static_assert(                                                             \
      std::is_same_v<std::remove_cv_t<decltype(is_trivially_reloc)>, bool>); \
  template <class SusOuterClassTypeForTriviallyReloc>                        \
  friend struct ::sus::mem::__private::relocatable_tag;                      \
  static constexpr bool SusUnsafeTrivialRelocate = is_trivially_reloc

/// Mark a class as trivially relocatable if all of the types passed as
/// arguments are also marked as such.
#define sus_class_maybe_trivial_relocatable_types(unsafe_fn, ...)     \
  static_assert(std::is_same_v<decltype(unsafe_fn),                   \
                               const ::sus::marker::UnsafeFnMarker>); \
  template <class SusOuterClassTypeForTriviallyReloc>                 \
  friend struct ::sus::mem::__private::relocatable_tag;               \
  static constexpr bool SusUnsafeTrivialRelocate =                    \
      ::sus::mem::relocate_one_by_memcpy<__VA_ARGS__>

/// Mark a class as unconditionally trivially relocatable while also asserting
/// that all of the types passed as arguments are also marked as such.
///
/// To additionally allow the class to be passed in registers, the class can be
/// marked with the `sus_trivial_abi` attribute.
#define sus_class_assert_trivial_relocatable_types(unsafe_fn, ...)   \
  sus_class_maybe_trivial_relocatable_types(unsafe_fn, __VA_ARGS__); \
  static_assert(SusUnsafeTrivialRelocate,                            \
                "Type is not trivially "                             \
                "relocatable");
