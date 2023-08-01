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

namespace sus::choice_type::__private {

template <class... Ts>
struct PackFirstHelper;

template <class T, class... Ts>
struct PackFirstHelper<T, Ts...> {
  using type = T;
};

template <class... Ts>
using PackFirst = PackFirstHelper<Ts...>::type;

template <size_t I, class... Ts>
struct PackIthHelper;

template <size_t I, class T, class... Ts>
struct PackIthHelper<I, T, Ts...> {
  using type = PackIthHelper<I - 1, Ts...>::type;
};

template <class T, class... Ts>
struct PackIthHelper<0, T, Ts...> {
  using type = T;
};

template <size_t I, class... Ts>
using PackIth = PackIthHelper<I, Ts...>::type;

}  // namespace sus::choice_type::__private
