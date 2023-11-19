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

// IWYU pragma: private, include "sus/prelude.h"
// IWYU pragma: friend "sus/.*"
#pragma once

#include <concepts>

namespace sus::mem {

/// A `Copy` type can be copied to construct a new object and can be assigned to
/// by copy.
///
/// Satisfying `Copy` also implies that the type satisfies `Clone`.
///
/// This concept tests the object type of `T`, not a reference type `T&` or
/// `const T&`.
///
/// Typically types should only be `Copy` when performing a copy is very cheap,
/// and thus unlikely to cause performance problems. For types that are larger
/// or more complex to copy, it is better to make them satisfy `Clone` instead
/// so that copies are always explicit.
///
/// Types that can not be assigned to at all, by copy or move, can still satisfy
/// Copy by being able to construct by copy. This is required for types with
/// const fields.
///
/// # Example
/// ```
/// struct S {
///   S() = default;
///   S(const S&) = default;
///   S& operator=(const S&) = default;
/// };
/// static_assert(sus::mem::Copy<S>);
/// static_assert(sus::mem::Clone<S>);
/// ```
template <class T>
concept Copy = std::is_copy_constructible_v<
                   std::remove_const_t<std::remove_reference_t<T>>> &&
               (std::is_copy_assignable_v<
                    std::remove_const_t<std::remove_reference_t<T>>> ||
                !std::is_move_assignable_v<
                    std::remove_const_t<std::remove_reference_t<T>>>);

/// A `TrivialCopy` type is `Copy` but may be copied with memcpy() or memmove()
/// instead of calling the copy constructor/assignment. This allows groups of
/// items to be copied in a single operation.
///
/// Satisfying `TrivialCopy` also implies that the type satisfies both `Copy`
/// and `Clone`.
///
/// Typically types should only be `TrivialCopy` when performing a copy is very
/// cheap, and thus unlikely to cause performance problems. For types that are
/// larger, it is better to make them satisfy `Clone` instead so that copies are
/// always explicit.
template <class T>
concept TrivialCopy =
    Copy<T> && std::is_trivially_copyable_v<
                   std::remove_const_t<std::remove_reference_t<T>>>;

/// A `CopyOrRef` object or reference of type `T` can be copied to construct a
/// new `T`.
///
/// Satisfying `CopyOrRef` also implies that the type satisfies `CloneOrRef`.
///
/// This concept is used for templates that want to be generic over references,
/// that is templates that want to allow their template parameter to be a
/// reference and work with that reference as if it were an object itself. This
/// is uncommon outside of library implementations, and its usage should
/// typically be encapsulated inside a type that is `Copy`.
template <class T>
concept CopyOrRef = Copy<T> || std::is_reference_v<T>;

/// Matches types which are [`CopyOrRef`]($sus::mem::CopyOrRef) or are `void`.
///
/// A helper for genertic types which can hold void as a type.
template <class T>
concept CopyOrRefOrVoid = CopyOrRef<T> || std::is_void_v<T>;

}  // namespace sus::mem
