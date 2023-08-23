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
struct [[nodiscard]] ArrayMarker {
  sus_clang_bug_54040(
      constexpr inline ArrayMarker(::sus::tuple_type::Tuple<Ts&&...>&& values)
      : values(::sus::move(values)){});

  ::sus::tuple_type::Tuple<Ts&&...> values;

  // Gtest macros force evaluation against a const reference.
  // https://github.com/google/googletest/issues/4350
  //
  // So we allow constructing the target type from a const ref `value`. This
  // can perform explicit conversions which isn't correct but is required in
  // order to be able to call this method twice, because `value` itself may not
  // be copyable (marker types are not) but construction from const is more
  // likely.
  template <class U>
  inline constexpr operator Array<U, sizeof...(Ts)>() const& noexcept {
    static_assert(
        (... && std::convertible_to<::sus::mem::remove_rvalue_reference<Ts>, U>),
        "ArrayMarker(T) can't convert const T& to U for Array<U>. "
        "Note that this is a test-only code path for Gtest support. "
        "Typically the T object is consumed as an rvalue, consider using "
        "EXPECT_TRUE(a == b) if needed.");
    static_assert((... && std::convertible_to<Ts&&, U>),
                  "ArrayMarker(T) can't convert T to U for Array<U>");
    auto make_array =
        [this]<size_t... Is>(std::integer_sequence<size_t, Is...>) {
          return Array<U, sizeof...(Is)>(U(values.template at<Is>())...);
        };
    return make_array(std::make_integer_sequence<size_t, sizeof...(Ts)>());
  }

  template <class U>
  inline constexpr operator Array<U, sizeof...(Ts)>() && noexcept {
    static_assert((... && std::convertible_to<Ts&&, U>),
                  "ArrayMarker(T) can't convert T to U for Array<U>");
    auto make_array =
        [this]<size_t... Is>(std::integer_sequence<size_t, Is...>) {
          return Array<U, sizeof...(Is)>(
              ::sus::forward<Ts>(values.template at_mut<Is>())...);
        };
    return make_array(std::make_integer_sequence<size_t, sizeof...(Ts)>());
  }

  template <class T>
  inline constexpr Array<T, sizeof...(Ts)> construct() && noexcept {
    return ::sus::move(*this);
  }

  template <int&..., class T = ::sus::choice_type::__private::PackFirst<Ts...>>
    requires(... && std::same_as<T, Ts>)
  inline constexpr Array<T, sizeof...(Ts)> construct() && noexcept {
    return ::sus::move(*this);
  }
};

}  // namespace sus::collections::__private
