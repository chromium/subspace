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
#include "mem/move.h"
#include "tuple/tuple.h"

namespace sus::containers::__private {

template <class... Ts>
struct ArrayMarker {
  sus_clang_bug_54040(
      constexpr inline ArrayMarker(::sus::tuple_type::Tuple<Ts&&...>&& values)
      : values(::sus::move(values)){});

  ::sus::tuple_type::Tuple<Ts&&...> values;

  template <class U>
  inline constexpr operator Array<U, sizeof...(Ts)>() && noexcept {
    auto make_array =
        [this]<size_t... Is>(std::integer_sequence<size_t, Is...>) {
          return Array<U, sizeof...(Is)>::with_values(
              ::sus::forward<Ts>(values.template get_mut<Is>())...);
        };
    return make_array(std::make_integer_sequence<size_t, sizeof...(Ts)>());
  }
};

}  // namespace sus::containers::__private
