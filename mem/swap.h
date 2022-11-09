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

#include <string.h>

#include <type_traits>

#include "mem/relocate.h"
#include "mem/addressof.h"
#include "mem/mref.h"

namespace sus::mem {

template <class T>
  requires(std::is_move_constructible_v<T> && std::is_move_assignable_v<T>)
constexpr void swap(Mref<T> lhs_ref, Mref<T> rhs_ref) noexcept {
  T& lhs = lhs_ref;
  T& rhs = rhs_ref;
  // memcpy() is not constexpr so we can't use it in constexpr evaluation.
  bool can_memcpy = ::sus::mem::relocate_one_by_memcpy<T> &&
                    !std::is_constant_evaluated();
  if (can_memcpy) {
    char temp[sizeof(T)];
    memcpy(temp, ::sus::mem::addressof(lhs), sizeof(T));
    memcpy(::sus::mem::addressof(lhs), ::sus::mem::addressof(rhs), sizeof(T));
    memcpy(::sus::mem::addressof(rhs), temp, sizeof(T));
  } else {
    T temp(static_cast<T&&>(lhs));
    lhs = static_cast<T&&>(rhs);
    rhs = static_cast<T&&>(temp);
  }
}

}  // namespace sus::mem
