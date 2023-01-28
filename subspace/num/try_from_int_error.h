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

#include "subspace/assertions/unreachable.h"

namespace sus::num {

/// The error type returned when a checked integral type conversion fails.
class TryFromIntError {
 public:
  /// The type of error which occured.
  enum class Kind {
    OutOfBounds,
  };

  /// Constructs a TryFromIntError with a `kind`.
  explicit constexpr TryFromIntError(Kind kind) : kind_(kind) {}

  constexpr std::string to_string() noexcept {
    switch (kind_) {
      case Kind::OutOfBounds: return std::string("out of bounds");
    }
    ::sus::assertions::unreachable_unchecked(::sus::marker::unsafe_fn);
  }

 private:
  const Kind kind_;
};

}  // namespace sus::num
