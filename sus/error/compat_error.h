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

#include <system_error>

#include "sus/error/error.h"

// Implements [`Error`]($sus::error::Error) for
// [`std::error_code`](https://en.cppreference.com/w/cpp/error/error_code).
//
// `std::error_code` can also represent success, but
// `Error` unconditionally represents an error, so it should be
// [checked for failure](
// https://en.cppreference.com/w/cpp/error/error_code/operator_bool)
// before being used an an `Error`.
//
// When the `sus/error/compat_error.h` header is included, a
// [`std::error_code`](https://en.cppreference.com/w/cpp/error/error_code) can
// be used whenever an [`Error`]($sus::error::Error) is needed.
template <>
struct sus::error::ErrorImpl<std::error_code> {
  static std::string display(const std::error_code& e) noexcept {
    return e.message();
  }
};

static_assert(sus::error::Error<std::error_code>);

// Implements [`Error`]($sus::error::Error) for
// [`std::error_condition`](https://en.cppreference.com/w/cpp/error/error_condition).
//
// `std::error_condition` can also represent success, but
// `Error` unconditionally represents an error, so it should be
// [checked for failure](
// https://en.cppreference.com/w/cpp/error/error_condition/operator_bool)
// before being used an an `Error`.
//
// When the `sus/error/compat_error.h` header is included, a
// [`std::error_condition`](https://en.cppreference.com/w/cpp/error/error_condition)
// can be used whenever an [`Error`]($sus::error::Error) is needed.
template <>
struct sus::error::ErrorImpl<std::error_condition> {
  static std::string display(const std::error_condition& e) noexcept {
    return e.message();
  }
};

static_assert(sus::error::Error<std::error_condition>);
