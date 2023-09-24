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

// IWYU pragma: private
// IWYU pragma: friend "sus/.*"
#pragma once

#include "sus/construct/safe_from_reference.h"
#include "sus/lib/__private/forward_decl.h"
#include "sus/mem/forward.h"
#include "sus/mem/move.h"
#include "sus/mem/remove_rvalue_reference.h"
#include "sus/result/ok_void.h"

namespace sus::result::__private {

struct [[nodiscard]] OkVoidMarker {
  OkVoidMarker() = default;

  // Gtest macros force evaluation against a const reference.
  // https://github.com/google/googletest/issues/4350
  template <class E>
  constexpr operator ::sus::result::Result<void, E>() const& noexcept {
    return Result<void, E>(::sus::result::OkVoid());
  }

  template <class E>
  constexpr operator ::sus::result::Result<void, E>() && noexcept {
    return Result<void, E>(::sus::result::OkVoid());
  }

  template <class E>
  constexpr ::sus::result::Result<void, E> construct() && noexcept {
    return ::sus::move(*this);
  }

  OkVoidMarker(const OkVoidMarker&) = delete;
  OkVoidMarker& operator=(const OkVoidMarker&) = delete;
};

template <class T>
struct [[nodiscard]] OkMarker {
  constexpr inline OkMarker(T&& value) : value(::sus::forward<T>(value)) {}

  T&& value;

  // Gtest macros force evaluation against a const reference.
  // https://github.com/google/googletest/issues/4350
  //
  // So we allow constructing the target type from a const ref `value`. This
  // can perform explicit conversions which isn't correct but is required in
  // order to be able to call this method twice, because `value` itself may not
  // be copyable (marker types are not) but construction from const is more
  // likely.
  template <class U, class E>
  inline constexpr operator ::sus::result::Result<U, E>() const& noexcept {
    static_assert(
        std::convertible_to<::sus::mem::remove_rvalue_reference<T>, U>,
        "OkMarker(T) can't convert const T& to U for Result<U, E>. "
        "Note that this is a test-only code path for Gtest support. "
        "Typically the T object is consumed as an rvalue, consider using "
        "EXPECT_TRUE(a == b) if needed.");
    static_assert(std::convertible_to<T&&, U>,
                  "OkMarker(T) can't convert T to U for Result<U, E>");
    static_assert(
        ::sus::construct::SafelyConstructibleFromReference<U, const T&>,
        "Unable to safely convert to a different reference type, as conversion "
        "would produce a reference to a temporary. The OkMarker's value type "
        "must match the Result's. For example a `Result<const i32&, E>` can "
        "not be constructed from an OkMarker holding `const i16&`, but it can "
        "be constructed from `i32&&`.");
    return Result<U, E>(U(value));
  }

  template <class U, class E>
  inline constexpr operator ::sus::result::Result<U, E>() && noexcept {
    static_assert(std::convertible_to<T&&, U>,
                  "OkMarker(T) can't convert T to U for Result<U, E>");
    static_assert(
        ::sus::construct::SafelyConstructibleFromReference<U, T&&>,
        "Unable to safely convert to a different reference type, as conversion "
        "would produce a reference to a temporary. The OkMarker's value type "
        "must match the Result's. For example a `Result<const i32&, E>` can "
        "not be constructed from an OkMarker holding `const i16&`, but it can "
        "be constructed from `i32&&`.");
    return Result<U, E>(::sus::forward<T>(value));
  }

  OkMarker(const OkMarker&) = delete;
  OkMarker& operator=(const OkMarker&) = delete;
};

template <class E>
struct [[nodiscard]] ErrMarker {
  constexpr inline ErrMarker(E&& value) : value(::sus::forward<E>(value)) {}

  E&& value;

  // Gtest macros force evaluation against a const reference.
  // https://github.com/google/googletest/issues/4350
  //
  // So we allow constructing the target type from a const ref `value`. This
  // can perform explicit conversions which isn't correct but is required in
  // order to be able to call this method twice, because `value` itself may not
  // be copyable (marker types are not) but construction from const is more
  // likely.
  template <class T, class F>
  inline constexpr operator ::sus::result::Result<T, F>() const& noexcept {
    static_assert(
        std::convertible_to<::sus::mem::remove_rvalue_reference<E>, F>,
        "ErrMarker(T) can't convert const E& to F for Result<T, F>. "
        "Note that this is a test-only code path for Gtest support. "
        "Typically the T object is consumed as an rvalue, consider using "
        "EXPECT_TRUE(a == b) if needed.");
    static_assert(std::convertible_to<E&&, F>,
                  "ErrMarker(E) can't convert E to F for Result<T, F>");
    return Result<T, F>::with_err(F(value));
  }

  template <class T, class F>
  inline constexpr operator ::sus::result::Result<T, F>() && noexcept {
    static_assert(std::convertible_to<E&&, F>,
                  "ErrMarker(E) can't convert E to F for Result<T, F>");
    return Result<T, F>::with_err(::sus::forward<E>(value));
  }

  ErrMarker(const ErrMarker&) = delete;
  ErrMarker& operator=(const ErrMarker&) = delete;
};

}  // namespace sus::result::__private
