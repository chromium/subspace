// Copyright 2023 Google LLC
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

#include "sus/ptr/subclass.h"

namespace sus::construct {

namespace __private {
template <class T>
concept IsConstLvalueReference = std::is_lvalue_reference_v<T> &&
                                 std::is_const_v<std::remove_reference_t<T>>;

}  // namespace __private

/// Returns whether a type `From` is safely constructible from a reference of
/// type `To`. If `To` is a const reference, then the types must match, as a
/// conversion would create a reference to a temporary.
///
/// A struct will typically not actually store a reference but may provide an
/// API of references nonetheless, and then store a pointer. When used with a
/// universal reference `From` type, this can catch the case where the incoming
/// reference will introduce a dangling reference when converted to the `To`
/// reference.
///
/// # Examples
/// template <std::convertible_to<i32&> U>
///   requires(SafelyConstructibleFromReference<i32%, U&&>
/// void Struct::stores_i32_ref(U&& u) {
///   ptr_ = static_cast<i32&>(&u);
/// }
template <class To, class From>
concept SafelyConstructibleFromReference =
    !__private::IsConstLvalueReference<To> ||
    // If the type is the same then no temporary will be created.
    std::same_as<std::remove_cvref_t<From>, std::remove_cvref_t<To>> ||
    // If the type is a base class then no temporary will be created.
    sus::ptr::SameOrSubclassOf<std::remove_cvref_t<From>*,
                                   std::remove_cvref_t<To>*>;

}  // namespace sus::construct
