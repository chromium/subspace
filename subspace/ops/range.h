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

#include "subspace/assertions/check.h"
#include "subspace/construct/default.h"
#include "subspace/num/unsigned_integer.h"
#include "subspace/ops/ord.h"

namespace sus::ops {

/// A (half-open) range bounded inclusively below and exclusively above
/// `(start..end)`.
///
/// The range `start..end` contains all values with `start <= x < end`. It is
/// empty if `start >= end`.
///
/// In most cases a Range is passed as an argument to a function or operator[]
/// call, and it can be constructed simply as `{start, end}`.
template <class T>
  requires(::sus::ops::Ord<T>)
struct Range {
  T start;
  T end;

  /// Returns true if `item` is contained in the range.
  constexpr bool contains(T item) const noexcept {
    return start <= item && item < end;
  }

  /// Returns true if the range contains no items.
  ///
  /// The range is empty if either side is incomparable, such as `f32::NAN`.
  constexpr bool is_empty() const noexcept { return !(start < end); }
};

}  // namespace sus::ops
