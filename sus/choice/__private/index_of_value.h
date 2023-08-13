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

#include <type_traits>

namespace sus::choice_type::__private {

template <auto SearchValue, size_t I, auto... Vs>
struct IndexOfValueHelper;

template <auto SearchValue, size_t I, auto... Vs>
struct IndexOfValueHelper<SearchValue, I, SearchValue, Vs...> {
  // We found the SearchValue.
  using index = std::integral_constant<size_t, I>;
};

template <auto SearchValue, size_t I, auto V, auto... Vs>
struct IndexOfValueHelper<SearchValue, I, V, Vs...> {
  static_assert(SearchValue != V);
  static constexpr size_t next_index = I + size_t{1u};
  // Still looking for the SearchValue, recurse.
  using index = IndexOfValueHelper<SearchValue, next_index, Vs...>::index;
};

template <auto SearchValue, size_t I>
struct IndexOfValueHelper<SearchValue, I> {
  // We didn't find the SearchValue, it's not part of the Choice.
  using index = void;
};

template <auto SearchValue, auto... Vs>
using IndexOfValue = IndexOfValueHelper<SearchValue, 0u, Vs...>::index;

template <auto Tag, auto... Tags>
static constexpr size_t get_index_for_value() noexcept {
  using Index = __private::IndexOfValue<Tag, Tags...>;
  static_assert(!std::is_void_v<Index>,
                "The Tag value is not part of the Choice.");
  return Index::value;
}

}  // namespace sus::choice_type::__private
