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
#include "subspace/mem/move.h"
#include "subspace/mem/mref.h"
#include "subspace/mem/relocate.h"
#include "subspace/mem/size_of.h"
#include "subspace/ptr/copy.h"

namespace sus::mem {

/// Swaps the objects `lhs` and `rhs`.
///
/// If both inputs point to the same object, no swap takes place, so no move
/// constructor/operator is called.
template <class T>
  requires(sus::mem::Move<T> && !std::is_const_v<T>)
inline constexpr void swap(T& lhs, T& rhs) noexcept {
  if (::sus::mem::addressof(lhs) != ::sus::mem::addressof(rhs)) [[likely]] {
    if constexpr (::sus::mem::relocate_by_memcpy<T>) {
      // copy_one() is not constexpr so we can't use it in constexpr evaluation.
      if (!std::is_constant_evaluated()) {
        constexpr auto data_size = ::sus::mem::data_size_of<T>();
        char temp[data_size];
        ::sus::ptr::copy_nonoverlapping(
            ::sus::marker::unsafe_fn,
            reinterpret_cast<char*>(::sus::mem::addressof(lhs)), temp,
            data_size);
        ::sus::ptr::copy_nonoverlapping(
            ::sus::marker::unsafe_fn,
            reinterpret_cast<char*>(::sus::mem::addressof(rhs)),
            reinterpret_cast<char*>(::sus::mem::addressof(lhs)), data_size);
        ::sus::ptr::copy_nonoverlapping(
            ::sus::marker::unsafe_fn, temp,
            reinterpret_cast<char*>(::sus::mem::addressof(rhs)), data_size);
        return;
      }
    }
    auto temp = T(::sus::move(lhs));
    lhs = ::sus::move(rhs);
    rhs = ::sus::move(temp);
  }
}

/// Swaps the objects `lhs` and `rhs`.
///
/// # Safety
/// The inputs must not both refer to the same object, and the objects must not
/// be overlapping, or Undefined Behaviour may result.
template <class T>
  requires(sus::mem::Move<T> && !std::is_const_v<T>)
inline constexpr void swap_nonoverlapping(::sus::marker::UnsafeFnMarker, T& lhs,
                                          T& rhs) noexcept {
  if constexpr (::sus::mem::relocate_by_memcpy<T>) {
    // memcpy() is not constexpr so we can't use it in constexpr evaluation.
    if (!std::is_constant_evaluated()) {
      constexpr auto data_size = ::sus::mem::data_size_of<T>();
      alignas(alignof(T)) char temp[data_size];
      char* l = reinterpret_cast<char*>(::sus::mem::addressof(lhs));
      char* r = reinterpret_cast<char*>(::sus::mem::addressof(rhs));
      ::sus::ptr::copy_nonoverlapping(::sus::marker::unsafe_fn, l, temp,
                                      data_size);
      ::sus::ptr::copy_nonoverlapping(::sus::marker::unsafe_fn, r, l,
                                      data_size);
      ::sus::ptr::copy_nonoverlapping(::sus::marker::unsafe_fn, temp, r,
                                      data_size);
      return;
    }
  }
  // Narrow the scope of the `__restrict` annotations as casting a
  // `__restrict` pointer to make a new pointer is UB.
  T& __restrict l = lhs;
  T& __restrict r = rhs;
  auto temp = T(::sus::move(l));
  l = ::sus::move(r);
  r = ::sus::move(temp);
}

}  // namespace sus::mem
