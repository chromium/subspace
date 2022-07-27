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

#include "assertions/builtin.h"
#include "marker/unsafe.h"

namespace sus::mem::__private {

template <class T>
struct relocatable_tag {
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
// TODO: This must also verify that the type has no padding at the end which can
// be used by an outer type. If so, either a) You can't memcpy it, or b) You
// must memcpy without including the padding bytes.
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
// clang-format off
template <class... T>
struct relocate_one_by_memcpy
    : public std::integral_constant<
        bool,
#if __has_extension(trivially_relocatable)
        (... && __is_trivially_relocatable(T))
#else
        (... && relocatable_tag<T>::value(0))
        || (... && (std::is_trivially_move_constructible_v<T> &&
            std::is_trivially_destructible_v<T>))
#endif
      > {};
// clang-format on

template <class... T>
inline constexpr bool relocate_one_by_memcpy_v =
    relocate_one_by_memcpy<T...>::value;

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
struct relocate_array_by_memcpy
    : public std::integral_constant<
        bool,
#if __has_extension(trivially_relocatable)
        __is_trivially_relocatable(std::remove_all_extents_t<T>)
        && !std::is_volatile_v<std::remove_all_extents_t<T>>
#else
        (relocatable_tag<T>::value(0)
         || (std::is_trivially_move_constructible_v<std::remove_all_extents_t<T>>
             && std::is_trivially_destructible_v<std::remove_all_extents_t<T>>))
        && !std::is_volatile_v<std::remove_all_extents_t<T>>
#endif
      > {};
// clang-format on

template <class T>
inline constexpr bool relocate_array_by_memcpy_v =
    relocate_array_by_memcpy<T>::value;

}  // namespace sus::mem::__private

#if defined(__clang__)
#define sus_trivial_abi clang::trivial_abi
#else
#define sus_trivial_abi
#endif

#define sus_class_trivial_relocatable(unsafe_fn)                     \
  static_assert(std::is_same_v<decltype(unsafe_fn),                  \
                               const ::sus::marker::UnsafeFnMarker>, \
                "");                                                 \
  template <class SusOuterClassTypeForTriviallyReloc>                \
  friend struct ::sus::mem::__private::relocatable_tag;              \
  static constexpr bool SusUnsafeTrivialRelocate = true

#define sus_class_trivial_relocatable_value(unsafe_fn, is_trivially_reloc)  \
  static_assert(std::is_same_v<decltype(unsafe_fn),                         \
                               const ::sus::marker::UnsafeFnMarker>,        \
                "");                                                        \
  static_assert(                                                            \
      std::is_same_v<std::remove_cv_t<decltype(is_trivially_reloc)>, bool>, \
      "");                                                                  \
  template <class SusOuterClassTypeForTriviallyReloc>                       \
  friend struct ::sus::mem::__private::relocatable_tag;                     \
  static constexpr bool SusUnsafeTrivialRelocate = is_trivially_reloc

#define sus_class_maybe_trivial_relocatable_types(unsafe_fn, ...)    \
  static_assert(std::is_same_v<decltype(unsafe_fn),                  \
                               const ::sus::marker::UnsafeFnMarker>, \
                "");                                                 \
  template <class SusOuterClassTypeForTriviallyReloc>                \
  friend struct ::sus::mem::__private::relocatable_tag;              \
  static constexpr bool SusUnsafeTrivialRelocate =                   \
      ::sus::mem::__private::relocate_one_by_memcpy_v<__VA_ARGS__>

#define sus_class_assert_trivial_relocatable_types(unsafe_fn, ...)   \
  sus_class_maybe_trivial_relocatable_types(unsafe_fn, __VA_ARGS__); \
  static_assert(SusUnsafeTrivialRelocate,                            \
                "Type is not trivially "                             \
                "relocatable");
