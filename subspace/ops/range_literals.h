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
#include "subspace/num/unsigned_integer.h"
#include "subspace/ops/range.h"

namespace sus::ops {

namespace __private {

struct RangeLiteralDeducer {
  // A non-consteval method used to indicate failure in parsing, which produces
  // a compiler error and still works in the presence of -fno-exceptions unlike
  // `throw`.
  static void invalid(const char*);

  consteval RangeLiteralDeducer(const char* c) {
    size_t i = 0u;

    bool has_lower = false;
    if (c[i] != '.') {
      if (c[i] < '0' || c[i] > '9')
        invalid("Invalid lower bound number in range literal");
      lower = static_cast<unsigned int>(c[i] - '0');
      i += 1u;

      for (; c[i] != '\0' && c[i] != '.'; i += 1u) {
        if (c[i] != '\'') {
          if (c[i] < '0' || c[i] > '9')
            invalid("Invalid lower bound number in range literal");
          lower = lower * 10u + static_cast<unsigned int>(c[i] - '0');
        } else if (c[i + 1] < '0' || c[i + 1] > '9') {
          invalid("Invalid lower bound number in range literal");
        }
      }
      has_lower = true;
    }

    if (c[i] != '.' || c[i + 1u] != '.')
      invalid("Missing `..` in range literal");
    i += 2u;

    if (c[i] == '\0') {
      if (has_lower) {
        type = LowerBound;
        upper = 0u;
        return;
      } else {
        type = NoBound;
        lower = 0u;
        upper = 0u;
        return;
      }
    }

    bool include_upper = false;
    if (c[i] == '=') {
      include_upper = true;
      i += 1u;
    }

    if (c[i] < '0' || c[i] > '9')
      invalid("Invalid upper bound number in range literal");
    upper = static_cast<unsigned int>(c[i] - '0');
    i += 1u;

    for (; c[i] != '\0'; i += 1u) {
      if (c[i] != '\'') {
        if (c[i] < '0' || c[i] > '9')
          invalid("Invalid upper bound number in range literal");
        upper = upper * 10u + static_cast<unsigned int>(c[i] - '0');
      } else if (c[i + 1] < '0' || c[i + 1] > '9') {
        invalid("Invalid upper bound number in range literal");
      }
    }

    if (include_upper) upper += 1u;
    if (has_lower) {
      type = LowerAndUpperBound;
      return;
    } else {
      type = UpperBound;
      lower = 0u;
    }
  }

  enum {
    NoBound,
    LowerBound,
    UpperBound,
    LowerAndUpperBound,
  } type;
  ::sus::num::usize lower;
  ::sus::num::usize upper;
};

}  // namespace __private

}  // namespace sus::ops

/// Returns a range that satisfies the `RangeBounds<usize>` concept.
///
/// The syntax is:
/// * `start..end` for a range including start and excluding end.
/// * `start..=end` for a range including start and including end.
/// * `start..` for a range including start and never ending.
/// * `..` for a range that has no bounds at all. Typically for a slicing range
///   to indicate the entire slice.
template <::sus::ops::__private::RangeLiteralDeducer D>
constexpr auto operator""_r() {
  using ::sus::ops::__private::RangeLiteralDeducer;
  if constexpr (D.type == RangeLiteralDeducer::NoBound)
    return ::sus::ops::RangeFull<::sus::num::usize>();
  else if constexpr (D.type == RangeLiteralDeducer::LowerBound)
    return ::sus::ops::RangeFrom<::sus::num::usize>::with(D.lower);
  else if constexpr (D.type == RangeLiteralDeducer::UpperBound)
    return ::sus::ops::RangeTo<::sus::num::usize>::with(D.upper);
  else
    return ::sus::ops::Range<::sus::num::usize>::with(D.lower, D.upper);
}

// TODO: _rs to make a signed RangeBounds over `isize`?
