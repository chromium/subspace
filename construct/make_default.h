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

#include <concepts>

namespace sus::construct {

namespace __private {

template <class T>
concept HasWithDefault = requires {
  { T::with_default() } -> std::same_as<T>;
};

}  // namespace __private

// clang-format off
template <class T>
concept MakeDefault = 
  (std::constructible_from<T> && !__private::HasWithDefault<T>) ||
  (!std::constructible_from<T> && __private::HasWithDefault<T>);
// clang-format on

template <MakeDefault T>
inline constexpr T make_default() noexcept {
  if constexpr (std::constructible_from<T>)
    return T();
  else
    return T::with_default();
}

template <MakeDefault T>
  requires(std::is_move_constructible_v<T>)
inline T* alloc_make_default() noexcept {
  if constexpr (std::constructible_from<T>)
    return new T();
  else
    return new T(T::with_default());
}

}  // namespace sus::construct
