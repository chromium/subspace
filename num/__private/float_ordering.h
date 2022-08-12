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

#include "num/__private/intrinsics.h"

namespace sus::num::__private {

template <class T>
  requires(std::is_floating_point_v<T> && sizeof(T) <= 8)
constexpr std::strong_ordering float_strong_ordering(T l, T r) noexcept {
  if (into_unsigned_integer(l) == into_unsigned_integer(r))
    return std::strong_ordering::equal;

  const bool l_neg = (into_unsigned_integer(l) & high_bit<T>()) != 0;
  const bool r_neg = (into_unsigned_integer(r) & high_bit<T>()) != 0;
  if (l_neg != r_neg) {
    if (l_neg)
      return std::strong_ordering::less;
    else
      return std::strong_ordering::greater;
  }

  const bool l_nan = float_is_nan(l);
  const bool r_nan = float_is_nan(r);
  if (l_nan != r_nan) {
    if (l_neg == l_nan)
      return std::strong_ordering::less;
    else
      return std::strong_ordering::greater;
  }
  if (l_nan && r_nan) {
    const bool l_quiet = float_is_nan_quiet(l);
    const bool r_quiet = float_is_nan_quiet(r);
    if (l_quiet != r_quiet) {
      if (l_neg == float_is_nan_quiet(l))
        return std::strong_ordering::less;
      else
        return std::strong_ordering::greater;
    } else {
        if (l_neg) {
      return std::strong_order(into_unsigned_integer(r),
                               into_unsigned_integer(l));
        } else {
      return std::strong_order(into_unsigned_integer(l),
                               into_unsigned_integer(r));
        }
    }
  }

  if (l < r)
    return std::strong_ordering::less;
  else
    return std::strong_ordering::greater;
}

}  // namespace sus::num::__private
