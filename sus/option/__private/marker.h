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
#include "sus/macros/__private/compiler_bugs.h"
#include "sus/mem/forward.h"
#include "sus/mem/move.h"
#include "sus/mem/remove_rvalue_reference.h"

namespace sus::option::__private {

template <class T>
struct [[nodiscard]] SomeMarker {
  explicit constexpr SomeMarker(T&& value) : value(::sus::forward<T>(value)) {}

  T&& value;

  // Gtest macros force evaluation against a const reference.
  // https://github.com/google/googletest/issues/4350
  //
  // So we allow constructing the target type from a const ref `value`. This
  // can perform explicit conversions which isn't correct but is required in
  // order to be able to call this method twice, because `value` itself may not
  // be copyable (marker types are not) but construction from const is more
  // likely.
  template <class U>
  inline constexpr operator Option<U>() const& noexcept {
    static_assert(
        std::convertible_to<::sus::mem::remove_rvalue_reference<T>, U>,
        "SomeMarker(T) can't convert const T& to U for Option<U>. "
        "Note that this is a test-only code path for Gtest support. "
        "Typically the T object is consumed as an rvalue, consider using "
        "EXPECT_TRUE(a == b) if needed.");
    static_assert(std::convertible_to<T&&, U>,
                  "SomeMarker(T) can't convert T to U for Option<U>");
    static_assert(
        ::sus::construct::SafelyConstructibleFromReference<U, const T&>,
        "Unable to safely convert to a different reference type, as conversion "
        "would produce a reference to a temporary. The SomeMarker's value type "
        "must match the Option's. For example an `Option<const i32&>` can not "
        "be constructed from a SomeMarker holding `const i16&`, but it can be "
        "constructed from `i32&&`.");
    return Option<U>(U(value));
  }

  template <class U>
  inline constexpr operator Option<U>() && noexcept {
    static_assert(std::convertible_to<T&&, U>,
                  "SomeMarker(T) can't convert T to U for Option<U>");
    static_assert(
        ::sus::construct::SafelyConstructibleFromReference<U, T&&>,
        "Unable to safely convert to a different reference type, as conversion "
        "would produce a reference to a temporary. The SomeMarker's value type "
        "must match the Option's. For example an `Option<const i32&>` can not "
        "be constructed from a SomeMarker holding `const i16&`, but it can be "
        "constructed from `i32&&`.");
    return Option<U>(::sus::forward<T>(value));
  }

  SomeMarker(SomeMarker&&) = delete;
  SomeMarker& operator=(SomeMarker&&) = delete;
};

struct [[nodiscard]] NoneMarker {
  explicit constexpr NoneMarker() = default;

  // Gtest macros force evaluation against a const reference.
  // https://github.com/google/googletest/issues/4350
  template <class U>
  sus_pure_const inline constexpr operator Option<U>() const& noexcept {
    return Option<U>();
  }

  template <class U>
  sus_pure_const inline constexpr operator Option<U>() && noexcept {
    return Option<U>();
  }

  NoneMarker(NoneMarker&&) = delete;
  NoneMarker& operator=(NoneMarker&&) = delete;
};

}  // namespace sus::option::__private
