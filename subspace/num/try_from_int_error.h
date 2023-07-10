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

#include <string>

#include "fmt/core.h"
#include "subspace/assertions/unreachable.h"
#include "subspace/macros/pure.h"
#include "subspace/marker/unsafe.h"

namespace sus::num {

/// The error type returned when a checked integral type conversion fails.
class TryFromIntError {
 public:
  /// The type of error which occured.
  enum class Kind {
    OutOfBounds,
  };

  /// Constructs a TryFromIntError with kind `OutOfBounds`.
  constexpr static TryFromIntError with_out_of_bounds() noexcept {
    return TryFromIntError(CONSTRUCT, Kind::OutOfBounds);
  }

  /// Gives the kind of error that occured.
  sus_pure constexpr Kind kind() const noexcept { return kind_; }

  /// sus::ops::Eq trait.
  sus_pure constexpr bool operator==(TryFromIntError rhs) const noexcept {
    return kind_ == rhs.kind_;
  }

 private:
  enum Construct { CONSTRUCT };
  constexpr inline explicit TryFromIntError(Construct, Kind k) noexcept
      : kind_(k) {}

  Kind kind_;
};

}  // namespace sus::num

// fmt support.
template <class Char>
struct fmt::formatter<::sus::num::TryFromIntError, Char> {
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return ctx.begin();
  }

  template <class FormatContext>
  constexpr auto format(::sus::num::TryFromIntError t,
                        FormatContext& ctx) const {
    switch (t.kind()) {
      using enum ::sus::num::TryFromIntError::Kind;
      case OutOfBounds: return fmt::format_to(ctx.out(), "out of bounds");
    }
    ::sus::assertions::unreachable_unchecked(::sus::marker::unsafe_fn);
  }
};
