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

#include "subspace/marker/unsafe.h"
#include "subspace/mem/addressof.h"
#include "subspace/mem/copy.h"
#include "subspace/mem/move.h"
#include "subspace/mem/mref.h"
#include "subspace/mem/relocate.h"
#include "subspace/mem/size_of.h"

namespace sus::mem {

// It would be nice to have an array overload of replace() but functions can't
// return arrays.

template <class T>
  requires(!std::is_array_v<T> && ::sus::mem::Move<T> && ::sus::mem::Copy<T>)
[[nodiscard]] inline constexpr T replace(T& dest, const T& src) noexcept {
  auto old = T(::sus::move(dest));

  // memcpy() is not constexpr so we can't use it in constexpr evaluation.
  bool can_memcpy =
      std::is_trivially_copy_assignable_v<T> && !std::is_constant_evaluated();
  if (can_memcpy) {
    memcpy(::sus::mem::addressof(dest), ::sus::mem::addressof(src),
           ::sus::mem::data_size_of<T>());
  } else {
    dest = src;
  }

  return old;
}

template <class T>
  requires(!std::is_array_v<T> && ::sus::mem::Move<T>)
[[nodiscard]] inline constexpr T replace(T& dest, T&& src) noexcept {
  auto old = T(::sus::move(dest));

  if constexpr (std::is_trivially_move_assignable_v<T>) {
    if (!std::is_constant_evaluated()) {
      memcpy(::sus::mem::addressof(dest), ::sus::mem::addressof(src),
             ::sus::mem::data_size_of<T>());
      return old;
    }
  }

  dest = ::sus::move(src);
  return old;
}

template <class T>
  requires(!std::is_array_v<T> && sus::mem::Copy<T>)
inline void replace_and_discard(T& dest, const T& src) noexcept {
  if constexpr (std::is_trivially_copy_assignable_v<T>) {
    if (!std::is_constant_evaluated()) {
      memcpy(::sus::mem::addressof(dest), ::sus::mem::addressof(src),
             ::sus::mem::data_size_of<T>());
      return;
    }
  }

  dest = src;
}

template <class T>
  requires(!std::is_array_v<T> && sus::mem::Move<T>)
inline void replace_and_discard(T& dest, T&& src) noexcept {
  if constexpr (std::is_trivially_move_assignable_v<T>) {
    if (!std::is_constant_evaluated()) {
      memcpy(::sus::mem::addressof(dest), ::sus::mem::addressof(src),
             ::sus::mem::data_size_of<T>());
    }
  }

  dest = ::sus::move(src);
}

template <class T>
[[nodiscard]] inline constexpr T* replace_ptr(T*& dest, T* src) noexcept {
  T* old = dest;
  dest = src;
  return old;
}

template <class T>
[[nodiscard]] inline constexpr T* replace_ptr(T*& dest,
                                              std::nullptr_t) noexcept {
  T* old = dest;
  dest = nullptr;
  return old;
}

}  // namespace sus::mem
