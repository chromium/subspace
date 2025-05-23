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

// IWYU pragma: private, include "sus/num/types.h"
// IWYU pragma: friend "sus/.*"
#pragma once

#include <string>

#include "sus/assertions/unreachable.h"
#include "sus/error/error.h"
#include "sus/macros/pure.h"
#include "sus/marker/unsafe.h"
#include "sus/num/try_from_int_error.h"

namespace sus::num {

constexpr TryFromIntError TryFromIntError::with_out_of_bounds() noexcept {
  return TryFromIntError(CONSTRUCT, Kind::OutOfBounds);
}

_sus_pure constexpr TryFromIntError::Kind TryFromIntError::kind()
    const noexcept {
  return kind_;
}

constexpr TryFromIntError::TryFromIntError(Construct, Kind k) noexcept
    : kind_(k) {}

}  // namespace sus::num

// sus::error::Error implementation.
template <>
struct sus::error::ErrorImpl<sus::num::TryFromIntError> {
  constexpr static std::string display(
      const sus::num::TryFromIntError& e) noexcept {
    switch (e.kind()) {
      case sus::num::TryFromIntError::Kind::OutOfBounds: return "out of bounds";
    }
    sus_unreachable_unchecked(::sus::marker::unsafe_fn);
  }
};

static_assert(sus::error::Error<sus::num::TryFromIntError>);
