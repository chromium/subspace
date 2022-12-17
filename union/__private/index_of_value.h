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

#include <type_traits>

namespace sus::union_type::__private {

template <auto SearchValue, size_t I, auto... Vs>
struct IndexOfValueHelper;

template <auto SearchValue, size_t I, auto... Vs>
struct IndexOfValueHelper<SearchValue, I, SearchValue, Vs...> {
  using index = std::integral_constant<size_t, I>;  // We found the SearchValue.
};

template <auto SearchValue, size_t I, auto V, auto... Vs>
struct IndexOfValueHelper<SearchValue, I, V, Vs...> {
  static_assert(SearchValue != V);
  static constexpr auto next_index = I + size_t{1u};
  using index = IndexOfValueHelper<SearchValue, next_index, Vs...>::index;
};

template <auto SearchValue, size_t I>
struct IndexOfValueHelper<SearchValue, I> {
  using index =
      void;  // We didn't find the SearchValue, it's not part of the Union.
};

template <auto SearchValue, auto... Vs>
using IndexOfValue = IndexOfValueHelper<SearchValue, 0u, Vs...>::index;

}  // namespace sus::union_type::__private
