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

#include "mem/__private/relocate.h"
#include "mem/addressof.h"

namespace sus::mem {

template <class T>
constexpr void swap(T& lhs, T& rhs) noexcept {
  // memcpy() is not constexpr so we can't use it in constexpr evaluation.
  if (::sus::mem::__private::relocate_one_by_memcpy_v<T> &&
      !std::is_constant_evaluated()) {
    char temp[sizeof(T)];
    memcpy(temp, ::sus::mem::addressof(lhs), sizeof(T));
    memcpy(::sus::mem::addressof(lhs), ::sus::mem::addressof(rhs), sizeof(T));
    memcpy(::sus::mem::addressof(rhs), temp, sizeof(T));
  } else {
    T temp(static_cast<T&&>(lhs));
    lhs = T(static_cast<T&&>(rhs));
    rhs = T(static_cast<T&&>(temp));
  }
}

template <class T, /* TODO: usize */ size_t N>
constexpr void swap(T (&lhs)[N], T (&rhs)[N]) noexcept {
  // memcpy() is not constexpr so we can't use it in constexpr evaluation.
  if (::sus::mem::__private::relocate_array_by_memcpy_v<T> &&
      !std::is_constant_evaluated()) {
    char temp[sizeof(T[N])];
    memcpy(temp, lhs, sizeof(T[N]));
    memcpy(lhs, rhs, sizeof(T[N]));
    memcpy(rhs, temp, sizeof(T[N]));
  } else {
    for (/*TODO: usize*/ size_t i = 0; i < N; ++i) {
      T temp(static_cast<T&&>(lhs[i]));
      lhs[i] = T(static_cast<T&&>(rhs[i]));
      rhs[i] = T(static_cast<T&&>(temp));
    }
  }
}
}  // namespace sus::mem
