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

#include <stddef.h>

#include <utility>  // TODO: Replace with our own integer_sequence.

#include "sus/choice/__private/pack_index.h"  // TODO: Move out of choice/ to share.
#include "sus/lib/__private/forward_decl.h"
#include "sus/macros/__private/compiler_bugs.h"
#include "sus/mem/move.h"
#include "sus/tuple/tuple.h"

namespace sus::collections::__private {

template <class... Ts>
struct VecEmptyMarker {
  // This largely exists to support use in Gtest's EXPECT_EQ, which uses them as
  // a const&, since marker types should normally be converted quickly to the
  // concrete type.
  template <class U>
  inline constexpr operator Vec<U>() const& noexcept {
    return Vec<U>();
  }

  template <class U>
  inline constexpr operator Vec<U>() && noexcept {
    return Vec<U>();
  }

  /// Constructs a `Vec<U>` for a user-specified `U`, as it can not be inferred.
  template <class U>
  inline constexpr Vec<U> construct() && noexcept {
    return ::sus::move(*this);
  }
};

template <class... Ts>
struct VecMarker {
  sus_clang_bug_54040(
      constexpr inline VecMarker(::sus::tuple_type::Tuple<Ts&&...>&& values)
      : values(::sus::move(values)){});

  ::sus::tuple_type::Tuple<Ts&&...> values;

  // If the Vec's type can construct from a const ref `values` (roughly, are
  // copy-constructible, but may change types), then the marker can do the same.
  //
  // This largely exists to support use in Gtest's EXPECT_EQ, which uses them as
  // a const&, since marker types should normally be converted quickly to the
  // concrete type.
  template <class U>
    requires((... && std::constructible_from<U, const Ts&>))
  inline constexpr operator Vec<U>() const& noexcept {
    auto v = Vec<U>::with_capacity(sizeof...(Ts));

    auto push_elements =
        [&, this]<size_t... Is>(std::integer_sequence<size_t, Is...>) {
          // This is a fold expression over the operator `,`.
          (v.push(values.template at<Is>()), ...);
        };
    push_elements(std::make_integer_sequence<size_t, sizeof...(Ts)>());

    return v;
  }

  template <class U>
    requires((... && std::constructible_from<U, Ts &&>))
  inline constexpr operator Vec<U>() && noexcept {
    auto v = Vec<U>::with_capacity(sizeof...(Ts));

    auto push_elements =
        [&, this]<size_t... Is>(std::integer_sequence<size_t, Is...>) {
          // This is a fold expression over the operator `,`.
          (v.push(::sus::forward<Ts>(values.template at_mut<Is>())), ...);
        };
    push_elements(std::make_integer_sequence<size_t, sizeof...(Ts)>());

    return v;
  }

  /// Constructs a `Vec<U>` for a user-specified `U`, as it can not be inferred.
  template <class U>
    requires((... && std::constructible_from<U, Ts &&>))
  inline constexpr Vec<U> construct() && noexcept {
    return ::sus::move(*this);
  }

  /// Constructs a `Vec<U>` where `U` is the exact type of the values passed to
  /// `sus::vec()`.
  ///
  /// This function is only callable if all values passed to `sus::vec()` had
  /// the same type.
  template <int&..., class U = ::sus::choice_type::__private::PackFirst<Ts...>>
    requires(... && std::same_as<U, Ts>)
  inline constexpr Vec<U> construct() && noexcept {
    return ::sus::move(*this);
  }
};

}  // namespace sus::collections::__private
