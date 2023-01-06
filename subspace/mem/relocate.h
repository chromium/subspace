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

#include <concepts>
#include <type_traits>

#include "subspace/macros/builtin.h"
#include "subspace/marker/unsafe.h"
#include "subspace/mem/size_of.h"

namespace sus::mem {

namespace __private {

template <class T>
struct relocatable_tag final {
  static constexpr bool value(...) { return false; }

  static constexpr bool value(int)
    requires requires {
      requires(std::same_as<decltype(T::SusUnsafeTrivialRelocate), const bool>);
    }
  {
    return T::SusUnsafeTrivialRelocate;
  };
};

/// Tests if the type T can be relocated with memcpy(). Checking for trivially
/// movable and destructible is not sufficient - this also honors the
/// [[trivial_abi]] clang attribute, as types annotated with the attribute are
/// now considered "trivially relocatable" in https://reviews.llvm.org/D114732.
///
/// IMPORTANT: If a class satisfies this trait, only `sus::data_size_of<T>()`
/// bytes should be memcpy'd or Undefine Behaviour can result, due to the
/// possibility of overwriting data stored in the padding bytes of `T`.
///
/// As @ssbr has pointed out, if a type has any padding at the end which can be
/// used by an outer type, then either:
/// - You can't memcpy it, or
/// - You must memcpy without including the padding bytes (i.e.
///   `data_size_of<T>()` bytes).
///
//
// clang-format off
template <class... T>
struct relocate_one_by_memcpy_helper final
    : public std::integral_constant<
        bool,
        (... &&
         (sus::mem::data_size_of<T>()
          && (relocatable_tag<T>::value(0)
#if __has_extension(trivially_relocatable)
              || __is_trivially_relocatable(T)
#else
              || (std::is_trivially_move_constructible_v<T> &&
                  std::is_trivially_destructible_v<T>)
#endif
             )
         )
        )
      > {};
// clang-format on

/// Tests if an array of type T[] can be relocated with memcpy(). Checking for
/// trivially movable and destructible is not sufficient - this also honors the
/// [[trivial_abi]] clang attribute, as types annotated with the attribute are
/// now considered "trivially relocatable" in https://reviews.llvm.org/D114732.
///
/// Tests against `std::remove_all_extents_t<T>` so that the same answer is
/// returned for `T` or `T[]` or `T[][][]` etc.
///
/// Volatile types are excluded, since if we have a range of volatile Foo, then
/// the user is probably expecting us to follow the abstract machine and copy
/// the Foo objects one by one, instead of byte-by-byte (possible tearing). See:
/// https://reviews.llvm.org/D61761?id=198907#inline-548830
//
// clang-format off
template <class T>
struct relocate_array_by_memcpy_helper final
    : public std::integral_constant<
        bool,
        sus::mem::data_size_of<T>() &&
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
#define sus_class_trivial_relocatable(unsafe_fn)       \
  static_assert(std::is_same_v<decltype(unsafe_fn),    \
                               const ::sus::marker::UnsafeFnMarker>); \
  template <class SusOuterClassTypeForTriviallyReloc>                 \
  friend struct ::sus::mem::__private::relocatable_tag;               \
  static constexpr bool SusUnsafeTrivialRelocate = true

/// Mark a class as trivially relocatable based on a compile-time condition.
#define sus_class_trivial_relocatable_value(unsafe_fn,        \
                                            is_trivially_reloc)              \
  static_assert(std::is_same_v<decltype(unsafe_fn),           \
                               const ::sus::marker::UnsafeFnMarker>);        \
  static_assert(                                                             \
      std::is_same_v<std::remove_cv_t<decltype(is_trivially_reloc)>, bool>); \
  template <class SusOuterClassTypeForTriviallyReloc>                        \
  friend struct ::sus::mem::__private::relocatable_tag;                      \
  static constexpr bool SusUnsafeTrivialRelocate = is_trivially_reloc

/// Mark a class as trivially relocatable if all of the types passed as
/// arguments are also marked as such.
#define sus_class_maybe_trivial_relocatable_types(unsafe_fn, \
                                                  ...)                      \
  static_assert(std::is_same_v<decltype(unsafe_fn),          \
                               const ::sus::marker::UnsafeFnMarker>);       \
  template <class SusOuterClassTypeForTriviallyReloc>                       \
  friend struct ::sus::mem::__private::relocatable_tag;                     \
  static constexpr bool SusUnsafeTrivialRelocate =                          \
      ::sus::mem::relocate_one_by_memcpy<__VA_ARGS__>

/// Mark a class as unconditionally trivially relocatable while also asserting
/// that all of the types passed as arguments are also marked as such.
///
/// To additionally allow the class to be passed in registers, the class can be
/// marked with the `sus_trivial_abi` attribute.
#define sus_class_assert_trivial_relocatable_types(unsafe_fn, \
                                                   ...)                      \
  static_assert(std::is_same_v<decltype(unsafe_fn),           \
                               const ::sus::marker::UnsafeFnMarker>);        \
  sus_class_maybe_trivial_relocatable_types(unsafe_fn,        \
                                            __VA_ARGS__);                    \
  static_assert(SusUnsafeTrivialRelocate,                                    \
                "Type is not trivially "                                     \
                "relocatable")
