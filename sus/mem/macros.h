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

#include "sus/macros/compiler.h"

/// An attribute to allow a class to be passed in registers.
///
/// This should only be used when the class is also marked as unconditionally
/// relocatable with `sus_class_trivially_relocatable()` or
/// `sus_class_trivially_relocatable_unchecked()`.
///
/// This also enables trivial relocation in libc++ if compiled with clang.
#define _sus_trivial_abi sus_if_clang(clang::trivial_abi)

/// Mark a class as unconditionally trivially relocatable while also asserting
/// that all of the types passed as arguments are also marked as such.
///
/// Typically all field types in the class should be passed to the macro as
/// its arguments.
///
/// To additionally allow the class to be passed in registers, the class can be
/// marked with the `[[clang::trivial_abi]]` attribute.
///
/// Use the [`TriviallyRelocatable`]($sus::mem::TriviallyRelocatable) concept to
/// determine if a type is trivially relocatable, and to test with static_assert
/// that types are matching what you are expecting. This allows containers to
/// optimize their implementations when relocating the type in memory.
///
/// | Macro | Style |
/// | ----- | ----- |
/// | [`sus_class_trivially_relocatable`]($sus_class_trivially_relocatable) | **asserts** all param types are trivially relocatable |
/// | [`sus_class_trivially_relocatable_if_types`]($sus_class_trivially_relocatable_if_types) | is **conditionally** trivially relocatable if all param types are |
/// | [`sus_class_trivially_relocatable_if`]($sus_class_trivially_relocatable_if) | is **conditionally** trivially relocatable if the condition is true |
/// | [`sus_class_trivially_relocatable_unchecked`]($sus_class_trivially_relocatable_unchecked) | is trivially relocatable without any condition or assertion |
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
/// Avoid marking the class with the `[[clang::trivial_abi]]` attribute, as when
/// the class is not trivially relocatable (since a subtype is not trivially
/// relocatable), it can cause memory safety bugs and Undefined Behaviour.
///
/// Use the [`TriviallyRelocatable`]($sus::mem::TriviallyRelocatable) concept to
/// determine if a type is trivially relocatable, and to test with static_assert
/// that types are matching what you are expecting. This allows containers to
/// optimize their implementations when relocating the type in memory.
///
/// | Macro | Style |
/// | ----- | ----- |
/// | [`sus_class_trivially_relocatable`]($sus_class_trivially_relocatable) | **asserts** all param types are trivially relocatable |
/// | [`sus_class_trivially_relocatable_if_types`]($sus_class_trivially_relocatable_if_types) | is **conditionally** trivially relocatable if all param types are |
/// | [`sus_class_trivially_relocatable_if`]($sus_class_trivially_relocatable_if) | is **conditionally** trivially relocatable if the condition is true |
/// | [`sus_class_trivially_relocatable_unchecked`]($sus_class_trivially_relocatable_unchecked) | is trivially relocatable without any condition or assertion |
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
  friend struct ::sus::mem::__private::RelocatableTag;                \
  /** #[doc.hidden] */                                                \
  static constexpr bool SusUnsafeTrivialRelocate =                    \
      ::sus::mem::TriviallyRelocatable<__VA_ARGS__>

/// Mark a class as trivially relocatable based on a compile-time condition.
///
/// This macro is most useful in templates where the condition is based on the
/// template parameters.
///
/// Avoid marking the class with the `[[clang::trivial_abi]]` attribute, as when
/// the class is not trivially relocatable (the value is false), it can cause
/// memory safety bugs and Undefined Behaviour.
///
/// Use the [`TriviallyRelocatable`]($sus::mem::TriviallyRelocatable) concept to
/// determine if a type is trivially relocatable, and to test with static_assert
/// that types are matching what you are expecting. This allows containers to
/// optimize their implementations when relocating the type in memory.
///
/// | Macro | Style |
/// | ----- | ----- |
/// | [`sus_class_trivially_relocatable`]($sus_class_trivially_relocatable) | **asserts** all param types are trivially relocatable |
/// | [`sus_class_trivially_relocatable_if_types`]($sus_class_trivially_relocatable_if_types) | is **conditionally** trivially relocatable if all param types are |
/// | [`sus_class_trivially_relocatable_if`]($sus_class_trivially_relocatable_if) | is **conditionally** trivially relocatable if the condition is true |
/// | [`sus_class_trivially_relocatable_unchecked`]($sus_class_trivially_relocatable_unchecked) | is trivially relocatable without any condition or assertion |
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
///       && sus::mem::TriviallyRelocatable<decltype(thing)>
///       && sus::mem::TriviallyRelocatable<decltype(more)>);
/// };
/// ```
#define sus_class_trivially_relocatable_if(unsafe_fn, is_trivially_reloc)    \
  static_assert(std::is_same_v<decltype(unsafe_fn),                          \
                               const ::sus::marker::UnsafeFnMarker>);        \
  static_assert(                                                             \
      std::is_same_v<std::remove_cv_t<decltype(is_trivially_reloc)>, bool>); \
  template <class SusOuterClassTypeForTriviallyReloc>                        \
  friend struct ::sus::mem::__private::RelocatableTag;                       \
  static constexpr bool SusUnsafeTrivialRelocate = is_trivially_reloc

