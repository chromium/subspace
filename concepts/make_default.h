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

namespace sus::concepts {

namespace __private {

template <class T, class Signature>
constexpr inline bool has_with_default(...) {
  return false;
}

template <class T, class R, class... Args>
  requires(
      std::is_same_v<decltype(T::with_default(std::declval<Args>()...)), R>)
constexpr inline bool has_with_default(int) {
  return true;
}

}  // namespace __private

template <class T>
struct MakeDefault final {
  static constexpr bool has_concept =
      std::is_default_constructible_v<T> ^ __private::has_with_default<T, T>(0);

  static constexpr T make_default() noexcept {
    static_assert(has_concept,
                  "MakeDefault trait used when the trait is not present for "
                  "type T. Verify with MakeDefault<T>::has_concept.");
    if constexpr (std::is_default_constructible_v<T>)
      return T();
    else
      return T::with_default();
  }
};

template <class T>
struct MakeDefault<T&&> {
  static constexpr bool has_concept = false;
};
template <class T>
struct MakeDefault<T&> {
  static constexpr bool has_concept = false;
};
template <class T>
struct MakeDefault<const T&> {
  static constexpr bool has_concept = false;
};

}  // namespace sus::concepts
