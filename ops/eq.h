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

namespace sus::ops {

// Type `A` and `B` are `Eq<A, B>` if an object of each type can be compared for
// equality with the `==` operator.
//
// TODO: How do we do PartialEq? Can we even? Can we require Ord to be Eq? But
// then it depends on ::num?
template <class Lhs, class Rhs>
concept Eq = requires(const Lhs& lhs, const Rhs& rhs) {
               { lhs == rhs } -> std::same_as<bool>;
             };

}  // namespace sus::ops
