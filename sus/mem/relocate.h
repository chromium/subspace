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

#include "sus/macros/builtin.h"
#include "sus/macros/compiler.h"
#include "sus/marker/unsafe.h"
#include "sus/mem/size_of.h"
#include "sus/mem/macros.h"

namespace sus::mem {

namespace __private {

/// Detects the presence and value of the static const member
/// `T::SusUnsafeTrivialRelocate`.
///
/// The static member is created by `sus_class_trivially_relocatable()` and
/// similar macros.
template <class T>
struct RelocatableTag final {
  static constexpr bool value(...) { return false; }

  static constexpr bool value(int)
    requires requires {
      requires(std::same_as<decltype(T::SusUnsafeTrivialRelocate), const bool>);
    }
  {
    return T::SusUnsafeTrivialRelocate;
  };
};

// We use a separate concept to test each `T` so that it can short-circuit when
// `T` is a reference type to an undefined (forward-declared) type.
// clang-format off
template <class T>
concept TriviallyRelocatable_impl =
    std::is_reference_v<T>
    || (!std::is_volatile_v<std::remove_all_extents_t<T>>
       && sus::mem::data_size_of<std::remove_all_extents_t<std::remove_reference_t<T>>>() != size_t(-1)
       && (__private::RelocatableTag<std::remove_all_extents_t<T>>::value(0)
         || std::is_trivially_copyable_v<std::remove_all_extents_t<T>>
         || (std::is_trivially_move_constructible_v<std::remove_all_extents_t<T>> &&
             std::is_trivially_move_assignable_v<std::remove_all_extents_t<T>> &&
             std::is_trivially_destructible_v<std::remove_all_extents_t<T>>)
  #if __has_extension(trivially_relocatable)
         || __is_trivially_relocatable(std::remove_all_extents_t<T>)
  #endif
         )
       );
// clang-format on

}  // namespace __private

/// Tests if a variable of type `T` can be relocated with
/// [`ptr::copy`]($sus::ptr::copy).
///
/// IMPORTANT: If a class satisfies this trait, only
/// [`sus::mem::data_size_of<T>()`]($sus::mem::data_size_of)
/// bytes should be copied when relocating the type, or Undefined Behaviour can
/// result, due to the possibility of overwriting data stored in the tail
/// padding bytes of `T`. See the docs on
/// [`data_size_of`]($sus::mem::data_size_of) for more.
///
/// Volatile types are excluded, as they can not be safely `memcpy`'d
/// byte-by-byte without introducing tearing.
/// References are treated like pointers, and are always trivially relocatable,
/// as reference data members are relocatable in the same way pointers are.
///
/// # Marking a type as trivially relocatable
///
/// Use one of the provided macros inside a class to mark it as conditionally or
/// unconditionally trivially relocatable. They are unsafe because there are no
/// compiler checks that the claim is actually true, though macros are provided
/// to make this easier. The
/// [`sus_class_trivially_relocatable`]($sus_class_trivially_relocatable) and
/// [`sus_class_trivially_relocatable_if_types`]($sus_class_trivially_relocatable_if_types)
/// macros verify that all the types given to it are also trivially relocatable.
/// As long as every field type of the class is given as a parameter, and each
/// field type correctly advertises its trivial relocatability, it will ensure
/// correctness.
///
/// This concept also honors the `[[clang::trivial_abi]]` Clang attribute, as
/// types annotated with the attribute are now considered trivially relocatable
/// in https://reviews.llvm.org/D114732. However, when marking a class with this
/// attribute it is highly recommended to use the
/// [`sus_class_trivially_relocatable`]($sus_class_trivially_relocatable) macro
/// as well to help verify correctness and gain compiler independence.
///
/// | Macro | Style |
/// | ----- | ----- |
/// | [`sus_class_trivially_relocatable`]($sus_class_trivially_relocatable) | **asserts** all param types are trivially relocatable |
/// | [`sus_class_trivially_relocatable_if_types`]($sus_class_trivially_relocatable_if_types) | is **conditionally** trivially relocatable if all param types are |
/// | [`sus_class_trivially_relocatable_if`]($sus_class_trivially_relocatable_if) | is **conditionally** trivially relocatable if the condition is true |
/// | [`sus_class_trivially_relocatable_unchecked`]($sus_class_trivially_relocatable_unchecked) | is trivially relocatable without any condition or assertion |
///
/// # Implementation notes
/// The concept tests against
/// [`std::remove_all_extents_t<T>`](
/// https://en.cppreference.com/w/cpp/types/remove_all_extents)
/// so that the same answer is returned for arrays of `T`, such as for `T` or
/// `T[]` or `T[][][]`.
///
/// While the standard expects types to be `std::is_trivially_copyble` in order
/// to copy by `memcpy`, Clang also allows trivial relocation of move-only types
/// with non-trivial move operations and destructors through
/// `[[clang::trivial_abi]]` while also noting that attributes are not part of
/// and do not modify the type or how the compiler itself treats it outside of
/// argument passing. As such, we consider trivally moveable and destructible
/// types as automatically trivially relocatable, and allow other types to opt
/// in conditionally with the macros and without `[[clang::trivial_abi]]`.
template <class... T>
concept TriviallyRelocatable = (... && __private::TriviallyRelocatable_impl<T>);

}  // namespace sus::mem
