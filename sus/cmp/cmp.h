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

// IWYU pragma: private
// IWYU pragma: friend "sus/cmp/.*"
#pragma once

namespace sus {

/// Utilities for comparing and ordering values.
/// 
/// This namespace contains various tools for comparing and ordering values. In
/// summary:
/// 
/// * [`Eq`]($sus::cmp::Eq) is a concept that allows you to require partial
///   equality between values. It requires the `==` and `!=` operators to be
///   available, which is typically implemented just by `==`. Note that C++ does
///   not model total vs partial equality so there's no differentiation for a
///   `PartialEq` concept.
/// * [`Ord`]($sus::cmp::Ord), [`StrongOrd`]($sus::cmp::StrongOrd) and
///   [`PartialOrd`]($sus::cmp::PartialOrd) are concept that allows you to
///   require total, unique, and partial orderings between values, respectively.
///   Implementing them requires the `<=>` operator to be defined, and makes the
///   the `<`, `<=`, `>`, and `>=` operators accessible.
///   [`Ordering`]($sus::cmp::Ordering) is a concept that matches any of the
///   standard library ordering types which are returned from the `<=>`
///   operator: [`std::weak_ordering`](
///   https://en.cppreference.com/w/cpp/utility/compare/weak_ordering),
///   [`std::strong_ordering`](
///   https://en.cppreference.com/w/cpp/utility/compare/strong_ordering), or
///   [`std::partial_ordering`](
///   https://en.cppreference.com/w/cpp/utility/compare/partial_ordering).
/// * [`Reverse`]($sus::cmp::Reverse) is a struct that allows you to easily
///   reverse an ordering.
/// * [`max`]($sus::cmp::max) and [`min`]($sus::cmp::min) are functions that
///   build off of [`Ord`]($sus::cmp::Ord) and allow you to find the maximum or
///   minimum of two values. There are other similar functions that make
///   use of [`Ord`]($sus::cmp::Ord) as well.
///
/// For more details, see the respective documentation of each item in the list.
namespace cmp {}

}  // namespace sus
