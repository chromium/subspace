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

#include "subspace/convert/subclass.h"

namespace sus::construct {

namespace __private {
template <class T>
concept IsConstLvalueReference = std::is_lvalue_reference_v<T> &&
                                 std::is_const_v<std::remove_reference_t<T>>;

}  // namespace __private

/// Returns if a type `From` is safely constructible from a reference of type
/// `To`. This is useful for marker types which hold a reference internally and
/// are used to construct another type.
///
/// If `To` is a const reference, then they types must match, as a conversion
/// would create a reference to a temporary.
///
/// This concept only produces a useful result when `From` is a reference type
/// which may not outlive the storage of `To`, such as with a reference to a
/// temporary.
template <class To, class From>
concept SafelyConstructibleFromReference =
    !__private::IsConstLvalueReference<To> ||
    // If the type is the same then no temporary will be created.
    std::same_as<std::remove_cvref_t<From>, std::remove_cvref_t<To>> ||
    // If the type is a base class then no temporary will be created.
    sus::convert::SameOrSubclassOf<std::remove_cvref_t<From>*,
                                   std::remove_cvref_t<To>*>;

}  // namespace sus::construct
