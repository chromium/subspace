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

#include "sus/macros/pure.h"

namespace sus::num {

/// The error type returned when a checked integral type conversion fails.
class TryFromIntError {
 public:
  /// The type of error which occured.
  enum class Kind {
    OutOfBounds,
  };

  /// Constructs a TryFromIntError with kind `OutOfBounds`.
  constexpr static TryFromIntError with_out_of_bounds() noexcept;

  /// Gives the kind of error that occured.
  sus_pure constexpr Kind kind() const noexcept;

  /// Satisfies the [`Eq`]($sus::cmp::Eq) concept.
  sus_pure constexpr bool operator==(TryFromIntError rhs) const noexcept;

 private:
  enum Construct { CONSTRUCT };
  constexpr explicit TryFromIntError(Construct, Kind k) noexcept;

  Kind kind_;
};

}  // namespace sus::num
