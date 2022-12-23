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

#include <type_traits>

#include "macros/no_unique_address.h"

#pragma once

namespace sus::result::__private {

enum WithT { kWithT };
enum WithE { kWithE };

template <class T, class E>
union Storage {
  constexpr Storage() {}

  constexpr Storage(WithT, const T& t) noexcept : ok_(t) {}
  constexpr Storage(WithT, T&& t) noexcept : ok_(::sus::move(t)) {}
  constexpr Storage(WithE, const E&& e) noexcept : err_(e) {}
  constexpr Storage(WithE, E&& e) noexcept : err_(::sus::move(e)) {}

  constexpr ~Storage()
    requires(std::is_trivially_destructible_v<T> &&
             std::is_trivially_destructible_v<E>)
  = default;
  constexpr ~Storage()
    requires(!(std::is_trivially_destructible_v<T> &&
               std::is_trivially_destructible_v<E>))
  {}

  constexpr Storage(Storage&&)
    requires(std::is_trivially_move_constructible_v<T> &&
             std::is_trivially_move_constructible_v<E>)
  = default;
  constexpr Storage& operator=(Storage&&)
    requires(std::is_trivially_move_assignable_v<T> &&
             std::is_trivially_move_assignable_v<E>)
  = default;

  [[sus_no_unique_address]] T ok_;
  [[sus_no_unique_address]] E err_;
};

}  // namespace sus::result::__private