/// Mark a class as unconditionally trivially relocatable, without any
/// additional assertion to help verify correctness.
///
/// Generally, prefer to use sus_class_trivially_relocatable() with
/// all field types passed to the macro.
/// To additionally allow the class to be passed in registers, the class can be
/// marked with the `[[clang::trivial_abi]]` attribute.
///
/// Use the [`TriviallyRelocatable`]($sus::mem::TriviallyRelocatable) concept to
/// determine if a type is trivially relocatable, and to test with static_assert
/// that types are matching what you are expecting. This allows containers to
/// optimize their implementations when relocating the type in memory.
///
/// | Macro | Style |
/// | ----- | ----- |
/// | [`sus_class_trivially_relocatable`]($sus_class_trivially_relocatable) | **asserts** all param types are trivially relocatable |
/// | [`sus_class_trivially_relocatable_if_types`]($sus_class_trivially_relocatable_if_types) | is **conditionally** trivially relocatable if all param types are |
/// | [`sus_class_trivially_relocatable_if`]($sus_class_trivially_relocatable_if) | is **conditionally** trivially relocatable if the condition is true |
/// | [`sus_class_trivially_relocatable_unchecked`]($sus_class_trivially_relocatable_unchecked) | is trivially relocatable without any condition or assertion |
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
  friend struct ::sus::mem::__private::RelocatableTag;                \
  static constexpr bool SusUnsafeTrivialRelocate = true

/// Mark a class field as never being a specific value, often a zero, after a
/// constructor has run and before the destructor has completed. This allows
/// querying if a class is constructed in a memory location, since the class is
/// constructed iff the value of the field is not the never-value.
///
/// The named field can be compared to the `never_value` to determine if the
/// object is constructed. The field must be set to the `destroy_value` just
/// prior to destruction. The latter is meant to help the destructor be a no-op
/// when the type is in a never-value state, if the never-value would be read in
/// the destructor.
///
/// The macro includes `private:` which changes the class definition visibility
/// to private.
#define sus_class_never_value_field(unsafe_fn, T, field_name, never_value,     \
                                    destroy_value)                             \
 private:                                                                      \
  static_assert(                                                               \
      std::same_as<decltype(unsafe_fn), const ::sus::marker::UnsafeFnMarker>); \
                                                                               \
  template <class>                                                             \
  friend struct ::sus::mem::__private::NeverValueAccess;                       \
  template <class>                                                             \
  friend struct ::sus::mem::__private::NeverValueChecker;                      \
                                                                               \
  _sus_pure constexpr bool _sus_Unsafe_NeverValueIsConstructed(                \
      ::sus::marker::UnsafeFnMarker) const noexcept {                          \
    static_assert(                                                             \
        std::is_assignable_v<decltype(field_name)&, decltype(never_value)>,    \
        "The `never_value` must be able to be assigned to the named field.");  \
    return !(field_name == never_value);                                       \
  }                                                                            \
  constexpr void _sus_Unsafe_NeverValueSetDestroyValue(                        \
      ::sus::marker::UnsafeFnMarker) noexcept {                                \
    static_assert(::sus::cmp::Eq<decltype(field_name), decltype(never_value)>, \
                  "The `never_value` must be comparable to the named field."); \
    field_name = destroy_value;                                                \
  }                                                                            \
  static_assert(true)
