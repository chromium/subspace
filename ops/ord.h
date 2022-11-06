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

#include <compare>
#include <concepts>

namespace sus::ops {

/// Determines if the types `Lhs` and `Rhs` have a total ordering (aka
/// `std::strong_ordering`).
template <class Lhs, class Rhs>
concept Ord = requires(const Lhs& lhs, const Rhs& rhs) {
                { lhs <=> rhs } -> std::same_as<std::strong_ordering>;
              };

/// Determines if the types `Lhs` and `Rhs` have a weak ordering (aka
/// `std::weak_ordering`).
///
/// This will be true if the types have a total ordering as well, which is
/// stronger than a weak ordering. To determine if a weak ordering is the
/// strongest type of ordering between the types, use `ExclusiveWeakOrd`.
template <class Lhs, class Rhs>
concept WeakOrd = Ord<Lhs, Rhs> || requires(const Lhs& lhs, const Rhs& rhs) {
                                     {
                                       lhs <=> rhs
                                       } -> std::same_as<std::weak_ordering>;
                                   };

/// Determines if the types `Lhs` and `Rhs` have a partial ordering (aka
/// `std::partial_ordering`).
///
/// This will be true if the types have a weak r total ordering as well, which
/// is stronger than a partial ordering. To determine if a partial ordering is
/// the strongest type of ordering between the types, use `ExclusivePartialOrd`.
template <class Lhs, class Rhs>
concept PartialOrd = WeakOrd<Lhs, Rhs> || Ord<Lhs, Rhs> ||
                     requires(const Lhs& lhs, const Rhs& rhs) {
                       { lhs <=> rhs } -> std::same_as<std::partial_ordering>;
                     };

/// Determines if the types `Lhs` and `Rhs` have a total ordering (aka
/// `std::strong_ordering`).
template <class Lhs, class Rhs>
concept ExclusiveOrd = Ord<Lhs, Rhs>;

/// Determines if the types `Lhs` and `Rhs` have a weak ordering (aka
/// `std::weak_ordering`), and that this is the strongest ordering that exists
/// between the types.
template <class Lhs, class Rhs>
concept ExclusiveWeakOrd = (!Ord<Lhs, Rhs> && WeakOrd<Lhs, Rhs>);

/// Determines if the types `Lhs` and `Rhs` have a partial ordering (aka
/// `std::partial_ordering`), and that this is the strongest ordering that
/// exists between the types.
template <class Lhs, class Rhs>
concept ExclusivePartialOrd = (!Ord<Lhs, Rhs> && !WeakOrd<Lhs, Rhs> &&
                               PartialOrd<Lhs, Rhs>);

}  // namespace sus::ops
