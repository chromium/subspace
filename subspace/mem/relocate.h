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
#include "subspace/macros/compiler.h"
#include "subspace/marker/unsafe.h"
#include "subspace/mem/size_of.h"

namespace sus::mem {

namespace __private {

/// Detects the presence and value of the static const member
/// `T::SusUnsafeTrivialRelocate`.
///
/// The static member is created by `sus_class_trivially_relocatable()` and
/// similar macros.
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

}  // namespace __private

/// Tests if a variable of type T can be relocated with memcpy(). Checking for
/// trivially movable and destructible is not sufficient - this also honors the
/// [[trivial_abi]] clang attribute, as types annotated with the attribute are
/// now considered "trivially relocatable" in https://reviews.llvm.org/D114732.
/// References are treated like pointers, and are always trivially relocatable,
/// as reference data members are relocatable in the same way pointers are.
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
/// Volatile types are excluded, as they can not be safely memcpy()'d
/// byte-by-byte without introducing tearing.
///
/// Tests against `std::remove_all_extents_t<T>` so that the same answer is
/// returned for `T` or `T[]` or `T[][][]` etc.
//
// clang-format off
template <class... T>
concept relocate_by_memcpy =
  (... &&
    (std::is_reference_v<T>
    || (!std::is_volatile_v<std::remove_all_extents_t<T>>
       && sus::mem::data_size_of<std::remove_all_extents_t<std::remove_reference_t<T>>>() != size_t(-1)
       && (__private::relocatable_tag<std::remove_all_extents_t<T>>::value(0)
          || std::is_trivially_copyable_v<std::remove_all_extents_t<T>>
          || (std::is_trivially_move_constructible_v<std::remove_all_extents_t<T>> &&
              std::is_trivially_move_assignable_v<std::remove_all_extents_t<T>> &&
              std::is_trivially_destructible_v<std::remove_all_extents_t<T>>)
#if __has_extension(trivially_relocatable)
          || __is_trivially_relocatable(std::remove_all_extents_t<T>)
#endif
          )
       )
    )
  );
// clang-format on

}  // namespace sus::mem

/// An attribute to allow a class to be passed in registers.
///
/// This should only be used when the class is also marked as unconditionally
/// relocatable with `sus_class_trivially_relocatable()` or
/// `sus_class_trivially_relocatable_unchecked()`.
///
/// This also enables trivial relocation in libc++ if compiled with clang.
#define sus_trivial_abi sus_if_clang(clang::trivial_abi)

/// Mark a class as unconditionally trivially relocatable while also asserting
/// that all of the types passed as arguments are also marked as such.
///
/// Typically all field types in the class should be passed to the macro as
/// its arguments.
///
/// To additionally allow the class to be passed in registers, the class can be
/// marked with the `sus_trivial_abi` attribute.
///
/// # Example
/// ```
/// struct S {
///   Thing<i32> thing;
///   i32 more;
///
///   sus_class_trivially_relocatable(
///       unsafe_fn,
///       decltype(thing),
///       decltype(more));
/// };
/// ```
#define sus_class_trivially_relocatable(unsafe_fn, ...)               \
  static_assert(std::is_same_v<decltype(unsafe_fn),                   \
                               const ::sus::marker::UnsafeFnMarker>); \
  sus_class_trivially_relocatable_if_types(unsafe_fn, __VA_ARGS__);   \
  static_assert(SusUnsafeTrivialRelocate, "Type is not trivially relocatable")

/// Mark a class as trivially relocatable if the types passed as arguments are
/// all trivially relocatable.
///
/// This macro is most useful in templates where the template parameter types
/// are unknown and can be passed to the macro to determine if they are
/// trivially relocatable.
///
/// Avoid marking the class with the `sus_trivial_abi` attribute, as when the
/// class is not trivially relocatable (since a subtype is not trivially
/// relocatable), it can cause memory safety bugs and Undefined Behaviour.
///
/// # Example
/// ```
/// template <class T>
/// struct S {
///   Thing<i32> thing;
///   i32 more;
///
///   sus_class_trivially_relocatable_if_types(
///       unsafe_fn,
///       decltype(thing),
///       decltype(more));
/// };
/// ```
#define sus_class_trivially_relocatable_if_types(unsafe_fn, ...)      \
  static_assert(std::is_same_v<decltype(unsafe_fn),                   \
                               const ::sus::marker::UnsafeFnMarker>); \
  template <class SusOuterClassTypeForTriviallyReloc>                 \
  friend struct ::sus::mem::__private::relocatable_tag;               \
  static constexpr bool SusUnsafeTrivialRelocate =                    \
      ::sus::mem::relocate_by_memcpy<__VA_ARGS__>

/// Mark a class as trivially relocatable based on a compile-time condition.
///
/// This macro is most useful in templates where the condition is based on the
/// template parameters.
///
/// Avoid marking the class with the `sus_trivial_abi` attribute, as when the
/// class is not trivially relocatable (the value is false), it can cause
/// memory safety bugs and Undefined Behaviour.
///
/// # Example
/// ```
/// template <class T>
/// struct S {
///   Thing<i32> thing;
///   i32 more;
///
///   sus_class_trivially_relocatable_if(
///       unsafe_fn,
///       SomeCondition<Thing<T>>
///       && sus::mem::relocate_by_memcpy<decltype(thing)>
///       && sus::mem::relocate_by_memcpy<decltype(more)>);
/// };
/// ```
#define sus_class_trivially_relocatable_if(unsafe_fn, is_trivially_reloc)    \
  static_assert(std::is_same_v<decltype(unsafe_fn),                          \
                               const ::sus::marker::UnsafeFnMarker>);        \
  static_assert(                                                             \
      std::is_same_v<std::remove_cv_t<decltype(is_trivially_reloc)>, bool>); \
  template <class SusOuterClassTypeForTriviallyReloc>                        \
  friend struct ::sus::mem::__private::relocatable_tag;                      \
  static constexpr bool SusUnsafeTrivialRelocate = is_trivially_reloc

/// Mark a class as unconditionally trivially relocatable, without any
/// additional assertion to help verify correctness.
///
/// Genrally, prefer to use sus_class_trivially_relocatable() with
/// all field types passed to the macro.
///
/// To additionally allow the class to be passed in registers, the class can be
/// marked with the `sus_trivial_abi` attribute.
///
/// # Example
/// ```
/// struct S {
///   Thing<i32> thing;
///   i32 more;
///
///   sus_class_trivially_relocatable_unchecked(unsafe_fn);
/// };
/// ```
#define sus_class_trivially_relocatable_unchecked(unsafe_fn)          \
  static_assert(std::is_same_v<decltype(unsafe_fn),                   \
                               const ::sus::marker::UnsafeFnMarker>); \
  template <class SusOuterClassTypeForTriviallyReloc>                 \
  friend struct ::sus::mem::__private::relocatable_tag;               \
  static constexpr bool SusUnsafeTrivialRelocate = true
