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

namespace sus::result::__private {

struct OkVoidMarker {
  // If the Result's type can construct from a const ref `value` (roughly, is
  // copy-constructible, but may change types), then the marker can do the same.
  //
  // This largely exists to support use in Gtest's EXPECT_EQ, which uses them as
  // a const&, since marker types should normally be converted quickly to the
  // concrete type.
  template <class E>
  inline constexpr operator ::sus::result::Result<void, E>() const& noexcept {
    return Result<void, E>(::sus::result::OkVoid());
  }

  template <class E>
  inline constexpr operator ::sus::result::Result<void, E>() && noexcept {
    return Result<void, E>(::sus::result::OkVoid());
  }

  template <class E>
  inline constexpr ::sus::result::Result<void, E> construct() && noexcept {
    return ::sus::move(*this);
  }
};

template <class T>
struct OkMarker {
  sus_clang_bug_54040(constexpr inline OkMarker(T&& value)
                      : value(::sus::forward<T>(value)){});

  T&& value;

  // If the Result's type can construct from a const ref `value` (roughly, is
  // copy-constructible, but may change types), then the marker can do the same.
  //
  // This largely exists to support use in Gtest's EXPECT_EQ, which uses them as
  // a const&, since marker types should normally be converted quickly to the
  // concrete type.
  template <class U, class E>
    requires(std::constructible_from<U, const T&>)
  inline constexpr operator ::sus::result::Result<U, E>() const& noexcept {
    static_assert(
        ::sus::construct::SafelyConstructibleFromReference<U, const T&>,
        "Unable to safely convert to a different reference type, as conversion "
        "would produce a reference to a temporary. The OkMarker's value type "
        "must match the Result's. For example a `Result<const i32&, E>` can "
        "not be constructed from an OkMarker holding `const i16&`, but it can "
        "be constructed from `i32&&`.");
    return Result<U, E>(U(static_cast<const T&>(value)));
  }

  template <class U, class E>
  inline constexpr operator ::sus::result::Result<U, E>() && noexcept {
    static_assert(
        ::sus::construct::SafelyConstructibleFromReference<U, T&&>,
        "Unable to safely convert to a different reference type, as conversion "
        "would produce a reference to a temporary. The OkMarker's value type "
        "must match the Result's. For example a `Result<const i32&, E>` can "
        "not be constructed from an OkMarker holding `const i16&`, but it can "
        "be constructed from `i32&&`.");
    return Result<U, E>(::sus::forward<T>(value));
  }

  template <class E>
  inline constexpr ::sus::result::Result<::sus::mem::remove_rvalue_reference<T>,
                                         E>
  construct() && noexcept {
    return ::sus::move(*this);
  }

  template <class U, class E>
  inline constexpr ::sus::result::Result<U, E> construct() && noexcept {
    return ::sus::move(*this);
  }
};

template <class E>
struct ErrMarker {
  sus_clang_bug_54040(constexpr inline ErrMarker(E&& value)
                      : value(::sus::forward<E>(value)){});

  E&& value;

  // If the Result's type can construct from a const ref `value` (roughly, is
  // copy-constructible, but may change types), then the marker can do the same.
  //
  // This largely exists to support use in Gtest's EXPECT_EQ, which uses them as
  // a const&, since marker types should normally be converted quickly to the
  // concrete type.
  template <class T, class F>
    requires(std::constructible_from<F, std::remove_reference_t<E>&>)
  inline constexpr operator ::sus::result::Result<T, F>() const& noexcept {
    return Result<T, F>::with_err(value);
  }

  template <class T, class F>
  inline constexpr operator ::sus::result::Result<T, F>() && noexcept {
    return Result<T, F>::with_err(::sus::forward<E>(value));
  }

  template <class T, class F = std::remove_reference_t<E>>
  inline constexpr ::sus::result::Result<T, F> construct() && noexcept {
    return ::sus::move(*this);
  }
};

}  // namespace sus::result::__private
