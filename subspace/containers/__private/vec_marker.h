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

#include "tuple/tuple.h"

namespace sus::containers::__private {

template <class... Ts>
struct VecMarker {
  ::sus::tuple_type::Tuple<Ts &&...> values;

  template <class U>
  inline constexpr operator Vec<U>() &&noexcept {
    auto v = Vec<U>::with_capacity(sizeof...(Ts));

    auto push_elements =
        [&, this]<size_t... Is>(std::integer_sequence<size_t, Is...>) {
          // This is a fold expression over the operator `,`.
          (v.push(::sus::forward<Ts>(values.template get_mut<Is>())), ...);
        };
    push_elements(std::make_integer_sequence<size_t, sizeof...(Ts)>());

    return v;
  }
};

}  // namespace sus::containers::__private
