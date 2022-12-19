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

/// A `Copy` type can be copied to construct a new object and can assigned to by
/// copy.
///
/// Satisfying `Copy` also implies that the type satisfies `Clone`.
///
/// Const (non-reference) types are not `Copy` as they can't be assigned to.
/// References are always `Copy`, even if const, as a reference can always be
/// constructed from a reference.
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
concept Copy = std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T>;

}  // namespace sus::mem
