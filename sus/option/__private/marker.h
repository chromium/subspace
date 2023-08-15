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
struct SomeMarker {
  sus_clang_bug_54040(constexpr inline SomeMarker(T&& value)
                      : value(::sus::forward<T>(value)){});

  T&& value;

  // If the Option's type can construct from a const ref `value` (roughly, is
  // copy-constructible, but may change types), then the marker can do the same.
  //
  // This largely exists to support use in Gtest's EXPECT_EQ, which uses them as
  // a const&, since marker types should normally be converted quickly to the
  // concrete type.
  template <class U>
    requires(std::constructible_from<U, const T&>)
  inline constexpr operator Option<U>() const& noexcept {
    static_assert(
        ::sus::construct::SafelyConstructibleFromReference<U, const T&>,
        "Unable to safely convert to a different reference type, as conversion "
        "would produce a reference to a temporary. The SomeMarker's value type "
        "must match the Option's. For example an `Option<const i32&>` can not "
        "be constructed from a SomeMarker holding `const i16&`, but it can be "
        "constructed from `i32&&`.");
    return Option<U>(U(static_cast<const T&>(value)));
  }

  template <class U>
  inline constexpr operator Option<U>() && noexcept {
    static_assert(
        ::sus::construct::SafelyConstructibleFromReference<U, T&&>,
        "Unable to safely convert to a different reference type, as conversion "
        "would produce a reference to a temporary. The SomeMarker's value type "
        "must match the Option's. For example an `Option<const i32&>` can not "
        "be constructed from a SomeMarker holding `const i16&`, but it can be "
        "constructed from `i32&&`.");
    return Option<U>(::sus::forward<T>(value));
  }

  template <class U = ::sus::mem::remove_rvalue_reference<T>>
  inline constexpr Option<U> construct() && noexcept {
    return sus::move(*this);
  }
};

struct NoneMarker {
  // If the Option's type can construct from a const ref `value` (roughly, is
  // copy-constructible, but may change types), then the marker can do the
  // same.
  //
  // This largely exists to support use in Gtest's EXPECT_EQ, which uses them
  // as a const&, since marker types should normally be converted quickly to
  // the concrete type.
  template <class U>
  sus_pure_const inline constexpr operator Option<U>() const& noexcept {
    return Option<U>();
  }

  template <class U>
  sus_pure_const inline constexpr operator Option<U>() && noexcept {
    return Option<U>();
  }

  template <class T>
  inline constexpr Option<T> construct() && noexcept {
    return sus::move(*this);
  }
};

}  // namespace sus::option::__private
