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
#include <string_view>

#include "sus/assertions/unreachable.h"
#include "sus/env/env.h"
#include "sus/error/error.h"
#include "sus/result/result.h"

namespace sus::env {

/// The error type for operations interacting with environment variables.
/// Possibly returned from [`env::var()`]($sus::env::var).
struct VarError {
  enum Reason { NotFound, InvalidKeyEncoding };

  /// This struct acts as a proxy for the Reason enum, so it can be implicitly
  /// constructed from the reason value.
  VarError(Reason r) : reason(r) {}

  Reason reason;

  /// Satisfies the [`Eq`]($sus::cmp::Eq) concept.
  constexpr bool operator==(const VarError& rhs) const noexcept = default;
};

/// Fetches the environment variable `key` from the current process.
///
/// # Errors
/// This function will return an error if the environment variable isn't set.
///
///
/// This function may return an error if the `key` is not a valid multi-byte
/// encoding for the current locale (typically utf8).
///
/// This function may return an error if the environment variable's name
/// contains the equal sign character `'='` or the NUL character `'\0'`.
///
/// TODO: [Figure out](https://github.com/chromium/subspace/issues/326) a nicer
/// way to receive unowned possibly-null-terminated strings. string_view reports
/// a size before the null so you can't tell and you need to copy from it.
sus::Result<std::string, VarError> var(const std::string& key) noexcept;

/// Sets the environment variable `key` to the value `value` for the currently
/// running process.
///
/// # Panics
/// This function may panic if the `key` or `value` are not a valid multi-byte
/// encoding for the current locale (typically utf8).
///
/// This function may panic if `key` is empty, contains the equals sign
/// character `'='` or the NUL character `'\0'` (except as the terminating
/// character), or when `value` contains the NUL character (except as the
/// terminating character).
///
/// TODO: [Figure out](https://github.com/chromium/subspace/issues/326) a nicer
/// way to receive unowned possibly-null-terminated strings. string_view reports
/// a size before the null so you can't tell and you need to copy from it.
void set_var(const std::string& key, const std::string& value) noexcept;

}  // namespace sus::env

template <>
struct sus::error::ErrorImpl<::sus::env::VarError> {
  constexpr static std::string display(
      const ::sus::env::VarError& self) noexcept {
    switch (self.reason) {
      case ::sus::env::VarError::NotFound: return "NotFound";
      case ::sus::env::VarError::InvalidKeyEncoding:
        return "InvalidKeyEncoding";
    }
    ::sus::unreachable();
  }
};
