// Copyright 2023 Google LLC
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

#include "subspace/assertions/debug_check.h"
#include "subspace/macros/assume.h"
#include "subspace/marker/unsafe.h"
#include "subspace/mem/relocate.h"
#include "subspace/mem/size_of.h"
#include "subspace/mem/swap.h"
#include "subspace/num/unsigned_integer.h"
#include "subspace/option/option.h"

namespace sus::ptr {

/// Swaps the object array at `x` with the object array at `y`, where both
/// arrays have a length of `count`.
///
/// If `T` is trivially relocatable (`sus::mem::relocate_by_memcpy<T>` is true),
/// then the swap may be done by memcpy() or equivalent to be more efficient.
///
/// # Safety
/// To avoid Undefined Behaviour, the following must be met:
/// * The pointers `x` and `y` must both be non-null and properly aligned for
///   `T`.
/// * The memory region at `x` and including `count` elements must not overlap
///   the region at `y` including `count` elements.
/// * The objects at `x` and `y` must not have an overlapping object in their
///   tail padding. If `x` and `y` are arrays, or were heap allocated, then this
///   will always be satisfied.
template <class T>
  requires(sus::mem::Move<T> && !std::is_const_v<T>)
inline constexpr void swap_nonoverlapping(::sus::marker::UnsafeFnMarker, T* x,
                                          T* y, usize count) noexcept {
  // Nonnull.
  sus_debug_check(x != nullptr);
  sus_debug_check(y != nullptr);
  // Well aligned.
  if (!std::is_constant_evaluated()) {
    sus_debug_check(reinterpret_cast<uintptr_t>(x) % alignof(T) == 0);
    sus_debug_check(reinterpret_cast<uintptr_t>(y) % alignof(T) == 0);
  }
  // Non-overlapping.
  sus_debug_check((x <= y && x <= y - count.primitive_value) ||
                  (y <= x && y <= x - count.primitive_value));

  constexpr usize t_size = ::sus::mem::size_of<T>();

  // Split up the slice into small power-of-two-sized chunks that LLVM is able
  // to vectorize (unless it's a special type with more-than-pointer alignment,
  // because we don't want to pessimize things like slices of SIMD vectors.)
  constexpr bool has_split_up_alignment = alignof(T) <= alignof(uintptr_t);
  // Small types (<= 2 pointers) and power-of-two-sized types will already
  // vectorize well, so we don't need to split them up.
  constexpr bool has_split_up_size =
      !t_size.is_power_of_two() ||
      t_size > ::sus::mem::size_of<uintptr_t>() * 2;

  const ::sus::Option<usize> opt_byte_count = count.checked_mul(t_size);

  if (std::is_constant_evaluated() || !::sus::mem::relocate_by_memcpy<T> ||
      !has_split_up_alignment || !has_split_up_size ||
      opt_byte_count.is_none()) {
    // If the type can be vectorized already, or can't be relocated by memcpy,
    // then swap in chunks of size of T.
    for (usize i; i < count; i += 1u) {
      ::sus::mem::swap_nonoverlapping(::sus::marker::unsafe_fn, *(x + i),
                                      *(y + i));
    }
    return;
  }

  const usize byte_count = *opt_byte_count;

  // If the type's alignment matches uintptr_t and the size is a multiple of
  // it, then we do the swap in chunks of uintptr_t.
  if constexpr (alignof(T) == alignof(uintptr_t) &&
                t_size % ::sus::mem::size_of<uintptr_t>() == 0u) {
    char* const cx = reinterpret_cast<char*>(x);
    char* const cy = reinterpret_cast<char*>(y);
    alignas(alignof(uintptr_t)) char buf[::sus::mem::size_of<uintptr_t>()];
    for (usize i; i < byte_count; i += ::sus::mem::size_of<uintptr_t>()) {
      ::sus::ptr::copy_nonoverlapping(::sus::marker::unsafe_fn, cx + i, buf,
                                      ::sus::mem::size_of<uintptr_t>());
      ::sus::ptr::copy_nonoverlapping(::sus::marker::unsafe_fn, cy + i, cx + i,
                                      ::sus::mem::size_of<uintptr_t>());
      ::sus::ptr::copy_nonoverlapping(::sus::marker::unsafe_fn, buf, cy + i,
                                      ::sus::mem::size_of<uintptr_t>());
    }
  } else {
    // Otherwise, we swap a byte at a time.
    char* const __restrict cx = reinterpret_cast<char*>(x);
    char* const __restrict cy = reinterpret_cast<char*>(y);
    sus_debug_check((cx <= cy && cx <= cy - byte_count.primitive_value) ||
                    (cy <= cx && cy <= cx - byte_count.primitive_value));
    char buf;
    for (usize i; i < byte_count; i += 1u) {
      buf = *(cx + i);
      *(cx + i) = *(cy + i);
      *(cy + i) = buf;
    }
  }
}

}  // namespace sus::ptr
