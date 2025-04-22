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

#include <string.h>

#include <type_traits>

#include "sus/assertions/debug_check.h"
#include "sus/marker/unsafe.h"
#include "sus/mem/size_of.h"
#include "sus/num/unsigned_integer.h"

namespace sus::ptr {

/// Copies `count * size_of<T>()` bytes from `src` to `dst`. The source and
/// destination must not overlap.
///
/// For regions of memory which might overlap, use [`copy`]($sus::ptr::copy)
/// instead.
///
/// [`copy_nonoverlapping`]($sus::ptr::copy_nonoverlapping) is semantically
/// equivalent to
/// [`memcpy`](https://en.cppreference.com/w/cpp/string/byte/memcpy), but with the argument order swapped.
///
/// The copy is “untyped” in the sense that data may be uninitialized or
/// otherwise violate the requirements of `T`. The initialization state is
/// preserved exactly.
///
/// # Panics
/// This function will panic if the number of bytes, `count * size_of<T>()`,
/// overflows.
///
/// # Safety
/// Behavior is undefined if any of the following conditions are violated:
/// * `src` must be valid for reads of `count * size_of<T>()` bytes.
/// * `dst` must be valid for writes of `count * size_of<T>()` bytes.
/// * The region of memory beginning at `src` with a size of `count *
///   size_of<T>()` bytes must not overlap with the region of memory beginning
///   at `dst` with the same size.
/// * `dst` must not have an overlapping object in its tail padding. If `dst` is
///   in an array, or was heap allocated, then this will always be satisfied.
///
/// Like [`copy`]($sus::ptr::copy),
/// [`copy_nonoverlapping`]($sus::ptr::copy_nonoverlapping) creates a bitwise
/// copy of `T`,
/// regardless of whether `T` is [`TrivialCopy`]($sus::mem::TrivialCopy)
/// or [`TriviallyRelocatable`]($sus::mem::TriviallyRelocatable). If `T`
/// is not [`TrivialCopy`]($sus::mem::TrivialCopy), using the value in
/// `*dst` can violate memory safety.
/// If `T` is not [`TriviallyRelocatable`]($sus::mem::TriviallyRelocatable),
/// using both the values in `*src` and in `*dst` can violate memory safety.
///
/// Note that even if the effectively copied size (`count * size_of<T>()`) is 0,
/// the pointers must be non-null and properly aligned.
template <class T>
  requires(!std::is_const_v<T>)
void copy_nonoverlapping(::sus::marker::UnsafeFnMarker, const T* src, T* dst,
                         ::sus::num::usize count) noexcept {
  sus_debug_check(src != nullptr);
  sus_debug_check(dst != nullptr);
  // UBSan won't catch the misaligned read/writes by memcpy, so we check it
  // ourselves.
  sus_debug_check(reinterpret_cast<uintptr_t>(src) % alignof(T) == 0);
  sus_debug_check(reinterpret_cast<uintptr_t>(dst) % alignof(T) == 0);
  sus_debug_check((src < dst && src <= dst - count.primitive_value) ||
                  (dst < src && dst <= src - count.primitive_value));
  if constexpr (::sus::mem::size_of<T>() > 1) {
    auto bytes = count.checked_mul(::sus::mem::size_of<T>()).expect("overflow");
    // Clang isn't able to detect that `T` is trivially relocatable, and errors
    // out when it's not trivially copyable. We cast to `void*` since we are
    // certain that it's trivially relocatable.
    memcpy(static_cast<void*>(dst), src, bytes);
  } else {
    memcpy(static_cast<void*>(dst), src, count);
  }
}

/// Copies `count * size_of<T>()` bytes from `src` to `dst`. The source and
/// destination may overlap.
///
/// If the source and destination will never overlap,
/// [`copy_nonoverlapping`]($sus::ptr::copy_nonoverlapping)
/// can be used instead.
///
/// [`copy`]($sus::ptr::copy) is semantically equivalent to
/// [`memmove`](https://en.cppreference.com/w/c/string/byte/memmove), but
/// with the argument order swapped.
/// Copying takes place as if the bytes were copied from `src` to
/// a temporary array and then copied from the array to `dst`.
///
/// The copy is “untyped” in the sense that data may be uninitialized or
/// otherwise violate the requirements of `T`. The initialization state is
/// preserved exactly.
///
/// # Panics
/// This function will panic if the number of bytes, `count * size_of<T>()`,
/// overflows.
///
/// # Safety
/// Behavior is undefined if any of the following conditions are violated:
/// * `src` must be valid for reads of `count * size_of<T>()` bytes.
/// * `dst` must be valid for writes of `count * size_of<T>()` bytes.
/// * `dst` must not have an overlapping object in its tail padding. If `dst` is
///   in an array, or was heap allocated, then this will always be satisfied.
///
/// [`copy`]($sus::ptr::copy) creates a bitwise copy of `T`, regardless of
/// whether `T` is
/// [`TrivialCopy`]($sus::mem::TrivialCopy) or
/// [`TriviallyRelocatable`]($sus::mem::TriviallyRelocatable). If `T` is not
/// [`TrivialCopy`]($sus::mem::TrivialCopy), using
/// the value in `*dst` can violate memory safety. If `T` is not
/// [`TriviallyRelocatable`]($sus::mem::TriviallyRelocatable), using both the
/// values in `*src` and in `*dst` can violate memory safety.
///
/// Note that even if the effectively copied size (`count * size_of<T>()`) is 0,
/// the pointers must be non-null and properly aligned.
template <class T>
  requires(!std::is_const_v<T>)
void copy(::sus::marker::UnsafeFnMarker, const T* src, T* dst,
          ::sus::num::usize count) noexcept {
  sus_debug_check(src != nullptr);
  sus_debug_check(dst != nullptr);
  // UBSan won't catch the misaligned read/writes by memcpy, so we check it
  // ourselves.
  sus_debug_check(reinterpret_cast<uintptr_t>(src) % alignof(T) == 0);
  sus_debug_check(reinterpret_cast<uintptr_t>(dst) % alignof(T) == 0);
  if constexpr (::sus::mem::size_of<T>() > 1) {
    auto bytes = count.checked_mul(::sus::mem::size_of<T>()).expect("overflow");
    memmove(dst, src, bytes);
  } else {
    memmove(dst, src, count);
  }
}

}  // namespace sus::ptr
