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

#include <optional>

#include "sus/option/option.h"
#include "sus/ops/try.h"

// Implements sus::ops::Try for std::optional.
template <class T>
struct sus::ops::TryImpl<std::optional<T>> {
  using Output = T;
  template <class U>
  using RemapOutput = std::optional<U>;
  constexpr static bool is_success(const std::optional<T>& t) {
    return t.has_value();
  }
  constexpr static Output into_output(std::optional<T> t) {
    // SAFETY: `operator*` causes UB if the optional is empty. But the optional
    // is verified to be holding a value by `sus::ops::try_into_output()` before
    // it calls here.
    return *::sus::move(t);
  }
  constexpr static std::optional<T> from_output(Output t) {
    return std::optional<T>(std::in_place, ::sus::move(t));
  }
  template <class U>
  constexpr static std::optional<T> preserve_error(std::optional<U>) noexcept {
    // The incoming optional is known to be empty (the error state) and this is
    // checked by try_preserve_error() before coming here. So we can just return
    // another empty optional.
    return std::optional<T>();
  }

  // Implements sus::ops::TryDefault for `std::optional<T>` if `T` satisfies
  // `Default`.
  constexpr static std::optional<T> from_default() noexcept
    requires(sus::construct::Default<T>)
  {
    return std::optional<T>(std::in_place, T());
  }
};

