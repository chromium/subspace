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
/// operators. There is no guarantee of a full equivalence relation, and a
/// partial equivalence is possible, which allows for values that compare
/// not-equal to themselves (such as NaN).
///
/// Implementations must ensure that `operator==` and `operator!=` are
/// consistent with each other:
///
/// * `a != b` if and only if `!(a == b)`.
///
/// The default implementation of `operator!=` provides this consistency
/// and is almost always sufficient. It should not be overridden without very
/// good reason.
///
/// This maps to the [`PartialEq`](
/// https://doc.rust-lang.org/stable/std/cmp/trait.PartialEq.html) trait in Rust
/// rather than the `Eq` trait. Since C++ does not understand
/// equivalent vs partial equivalence, we are unable to differentiate and
/// provide a stronger relationship than partial equivalence.
template <class T, class U = T>
concept Eq = requires(const std::remove_reference_t<T>& lhs,
                      const std::remove_reference_t<U>& rhs) {
  { lhs == rhs } -> std::same_as<bool>;
  { lhs != rhs } -> std::same_as<bool>;
};

}  // namespace sus::ops
