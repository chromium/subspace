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

// It would be nice to have an array overload of replace() but functions can't
// return arrays.

template <class T>
  requires(!std::is_array_v<T> && std::is_move_constructible_v<T> && std::is_copy_assignable_v<T>)
[[nodiscard]] constexpr T replace(T& dest, const T& src) noexcept {
  T old(static_cast<T&&>(dest));

  // memcpy() is not constexpr so we can't use it in constexpr evaluation.
  if (::sus::mem::__private::relocate_one_by_memcpy_v<T> &&
      !std::is_constant_evaluated()) {
    memcpy(::sus::mem::addressof(dest), ::sus::mem::addressof(src), sizeof(T));
  } else {
    dest = src;
  }

  return old;
}

template <class T>
  requires(!std::is_array_v<T> && std::is_move_constructible_v<T> && std::is_move_assignable_v<T>)
[[nodiscard]] constexpr T replace(T& dest, T&& src) noexcept {
  T old(static_cast<T&&>(dest));

  // memcpy() is not constexpr so we can't use it in constexpr evaluation.
  if (::sus::mem::__private::relocate_one_by_memcpy_v<T> &&
      !std::is_constant_evaluated()) {
    memcpy(::sus::mem::addressof(dest), ::sus::mem::addressof(src), sizeof(T));
  } else {
    dest = static_cast<T&&>(src);
  }

  return old;
}

template <class T>
  requires(!std::is_array_v<T> && std::is_copy_assignable_v<T>)
void replace_and_discard(T& dest, const T& src) noexcept {
  // memcpy() is not constexpr so we can't use it in constexpr evaluation.
  if (::sus::mem::__private::relocate_one_by_memcpy_v<T> &&
      !std::is_constant_evaluated()) {
    memcpy(::sus::mem::addressof(dest), ::sus::mem::addressof(src), sizeof(T));
  } else {
    dest = src;
  }
}

template <class T>
  requires(!std::is_array_v<T> && std::is_move_assignable_v<T>)
void replace_and_discard(T& dest, T&& src) noexcept {
  // memcpy() is not constexpr so we can't use it in constexpr evaluation.
  if (::sus::mem::__private::relocate_one_by_memcpy_v<T> &&
      !std::is_constant_evaluated()) {
    memcpy(::sus::mem::addressof(dest), ::sus::mem::addressof(src), sizeof(T));
  } else {
    dest = static_cast<T&&>(src);
  }
}

template <class T>
[[nodiscard]] constexpr T* replace_ptr(T*& dest, T* src) noexcept {
  T* old = dest;
  dest = src;
  return old;
}

template <class T>
[[nodiscard]] constexpr const T* replace_ptr(const T*& dest, const T* src) noexcept {
  const T* old = dest;
  dest = src;
  return old;
}

template <class T>
[[nodiscard]] constexpr T* replace_ptr(T*& dest, decltype(nullptr) src) noexcept {
  T* old = dest;
  dest = nullptr;
  return old;
}

template <class T>
[[nodiscard]] constexpr const T* replace_ptr(const T*& dest, decltype(nullptr) src) noexcept {
  const T* old = dest;
  dest = nullptr;
  return old;
}

}  // namespace sus::mem
