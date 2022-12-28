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

namespace sus::convert {

/// `SameOrSubclassOf<T*, U*>` is true if `T` is the same type as `U` or
/// inherits from `U`.
///
/// This can replace the use of std::is_convertible_v for handling compatible
/// pointers.
///
/// The inputs must be pointer types, which helps avoid accidental conversions
/// from arrays, as they are not pointers. And helps prevent the decay from the
/// array to a pointer before the concept is called.
//
// clang-format off
template <class T, class U>
concept SameOrSubclassOf =
    std::is_pointer_v<T> &&
    std::is_pointer_v<U> &&
    std::is_convertible_v<T, U>;
// clang-format on

}  // namespace sus::convert
