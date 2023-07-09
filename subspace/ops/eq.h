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

// TODO: Move all this to sus::cmp?

#pragma once

#include <concepts>

namespace sus::ops {

/// Concept for types that can be compared for equality with the `==` and `!=`
/// operators.
///
/// Implementations must ensure that eq and ne are consistent with each other:
///
/// * a != b if and only if !(a == b).
///
/// The default implementation of the `!=` operator provides this consistency
/// and is almost always sufficient. It should not be overridden without very
/// good reason.
///
/// This concept allows for partial equality, for types that do not have a full
/// equivalence relation. For example, in floating point numbers NaN != NaN, but
/// they can still be compared with `==` and `!=`. Unlike Rust, C++ does not
/// understand partial equivalence so we are unable to differentiate.
///
/// TODO: How do we do PartialEq? Can we even? Should we require Ord to be Eq?
template <class T, class U = T>
concept Eq = requires(const std::remove_reference_t<T>& lhs,
                      const std::remove_reference_t<U>& rhs) {
  { lhs == rhs } noexcept -> std::same_as<bool>;
  { lhs != rhs } noexcept -> std::same_as<bool>;
};

}  // namespace sus::ops
