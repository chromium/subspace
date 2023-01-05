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

namespace sus::mem {

/// A `Copy` type can be copied to construct a new object and can be assigned to
/// by copy.
///
/// Satisfying `Copy` also implies that the type satisfies `Clone`.
///
/// This concept tests the object type of `T`, not a reference type `T&` or `const T&`.
///
/// Typically types should only be `Copy` when performing a copy is very cheap,
/// and thus unlikely to cause performance problems. For types that are larger
/// or more complex to copy, it is better to make them satisfy `Clone` instead
/// so that copies are always explicit.
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
concept Copy = std::is_copy_constructible_v<std::remove_reference_t<T>> &&
               std::is_copy_assignable_v<std::remove_reference_t<T>>;

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

}  // namespace sus::mem
