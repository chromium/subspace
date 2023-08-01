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

// IWYU pragma: private, include "sus/prelude.h"
// IWYU pragma: friend "sus/.*"
#pragma once

#include <type_traits>

#include "sus/assertions/check.h"
#include "sus/num/signed_integer.h"
#include "sus/num/unsigned_integer.h"
#include "sus/ops/range.h"

namespace sus::ops {

namespace __private {

template <bool IsSigned>
struct RangeLiteralDeducer {
  using Int =
      std::conditional_t<IsSigned, ::sus::num::isize, ::sus::num::usize>;
  using Digit = std::conditional_t<IsSigned, int, unsigned int>;

  // A non-constexpr method used to indicate failure in parsing, which produces
  // a compiler error and still works in the presence of -fno-exceptions unlike
  // `throw`.
  static void invalid(const char*);

  constexpr RangeLiteralDeducer(const char* c) {
    size_t i = 0u;

    bool has_lower = false;
    bool negate_lower = false;
    if (c[i] != '.') {
      if constexpr (IsSigned) {
        if (c[i] == '-') {
          negate_lower = true;
          i += 1u;
        }
      }
      if (c[i] < '0' || c[i] > '9')
        invalid("Invalid lower bound number in range literal");
      {
        auto digit = static_cast<Digit>(c[i] - '0');
        if constexpr (IsSigned) {
          if (negate_lower) digit = -digit;
        }
        lower = digit;
      }
      i += 1u;

      for (; c[i] != '\0' && c[i] != '.'; i += 1u) {
        if (c[i] != '\'') {
          if (c[i] < '0' || c[i] > '9')
            invalid("Invalid lower bound number in range literal");
          auto mul = lower.checked_mul(Digit{10});
          if (mul.is_none()) invalid("Lower bound is out of range");
          lower = ::sus::move(mul).unwrap();
          {
            auto digit = static_cast<Digit>(c[i] - '0');
            if constexpr (IsSigned) {
              if (negate_lower) digit = -digit;
            }
            auto add = lower.checked_add(digit);
            if (add.is_none()) invalid("Lower bound is out of range");
            lower = ::sus::move(add).unwrap();
          }
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

    bool negate_upper = false;
    if constexpr (IsSigned) {
      if (c[i] == '-') {
        negate_upper = true;
        i += 1u;
      }
    }
    if (c[i] < '0' || c[i] > '9')
      invalid("Invalid upper bound number in range literal");
    {
      auto digit = static_cast<Digit>(c[i] - '0');
      if constexpr (IsSigned) {
        if (negate_upper) digit = -digit;
      }
      upper = digit;
    }
    i += 1u;

    for (; c[i] != '\0'; i += 1u) {
      if (c[i] != '\'') {
        if (c[i] < '0' || c[i] > '9')
          invalid("Invalid upper bound number in range literal");
        auto mul = upper.checked_mul(Digit{10});
        if (mul.is_none()) invalid("Upper bound is out of range");
        upper = ::sus::move(mul).unwrap();
        {
          auto digit = static_cast<Digit>(c[i] - '0');
          if constexpr (IsSigned) {
            if (negate_upper) digit = -digit;
          }
          auto add = upper.checked_add(digit);
          if (add.is_none()) invalid("Upper bound is out of range");
          upper = ::sus::move(add).unwrap();
        }
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
  Int lower;
  Int upper;
};

}  // namespace __private

}  // namespace sus::ops

/// Returns a range that satisfies the `RangeBounds<usize>` concept.
///
/// Because `usize` is unsigned, numbers may not be negative.
///
/// The syntax is:
/// * `start..end` for a range including start and excluding end. This returns a
///   `sus::ops::Range<usize>`.
/// * `start..=end` for a range including start and including end. This returns
///   a `sus::ops::Range<usize>`.
/// * `start..` for a range including start and never ending. This returns a
///   `sus::ops::RangeFrom<usize>`.
/// * `..end` for a range including everything up end. This returns a
///   `sus::ops::RangeTo<usize>`.
/// * `..=end` for a range including everything up and including end. This
///   returns a `sus::ops::RangeTo<usize>`.
/// * `..` for a range that has no bounds at all. Typically for a slicing range
///   to indicate the entire slice. This returns a `sus::ops::RangeFull<usize>`.
template <::sus::ops::__private::RangeLiteralDeducer<false> D>
constexpr auto operator""_r() {
  using ::sus::ops::__private::RangeLiteralDeducer;
  if constexpr (D.type == RangeLiteralDeducer<false>::NoBound)
    return ::sus::ops::RangeFull<::sus::num::usize>();
  else if constexpr (D.type == RangeLiteralDeducer<false>::LowerBound)
    return ::sus::ops::RangeFrom<::sus::num::usize>(D.lower);
  else if constexpr (D.type == RangeLiteralDeducer<false>::UpperBound)
    return ::sus::ops::RangeTo<::sus::num::usize>(D.upper);
  else
    return ::sus::ops::Range<::sus::num::usize>(D.lower, D.upper);
}

/// Returns a range that satisfies the `RangeBounds<isize>` concept.
///
/// Numbers may be positive or negative.
///
/// The syntax is:
/// * `start..end` for a range including start and excluding end. This returns a
///   `sus::ops::Range<isize>`.
/// * `start..=end` for a range including start and including end. This returns
///   a `sus::ops::Range<isize>`.
/// * `start..` for a range including start and never ending. This returns a
///   `sus::ops::RangeFrom<isize>`.
/// * `..end` for a range including everything up end. This returns a
///   `sus::ops::RangeTo<isize>`.
/// * `..=end` for a range including everything up and including end. This
///   returns a `sus::ops::RangeTo<isize>`.
/// * `..` for a range that has no bounds at all. Typically for a slicing range
///   to indicate the entire slice. This returns a `sus::ops::RangeFull<isize>`.
template <::sus::ops::__private::RangeLiteralDeducer<true> D>
constexpr auto operator""_rs() {
  using ::sus::ops::__private::RangeLiteralDeducer;
  if constexpr (D.type == RangeLiteralDeducer<true>::NoBound)
    return ::sus::ops::RangeFull<::sus::num::isize>();
  else if constexpr (D.type == RangeLiteralDeducer<true>::LowerBound)
    return ::sus::ops::RangeFrom<::sus::num::isize>(D.lower);
  else if constexpr (D.type == RangeLiteralDeducer<true>::UpperBound)
    return ::sus::ops::RangeTo<::sus::num::isize>(D.upper);
  else
    return ::sus::ops::Range<::sus::num::isize>(D.lower, D.upper);
}

// TODO: _rs to make a signed RangeBounds over `isize`?
