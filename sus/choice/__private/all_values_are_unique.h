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

namespace sus::choice_type::__private {

template <auto... Vs>
struct AllValuesAreUniqueHelper;

template <auto V>
struct AllValuesAreUniqueHelper<V> {
  // One value left, it's unique with itself.
  static constexpr bool value = true;
};

template <auto V, auto V2>
struct AllValuesAreUniqueHelper<V, V2> {
  // Two different values are unique.
  static constexpr bool value = !(V == V2);
};

template <auto V, auto V2, auto... Vs>
  requires(sizeof...(Vs) > 0)
struct AllValuesAreUniqueHelper<V, V2, Vs...> {
  // Two different values, now compare them to the rest.
  static constexpr bool value = !(V == V2) &&
                                AllValuesAreUniqueHelper<V, Vs...>::value &&
                                AllValuesAreUniqueHelper<V2, Vs...>::value;
};

template <auto... Vs>
concept AllValuesAreUnique = AllValuesAreUniqueHelper<Vs...>::value;

}  // namespace sus::choice_type::__private
