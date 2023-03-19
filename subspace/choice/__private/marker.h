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

#pragma once

#include <stddef.h>

#include <utility>  // TODO: Replace with our own integer_sequence.

#include "subspace/macros/__private/compiler_bugs.h"
#include "subspace/mem/forward.h"
#include "subspace/tuple/tuple.h"

namespace sus::choice_type {
template <class TypeListOfMemberTypes, auto... Tags>
class Choice;
}

namespace sus::choice_type::__private {

template <auto Tag, class... Ts>
struct ChoiceMarker;

template <auto Tag>
struct ChoiceMarkerVoid {
  // If the Choice's type can construct from a const ref `value` (roughly, is
  // copy-constructible, but may change types), then the marker can do the same.
  //
  // This largely exists to support use in Gtest's EXPECT_EQ, which uses them as
  // a const&, since marker types should normally be converted quickly to the
  // concrete type.
  template <class TypeListOfMemberTypes, auto... Vs>
  inline constexpr operator Choice<TypeListOfMemberTypes, Vs...>()
      const& noexcept {
    return Choice<TypeListOfMemberTypes, Vs...>::template with<Tag>();
  }

  template <class TypeListOfMemberTypes, auto... Vs>
  inline constexpr operator Choice<TypeListOfMemberTypes, Vs...>() && noexcept {
    return Choice<TypeListOfMemberTypes, Vs...>::template with<Tag>();
  }
};

template <auto Tag, class T>
struct ChoiceMarker<Tag, T> {
  sus_clang_bug_54040(constexpr inline ChoiceMarker(T&& value)
                      : value(::sus::forward<T>(value)){});

  T&& value;

  // If the Choice's type can construct from a const ref `value` (roughly, is
  // copy-constructible, but may change types), then the marker can do the same.
  //
  // This largely exists to support use in Gtest's EXPECT_EQ, which uses them as
  // a const&, since marker types should normally be converted quickly to the
  // concrete type.
  //
  // TODO: Write a requires class that verfies the Choice's storage type is
  // `constructible_from<const std::remove_reference_t<T>&>`.
  template <class TypeListOfMemberTypes, auto... Vs>
  inline constexpr operator Choice<TypeListOfMemberTypes, Vs...>()
      const& noexcept {
    return Choice<TypeListOfMemberTypes, Vs...>::template with<Tag>(value);
  }

  template <class TypeListOfMemberTypes, auto... Vs>
  inline constexpr operator Choice<TypeListOfMemberTypes, Vs...>() && noexcept {
    return Choice<TypeListOfMemberTypes, Vs...>::template with<Tag>(
        ::sus::forward<T>(value));
  }
};

template <auto Tag, class... Ts>
  requires(sizeof...(Ts) > 1)
struct ChoiceMarker<Tag, Ts...> {
  sus_clang_bug_54040(
      constexpr inline ChoiceMarker(::sus::tuple_type::Tuple<Ts&&...>&& values)
      : values(::sus::move(values)){});

  ::sus::tuple_type::Tuple<Ts&&...> values;

  // If the Choice's types can construct from const ref `values` (roughly, is
  // copy-constructible, but may change types), then the marker can do the same.
  //
  // This largely exists to support use in Gtest's EXPECT_EQ, which uses them as
  // a const&, since marker types should normally be converted quickly to the
  // concrete type.
  //
  // TODO: Write a requires class that verfies the Choice's storage types are
  // `constructible_from<const std::remove_reference_t<Ts>&>`.
  template <class TypeListOfMemberTypes, auto... Vs>
  inline constexpr operator Choice<TypeListOfMemberTypes, Vs...>()
      const& noexcept {
    using TupleType =
        decltype(std::declval<Choice<TypeListOfMemberTypes, Vs...>&&>()
                     .template into_inner<Tag>());
    auto make_tuple = [this]<size_t... Is>(
                          std::integer_sequence<size_t, Is...>) {
      return TupleType::with(values.template at<Is>()...);
    };
    return Choice<TypeListOfMemberTypes, Vs...>::template with<Tag>(
        make_tuple(std::make_integer_sequence<size_t, sizeof...(Ts)>()));
  }

  template <class TypeListOfMemberTypes, auto... Vs>
  inline constexpr operator Choice<TypeListOfMemberTypes, Vs...>() && noexcept {
    using TupleType =
        decltype(std::declval<Choice<TypeListOfMemberTypes, Vs...>&&>()
                     .template into_inner<Tag>());
    auto make_tuple =
        [this]<size_t... Is>(std::integer_sequence<size_t, Is...>) {
          return TupleType::with(
              ::sus::forward<Ts>(values.template at_mut<Is>())...);
        };
    return Choice<TypeListOfMemberTypes, Vs...>::template with<Tag>(
        make_tuple(std::make_integer_sequence<size_t, sizeof...(Ts)>()));
  }
};

}  // namespace sus::choice_type::__private
