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

#include "macros/__private/compiler_bugs.h"
#include "mem/forward.h"
#include "tuple/tuple.h"

namespace sus::choice_type {
template <class TypeListOfMemberTypes, auto... Tags>
class Choice;
}

namespace sus::choice_type::__private {

template <auto Tag, class... Ts>
struct ChoiceMarker;

template <auto Tag>
struct ChoiceMarkerVoid {
  template <class TypeListOfMemberTypes, auto... Vs>
  inline constexpr operator Choice<TypeListOfMemberTypes, Vs...>() &&noexcept {
    return Choice<TypeListOfMemberTypes, Vs...>::template with<Tag>();
  }
};

template <auto Tag, class T>
struct ChoiceMarker<Tag, T> {
  sus_clang_bug_54040(constexpr inline ChoiceMarker(T &&value)
                      : value(::sus::forward<T>(value)){});

  T &&value;

  template <class TypeListOfMemberTypes, auto... Vs>
  inline constexpr operator Choice<TypeListOfMemberTypes, Vs...>() &&noexcept {
    return Choice<TypeListOfMemberTypes, Vs...>::template with<Tag>(
        ::sus::forward<T>(value));
  }
};

template <auto Tag, class... Ts>
  requires(sizeof...(Ts) > 1)
struct ChoiceMarker<Tag, Ts...> {
  sus_clang_bug_54040(constexpr inline ChoiceMarker(Ts &&...value)
                      : values(::sus::tuple_type::Tuple<Ts &&...>::with(
                          ::sus::forward<Ts>(value)...)){});

  ::sus::tuple_type::Tuple<Ts &&...> values;

  template <class TypeListOfMemberTypes, auto... Vs>
  inline constexpr operator Choice<TypeListOfMemberTypes, Vs...>() &&noexcept {
    using TupleType =
        decltype(std::declval<Choice<TypeListOfMemberTypes, Vs...> &&>()
                     .template into_inner<Tag>());
    auto make_tuple =
        [this]<size_t... Is>(std::integer_sequence<size_t, Is...>) {
          return TupleType::with(
              ::sus::forward<Ts>(values.template get_mut<Is>())...);
        };
    return Choice<TypeListOfMemberTypes, Vs...>::template with<Tag>(
        make_tuple(std::make_integer_sequence<size_t, sizeof...(Ts)>()));
  }
};

}  // namespace sus::choice_type::__private
