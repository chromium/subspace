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

// IWYU pragma: private
// IWYU pragma: friend "sus/.*"
#pragma once

#include <fenv.h>
#include <stddef.h>
#include <stdint.h>

#include <bit>
#include <cmath>
#include <type_traits>

#if _MSC_VER
#include <intrin.h>
#endif

#include "sus/assertions/unreachable.h"
#include "sus/macros/builtin.h"
#include "sus/macros/inline.h"
#include "sus/macros/pure.h"
#include "sus/marker/unsafe.h"
#include "sus/mem/size_of.h"
#include "sus/num/fp_category.h"

namespace sus::num::__private {

template <class T>
struct OverflowOut final {
  bool overflow;
  T value;
};

// The correct type to perform math operations on given values of type `T`. This
// may be a larger type than `T` to avoid promotion to `int` which involves sign
// conversion!
template <class T>
  requires(std::is_integral_v<T>)
using MathType = std::conditional_t<
    ::sus::mem::size_of<T>() >= ::sus::mem::size_of<int>(), T,
    std::conditional_t<std::is_signed_v<T>, int, unsigned int>>;

/// A sizeof() function that returns type uint32_t.
template <class T>
sus_pure_const sus_always_inline constexpr uint32_t
unchecked_sizeof() noexcept {
  static_assert(::sus::mem::size_of<T>() <= 0xfffffff);
  return static_cast<uint32_t>(::sus::mem::size_of<T>());
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T>)
sus_pure_const sus_always_inline constexpr T unchecked_neg(T x) noexcept {
  return static_cast<T>(-MathType<T>{x});
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T>)
sus_pure_const sus_always_inline constexpr T unchecked_not(T x) noexcept {
  return static_cast<T>(~MathType<T>{x});
}

template <class T>
  requires(std::is_integral_v<T>)
sus_pure_const sus_always_inline constexpr T unchecked_add(T x, T y) noexcept {
  return static_cast<T>(MathType<T>{x} + MathType<T>{y});
}

template <class T>
  requires(std::is_integral_v<T>)
sus_pure_const sus_always_inline constexpr T unchecked_sub(T x, T y) noexcept {
  return static_cast<T>(MathType<T>{x} - MathType<T>{y});
}

template <class T>
  requires(std::is_integral_v<T>)
sus_pure_const sus_always_inline constexpr T unchecked_mul(T x, T y) noexcept {
  return static_cast<T>(MathType<T>{x} * MathType<T>{y});
}

template <class T>
  requires(std::is_integral_v<T>)
sus_pure_const sus_always_inline constexpr T unchecked_div(T x, T y) noexcept {
  return static_cast<T>(MathType<T>{x} / MathType<T>{y});
}

template <class T>
  requires(std::is_integral_v<T>)
sus_pure_const sus_always_inline constexpr T unchecked_rem(T x, T y) noexcept {
  return static_cast<T>(MathType<T>{x} % MathType<T>{y});
}

template <class T>
  requires(std::is_integral_v<T>)
sus_pure_const sus_always_inline constexpr T unchecked_and(T x, T y) noexcept {
  return static_cast<T>(MathType<T>{x} & MathType<T>{y});
}

template <class T>
  requires(std::is_integral_v<T>)
sus_pure_const sus_always_inline constexpr T unchecked_or(T x, T y) noexcept {
  return static_cast<T>(MathType<T>{x} | MathType<T>{y});
}

template <class T>
  requires(std::is_integral_v<T>)
sus_pure_const sus_always_inline constexpr T unchecked_xor(T x, T y) noexcept {
  return static_cast<T>(MathType<T>{x} ^ MathType<T>{y});
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T>)
sus_pure_const sus_always_inline constexpr T unchecked_shl(
    T x, uint32_t y) noexcept {
  return static_cast<T>(MathType<T>{x} << y);
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T>)
sus_pure_const sus_always_inline constexpr T unchecked_shr(
    T x, uint32_t y) noexcept {
  return static_cast<T>(MathType<T>{x} >> y);
}

template <class T>
  requires(std::is_integral_v<T>)
sus_pure_const sus_always_inline constexpr uint32_t num_bits() noexcept {
  return unchecked_mul(unchecked_sizeof<T>(), uint32_t{8});
}

template <class T>
  requires(std::is_integral_v<T> && ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline constexpr T high_bit() noexcept {
  if constexpr (::sus::mem::size_of<T>() == 1)
    return static_cast<T>(0x80);
  else if constexpr (::sus::mem::size_of<T>() == 2)
    return static_cast<T>(0x8000);
  else if constexpr (::sus::mem::size_of<T>() == 4)
    return static_cast<T>(0x80000000);
  else
    return static_cast<T>(0x8000000000000000);
}

template <class T>
  requires(std::is_floating_point_v<T> && ::sus::mem::size_of<T>() == 4)
sus_pure_const sus_always_inline constexpr uint32_t high_bit() noexcept {
  return uint32_t{0x80000000};
}

template <class T>
  requires(std::is_floating_point_v<T> && ::sus::mem::size_of<T>() == 8)
sus_pure_const sus_always_inline constexpr uint64_t high_bit() noexcept {
  return uint64_t{0x8000000000000000};
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T>)
sus_pure_const sus_always_inline constexpr T max_value() noexcept {
  return unchecked_not(T{0});
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline constexpr T max_value() noexcept {
  if constexpr (::sus::mem::size_of<T>() == 1)
    return T{0x7f};
  else if constexpr (::sus::mem::size_of<T>() == 2)
    return T{0x7fff};
  else if constexpr (::sus::mem::size_of<T>() == 4)
    return T{0x7fffffff};
  else
    return T{0x7fffffffffffffff};
}

template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr T max_int64_float() noexcept {
  // Computed with https://godbolt.org/z/1sj8YdMf5.
  if constexpr (::sus::mem::size_of<T>() == 4) {
    // Largest float that is <= i64::MAX.
    return 9223371487098961920.f;
  } else {
    // Largest double that is <= i64::MAX.
    return 9223372036854774784.0;
  }
}
template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr T max_int32_float() noexcept {
  // Computed with https://godbolt.org/z/1sj8YdMf5.
  if constexpr (::sus::mem::size_of<T>() == 4) {
    // Largest float that is <= i32::MAX.
    return 2147483520.f;
  } else {
    // Largest double that is <= i32::MAX.
    return 2147483648.0;
  }
}
template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr T max_int16_float() noexcept {
  if constexpr (::sus::mem::size_of<T>() == 4) {
    // Largest float that is <= i16::MAX.
    return 32767.f;
  } else {
    // Largest double that is <= i16::MAX.
    return 32767.0;
  }
}
template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr T max_int8_float() noexcept {
  if constexpr (::sus::mem::size_of<T>() == 4) {
    // Largest float that is <= i8::MAX.
    return 127.f;
  } else {
    // Largest double that is <= i8::MAX.
    return 127.0;
  }
}

template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr T min_int64_float() noexcept {
  // Computed with https://godbolt.org/z/1sj8YdMf5.
  if constexpr (::sus::mem::size_of<T>() == 4) {
    // Smallest float that is >= i64::MIN.
    return -9223371487098961920.f;
  } else {
    // Smallest double that is >= i64::MIN.
    return -9223372036854774784.0;
  }
}

template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr T min_int32_float() noexcept {
  // Computed with https://godbolt.org/z/1sj8YdMf5.
  if constexpr (::sus::mem::size_of<T>() == 4) {
    // Smallest float that is >= i32::MIN.
    return -2147483520.f;
  } else {
    // Smallest float that is >= i32::MIN.
    return -2147483649.0;
  }
}
template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr T min_int16_float() noexcept {
  if constexpr (::sus::mem::size_of<T>() == 4) {
    // Largest float that is >= i16::MIN.
    return -32768.f;
  } else {
    // Largest double that is >= i16::MIN.
    return -32768.0;
  }
}
template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr T min_int8_float() noexcept {
  if constexpr (::sus::mem::size_of<T>() == 4) {
    // Largest float that is >= i8::MIN.
    return -128.f;
  } else {
    // Largest double that is >= i8::MIN.
    return -128.0;
  }
}

template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr T max_uint64_float() noexcept {
  // Computed with https://godbolt.org/z/bY3vrvos9.
  if constexpr (::sus::mem::size_of<T>() == 4) {
    // Largest float that is <= u64::MAX.
    return 18446742974197923840.f;
  } else {
    // Largest double that is <= u64::MAX.
    return 18446744073709549568.0;
  }
}
template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr T max_uint32_float() noexcept {
  // Computed with https://godbolt.org/z/bY3vrvos9.
  if constexpr (::sus::mem::size_of<T>() == 4) {
    // Largest float that is <= u32::MAX.
    return 4294967040.f;
  } else {
    // Largest double that is <= u32::MAX.
    return 4294967295.0;
  }
}
template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr T max_uint16_float() noexcept {
  if constexpr (::sus::mem::size_of<T>() == 4) {
    // Largest float that is <= u16::MAX.
    return 65535.f;
  } else {
    // Largest double that is <= u16::MAX.
    return 65535.0;
  }
}
template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr T max_uint8_float() noexcept {
  if constexpr (::sus::mem::size_of<T>() == 4) {
    // Largest float that is <= u8::MAX.
    return 255.f;
  } else {
    // Largest double that is <= u8::MAX.
    return 255.0;
  }
}

template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr T min_uint64_float() noexcept {
  // Computed with https://godbolt.org/z/bY3vrvos9.
  if constexpr (::sus::mem::size_of<T>() == 4) {
    // Smallest float that is >= u64::MIN.
    return 0.f;
  } else {
    // Smallest double that is >= u64::MIN.
    return 0.0;
  }
}

template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr T min_uint32_float() noexcept {
  // Computed with https://godbolt.org/z/bY3vrvos9.
  if constexpr (::sus::mem::size_of<T>() == 4) {
    // Smallest float that is >= u32::MIN.
    return 0.f;
  } else {
    // Smallest float that is >= u32::MIN.
    return 0.0;
  }
}
template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr T min_uint16_float() noexcept {
  if constexpr (::sus::mem::size_of<T>() == 4) {
    // Largest float that is >= u16::MIN.
    return 0.f;
  } else {
    // Largest double that is >= u16::MIN.
    return 0.0;
  }
}
template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr T min_uint8_float() noexcept {
  if constexpr (::sus::mem::size_of<T>() == 4) {
    // Largest float that is >= u8::MIN.
    return 0.f;
  } else {
    // Largest double that is >= u8::MIN.
    return 0.0;
  }
}

template <class I, class T>
  requires(std::is_floating_point_v<T> && std::is_integral_v<I> &&
           ::sus::mem::size_of<I>() <= 8)
sus_pure_const sus_always_inline constexpr T max_int_float() noexcept {
  if constexpr (std::is_signed_v<I>) {
    if constexpr (::sus::mem::size_of<I>() == 8)
      return max_int64_float<T>();
    else if constexpr (::sus::mem::size_of<I>() == 4)
      return max_int32_float<T>();
    else if constexpr (::sus::mem::size_of<I>() == 2)
      return max_int16_float<T>();
    else
      return max_int8_float<T>();
  } else {
    if constexpr (::sus::mem::size_of<I>() == 8)
      return max_uint64_float<T>();
    else if constexpr (::sus::mem::size_of<I>() == 4)
      return max_uint32_float<T>();
    else if constexpr (::sus::mem::size_of<I>() == 2)
      return max_uint16_float<T>();
    else
      return max_uint8_float<T>();
  }
}

template <class I, class T>
  requires(std::is_floating_point_v<T> && std::is_integral_v<I> &&
           ::sus::mem::size_of<I>() <= 8)
sus_pure_const sus_always_inline constexpr T min_int_float() noexcept {
  if constexpr (std::is_signed_v<I>) {
    if constexpr (::sus::mem::size_of<I>() == 8)
      return min_int64_float<T>();
    else if constexpr (::sus::mem::size_of<I>() == 4)
      return min_int32_float<T>();
    else if constexpr (::sus::mem::size_of<I>() == 2)
      return min_int16_float<T>();
    else
      return min_int8_float<T>();
  } else {
    if constexpr (::sus::mem::size_of<I>() == 8)
      return min_uint64_float<T>();
    else if constexpr (::sus::mem::size_of<I>() == 4)
      return min_uint32_float<T>();
    else if constexpr (::sus::mem::size_of<I>() == 2)
      return min_uint16_float<T>();
    else
      return min_uint8_float<T>();
  }
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T>)
sus_pure_const sus_always_inline constexpr T min_value() noexcept {
  return T{0};
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline constexpr T min_value() noexcept {
  return -max_value<T>() - T{1};
}

template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr T epsilon() noexcept {
  if constexpr (::sus::mem::size_of<T>() == ::sus::mem::size_of<float>())
    return 1.1920929E-7f;
  else
    return 2.2204460492503131E-16;
}

template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr T max_value() noexcept {
  if constexpr (::sus::mem::size_of<T>() == ::sus::mem::size_of<float>()) {
    return 340282346638528859811704183484516925440.f;
  } else
    return 179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.0;
}

template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr T min_value() noexcept {
  if constexpr (::sus::mem::size_of<T>() == ::sus::mem::size_of<float>())
    return -340282346638528859811704183484516925440.f;
  else
    return -179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.0;
}

template <class T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline constexpr uint32_t count_ones(
    T value) noexcept {
#if _MSC_VER
  if (std::is_constant_evaluated()) {
    using M = MathType<T>;
    auto mvalue = M{value};
    // Algorithm to count the number of bits in parallel, up to a 128 bit value.
    // http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
    mvalue = mvalue - ((mvalue >> 1) & (~M{0} / M{3}));
    mvalue = (mvalue & (~M{0} / M{15} * M{3})) +
             ((mvalue >> 2) & (~M{0} / M{15} * M{3}));
    mvalue = (mvalue + (mvalue >> 4)) & (~M{0} / M{255} * M{15});
    auto count = (mvalue * (~T{0} / T{255})) >>
                 (unchecked_sizeof<T>() - uint32_t{1}) * uint32_t{8};
    return static_cast<uint32_t>(count);
  } else if constexpr (::sus::mem::size_of<T>() <= 2) {
    return uint32_t{__popcnt16(uint16_t{value})};
  } else if constexpr (::sus::mem::size_of<T>() == 8) {
    return static_cast<uint32_t>(__popcnt64(uint64_t{value}));
  } else {
    return uint32_t{__popcnt(uint32_t{value})};
  }
#else
  if constexpr (::sus::mem::size_of<T>() <=
                ::sus::mem::size_of<unsigned int>()) {
    using U = unsigned int;
    return static_cast<uint32_t>(__builtin_popcount(U{value}));
  } else if constexpr (::sus::mem::size_of<T>() <=
                       ::sus::mem::size_of<unsigned long>()) {
    using U = unsigned long;
    return static_cast<uint32_t>(__builtin_popcountl(U{value}));
  } else {
    using U = unsigned long long;
    return static_cast<uint32_t>(__builtin_popcountll(U{value}));
  }
#endif
}

template <class T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline constexpr uint32_t leading_zeros_nonzero(
    ::sus::marker::UnsafeFnMarker, T value) noexcept {
  if (std::is_constant_evaluated()) {
    uint32_t count = 0;
    for (auto i = uint32_t{0};
         i < unchecked_mul(unchecked_sizeof<T>(), uint32_t{8}); ++i) {
      const bool zero = (value & high_bit<T>()) == 0;
      if (!zero) break;
      count += 1;
      value <<= 1;
    }
    return count;
  }

#if _MSC_VER
  if constexpr (::sus::mem::size_of<T>() == 8u) {
#if 1
    unsigned long index;
    _BitScanReverse64(&index, value);
    return static_cast<uint32_t>(63ul ^ index);
#else
    // TODO: Enable this when target CPU is appropriate:
    // - AMD: Advanced Bit Manipulation (ABM)
    // - Intel: Haswell
    // TODO: On Arm ARMv5T architecture and later use `_arm_clz`
    return static_cast<uint32_t>(__lzcnt64(&count, int64_t{value}));
#endif
  } else if constexpr (::sus::mem::size_of<T>() == 4u) {
#if 1
    unsigned long index;
    _BitScanReverse(&index, uint32_t{value});
    return static_cast<uint32_t>(31ul ^ index);
#else
    // TODO: Enable this when target CPU is appropriate:
    // - AMD: Advanced Bit Manipulation (ABM)
    // - Intel: Haswell
    // TODO: On Arm ARMv5T architecture and later use `_arm_clz`
    return __lzcnt(&count, uint32_t{value});
#endif
  } else {
    static_assert(::sus::mem::size_of<T>() <= 2u);
#if 1
    unsigned long index;
    _BitScanReverse(&index, uint32_t{value});
    return static_cast<uint32_t>(
        (31ul ^ index) -
        ((::sus::mem::size_of<unsigned int>() - ::sus::mem::size_of<T>()) *
         8u));
#else
    // TODO: Enable this when target CPU is appropriate:
    // - AMD: Advanced Bit Manipulation (ABM)
    // - Intel: Haswell
    // TODO: On Arm ARMv5T architecture and later use `_arm_clz`
    return static_cast<uint32_t>(
        __lzcnt16(&count, uint16_t{value}) -
        ((::sus::mem::size_of<unsigned int>() - ::sus::mem::size_of<T>()) *
         8u));
#endif
  }
#else
  if constexpr (::sus::mem::size_of<T>() <=
                ::sus::mem::size_of<unsigned int>()) {
    using U = unsigned int;
    return static_cast<uint32_t>(
        __builtin_clz(U{value}) -
        ((::sus::mem::size_of<unsigned int>() - ::sus::mem::size_of<T>()) *
         8u));
  } else if constexpr (::sus::mem::size_of<T>() <=
                       ::sus::mem::size_of<unsigned long>()) {
    using U = unsigned long;
    return static_cast<uint32_t>(__builtin_clzl(U{value}));
  } else {
    using U = unsigned long long;
    return static_cast<uint32_t>(__builtin_clzll(U{value}));
  }
#endif
}

template <class T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline constexpr uint32_t leading_zeros(
    T value) noexcept {
  if (value == 0) return unchecked_mul(unchecked_sizeof<T>(), uint32_t{8});
  return leading_zeros_nonzero(::sus::marker::unsafe_fn, value);
}

/// Counts the number of trailing zeros in a non-zero input.
///
/// # Safety
/// This function produces Undefined Behaviour if passed a zero value.
template <class T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline constexpr uint32_t trailing_zeros_nonzero(
    ::sus::marker::UnsafeFnMarker, T value) noexcept {
  if (std::is_constant_evaluated()) {
    uint32_t count = 0;
    for (auto i = uint32_t{0};
         i < unchecked_mul(unchecked_sizeof<T>(), uint32_t{8}); ++i) {
      const bool zero = (value & 1) == 0;
      if (!zero) break;
      count += 1;
      value >>= 1;
    }
    return count;
  }

#if _MSC_VER
  if constexpr (::sus::mem::size_of<T>() == 8u) {
    unsigned long index;
    _BitScanForward64(&index, value);
    return static_cast<uint32_t>(index);
  } else if constexpr (::sus::mem::size_of<T>() == 4u) {
    unsigned long index;
    _BitScanForward(&index, uint32_t{value});
    return static_cast<uint32_t>(index);
  } else {
    static_assert(::sus::mem::size_of<T>() <= 2u);
    unsigned long index;
    _BitScanForward(&index, uint32_t{value});
    return static_cast<uint32_t>(index);
  }
#else
  if constexpr (::sus::mem::size_of<T>() <=
                ::sus::mem::size_of<unsigned int>()) {
    using U = unsigned int;
    return static_cast<uint32_t>(__builtin_ctz(U{value}));
  } else if constexpr (::sus::mem::size_of<T>() <=
                       ::sus::mem::size_of<unsigned long>()) {
    using U = unsigned long;
    return static_cast<uint32_t>(__builtin_ctzl(U{value}));
  } else {
    using U = unsigned long long;
    return static_cast<uint32_t>(__builtin_ctzll(U{value}));
  }
#endif
}

// TODO: Any way to make it constexpr?
template <class T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline constexpr uint32_t trailing_zeros(
    T value) noexcept {
  if (value == 0) return static_cast<uint32_t>(::sus::mem::size_of<T>() * 8u);
  return trailing_zeros_nonzero(::sus::marker::unsafe_fn, value);
}

template <class T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline constexpr T reverse_bits(T value) noexcept {
#if __clang__
  if constexpr (::sus::mem::size_of<T>() == 1) {
    return __builtin_bitreverse8(value);
  } else if constexpr (::sus::mem::size_of<T>() == 2) {
    return __builtin_bitreverse16(value);
  } else if constexpr (::sus::mem::size_of<T>() == 4) {
    return __builtin_bitreverse32(value);
  } else {
    static_assert(::sus::mem::size_of<T>() == 8);
    return __builtin_bitreverse64(value);
  }
#else
  // Algorithm from Ken Raeburn:
  // http://graphics.stanford.edu/~seander/bithacks.html#ReverseParallel
  uint32_t bits = unchecked_mul(unchecked_sizeof<T>(), uint32_t{8});
  auto mask = unchecked_not(T(0));
  while ((bits >>= 1) > 0) {
    mask ^= unchecked_shl(mask, bits);
    value = (unchecked_shr(value, bits) & mask) |
            (unchecked_shl(value, bits) & ~mask);
  }
  return value;
#endif
}

template <class T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T>)
sus_pure_const inline constexpr T rotate_left(T value, uint32_t n) noexcept {
  const uint32_t num_bits = unchecked_mul(unchecked_sizeof<T>(), uint32_t{8});
  // Try avoid slow % operation if we can. Comparisons are much faster than %.
  if (n >= num_bits) n %= num_bits;
  if (n == 0) return value;
  const auto rshift = unchecked_sub(num_bits, n);
  return unchecked_shl(value, n) | unchecked_shr(value, rshift);
}

template <class T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T>)
sus_pure_const inline constexpr T rotate_right(T value, uint32_t n) noexcept {
  const uint32_t num_bits = unchecked_mul(unchecked_sizeof<T>(), uint32_t{8});
  // Try avoid slow % operation if we can. Comparisons are much faster than %.
  if (n >= num_bits) n %= num_bits;
  if (n == 0) return value;
  const auto lshift = unchecked_sub(num_bits, n);
  return unchecked_shr(value, n) | unchecked_shl(value, lshift);
}

template <class T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline constexpr T swap_bytes(T value) noexcept {
  if (std::is_constant_evaluated()) {
    if constexpr (::sus::mem::size_of<T>() == 1) {
      return value;
    } else if constexpr (::sus::mem::size_of<T>() == 2) {
      unsigned char a = (value >> 0) & 0xff;
      unsigned char b = (value >> 8) & 0xff;
      return (a << 8) | (b << 0);
    } else if constexpr (::sus::mem::size_of<T>() == 4) {
      unsigned char a = (value >> 0) & 0xff;
      unsigned char b = (value >> 8) & 0xff;
      unsigned char c = (value >> 16) & 0xff;
      unsigned char d = (value >> 24) & 0xff;
      return (a << 24) | (b << 16) | (c << 8) | (d << 0);
    } else if constexpr (::sus::mem::size_of<T>() == 8) {
      unsigned char a = (value >> 0) & 0xff;
      unsigned char b = (value >> 8) & 0xff;
      unsigned char c = (value >> 16) & 0xff;
      unsigned char d = (value >> 24) & 0xff;
      unsigned char e = (value >> 32) & 0xff;
      unsigned char f = (value >> 40) & 0xff;
      unsigned char g = (value >> 48) & 0xff;
      unsigned char h = (value >> 56) & 0xff;
      return (a << 24) | (b << 16) | (c << 8) | (d << 0) | (e << 24) |
             (f << 16) | (g << 8) | (h << 0);
    }
  }

#if _MSC_VER
  if constexpr (::sus::mem::size_of<T>() == 1) {
    return value;
  } else if constexpr (::sus::mem::size_of<T>() ==
                       ::sus::mem::size_of<unsigned short>()) {
    using U = unsigned short;
    return _byteswap_ushort(U{value});
  } else if constexpr (::sus::mem::size_of<T>() ==
                       ::sus::mem::size_of<unsigned long>()) {
    using U = unsigned long;
    return _byteswap_ulong(U{value});
  } else {
    static_assert(::sus::mem::size_of<T>() == 8);
    return _byteswap_uint64(value);
  }
#else
  if constexpr (::sus::mem::size_of<T>() == 1) {
    return value;
  } else if constexpr (::sus::mem::size_of<T>() == 2) {
    return __builtin_bswap16(uint16_t{value});
  } else if constexpr (::sus::mem::size_of<T>() == 4) {
    return __builtin_bswap32(value);
  } else {
    static_assert(::sus::mem::size_of<T>() == 8);
    return __builtin_bswap64(value);
  }
#endif
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline constexpr auto into_unsigned(T x) noexcept {
  if constexpr (::sus::mem::size_of<T>() == 1)
    return static_cast<uint8_t>(x);
  else if constexpr (::sus::mem::size_of<T>() == 2)
    return static_cast<uint16_t>(x);
  else if constexpr (::sus::mem::size_of<T>() == 4)
    return static_cast<uint32_t>(x);
  else
    return static_cast<uint64_t>(x);
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 4)
sus_pure_const sus_always_inline constexpr auto into_widened(T x) noexcept {
  if constexpr (::sus::mem::size_of<T>() == 1)
    return static_cast<uint16_t>(x);
  else if constexpr (::sus::mem::size_of<T>() == 2)
    return static_cast<uint32_t>(x);
  else
    return static_cast<uint64_t>(x);
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 4)
sus_pure_const sus_always_inline constexpr auto into_widened(T x) noexcept {
  if constexpr (::sus::mem::size_of<T>() == 1)
    return static_cast<int16_t>(x);
  else if constexpr (::sus::mem::size_of<T>() == 2)
    return static_cast<int32_t>(x);
  else
    return static_cast<int64_t>(x);
}

template <class T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline constexpr auto into_signed(T x) noexcept {
  if constexpr (::sus::mem::size_of<T>() == 1)
    return static_cast<int8_t>(x);
  else if constexpr (::sus::mem::size_of<T>() == 2)
    return static_cast<int16_t>(x);
  else if constexpr (::sus::mem::size_of<T>() == 4)
    return static_cast<int32_t>(x);
  else
    return static_cast<int64_t>(x);
}

template <class T>
  requires(::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline constexpr bool sign_bit(T x) noexcept {
  if constexpr (::sus::mem::size_of<T>() == 1)
    return (x & (T(1) << 7)) != 0;
  else if constexpr (::sus::mem::size_of<T>() == 2)
    return (x & (T(1) << 15)) != 0;
  else if constexpr (::sus::mem::size_of<T>() == 4)
    return (x & (T(1) << 31)) != 0;
  else
    return (x & (T(1) << 63)) != 0;
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const inline constexpr OverflowOut<T> add_with_overflow(T x,
                                                                 T y) noexcept {
  return OverflowOut sus_clang_bug_56394(<T>){
      .overflow = x > max_value<T>() - y,
      .value = unchecked_add(x, y),
  };
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const inline constexpr OverflowOut<T> add_with_overflow(T x,
                                                                 T y) noexcept {
  const auto out =
      into_signed(unchecked_add(into_unsigned(x), into_unsigned(y)));
  return OverflowOut sus_clang_bug_56394(<T>){
      .overflow = y >= 0 != out >= x,
      .value = out,
  };
}

template <class T, class U = decltype(into_signed(std::declval<T>()))>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8 &&
           ::sus::mem::size_of<T>() == ::sus::mem::size_of<U>())
sus_pure_const inline constexpr OverflowOut<T> add_with_overflow_signed(
    T x, U y) noexcept {
  return OverflowOut sus_clang_bug_56394(<T>){
      .overflow = (y >= 0 && into_unsigned(y) > max_value<T>() - x) ||
                  (y < 0 && into_unsigned(-y) > x),
      .value = unchecked_add(x, into_unsigned(y)),
  };
}

template <class T, class U = decltype(into_unsigned(std::declval<T>()))>
  requires(std::is_integral_v<T> && std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8 &&
           ::sus::mem::size_of<T>() == ::sus::mem::size_of<U>())
sus_pure_const inline constexpr OverflowOut<T> add_with_overflow_unsigned(
    T x, U y) noexcept {
  const auto out = into_signed(unchecked_add(into_unsigned(x), y));
  return OverflowOut sus_clang_bug_56394(<T>){
      .overflow = static_cast<U>(max_value<T>()) - static_cast<U>(x) < y,
      .value = out,
  };
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const inline constexpr OverflowOut<T> sub_with_overflow(T x,
                                                                 T y) noexcept {
  return OverflowOut sus_clang_bug_56394(<T>){
      .overflow = x < unchecked_add(min_value<T>(), y),
      .value = unchecked_sub(x, y),
  };
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const inline constexpr OverflowOut<T> sub_with_overflow(T x,
                                                                 T y) noexcept {
  const auto out =
      into_signed(unchecked_sub(into_unsigned(x), into_unsigned(y)));
  return OverflowOut sus_clang_bug_56394(<T>){
      .overflow = y >= 0 != out <= x,
      .value = out,
  };
}

template <class T, class U = decltype(into_unsigned(std::declval<T>()))>
  requires(std::is_integral_v<T> && std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8 &&
           ::sus::mem::size_of<T>() == ::sus::mem::size_of<U>())
sus_pure_const inline constexpr OverflowOut<T> sub_with_overflow_unsigned(
    T x, U y) noexcept {
  const auto out = into_signed(unchecked_sub(into_unsigned(x), y));
  return OverflowOut sus_clang_bug_56394(<T>){
      .overflow = static_cast<U>(x) - static_cast<U>(min_value<T>()) < y,
      .value = out,
  };
}

/// SAFETY: Requires that `x > y` so the result will be positive.
template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T>)
sus_pure_const sus_always_inline constexpr std::make_unsigned_t<T>
sub_with_unsigned_positive_result(T x, T y) noexcept {
  return unchecked_sub(into_unsigned(x), into_unsigned(y));
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 4)
sus_pure_const inline constexpr OverflowOut<T> mul_with_overflow(T x,
                                                                 T y) noexcept {
  // TODO: Can we use compiler intrinsics?
  auto out = unchecked_mul(into_widened(x), into_widened(y));
  using Wide = decltype(out);
  return OverflowOut sus_clang_bug_56394(<T>){
      .overflow = out > Wide{max_value<T>()}, .value = static_cast<T>(out)};
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() == 8)
sus_pure_const inline constexpr OverflowOut<T> mul_with_overflow(T x,
                                                                 T y) noexcept {
#if _MSC_VER
  if (std::is_constant_evaluated()) {
    const bool overflow =
        x > T{1} && y > T{1} && x > unchecked_div(max_value<T>(), y);
    return OverflowOut sus_clang_bug_56394(<T>){.overflow = overflow,
                                                .value = unchecked_mul(x, y)};
  } else {
    // For MSVC, use _umul128, but what about constexpr?? If we can't do
    // it then make the whole function non-constexpr?
    uint64_t highbits;
    auto out = static_cast<T>(_umul128(x, y, &highbits));
    return OverflowOut sus_clang_bug_56394(<T>){.overflow = highbits != 0,
                                                .value = out};
  }
#else
  auto out = __uint128_t{x} * __uint128_t{y};
  return OverflowOut sus_clang_bug_56394(<T>){
      .overflow = out > __uint128_t{max_value<T>()},
      .value = static_cast<T>(out)};
#endif
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 4)
sus_pure_const inline constexpr OverflowOut<T> mul_with_overflow(T x,
                                                                 T y) noexcept {
  // TODO: Can we use compiler intrinsics?
  auto out = into_widened(x) * into_widened(y);
  using Wide = decltype(out);
  return OverflowOut sus_clang_bug_56394(<T>){
      .overflow = out > Wide{max_value<T>()} || out < Wide{min_value<T>()},
      .value = static_cast<T>(out)};
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() == 8)
sus_pure_const inline constexpr OverflowOut<T> mul_with_overflow(T x,
                                                                 T y) noexcept {
#if _MSC_VER
  if (x == T{0} || y == T{0})
    return OverflowOut sus_clang_bug_56394(<T>){.overflow = false,
                                                .value = T{0}};

  using U = decltype(into_unsigned(x));
  const auto absx =
      x >= T{0}
          ? into_unsigned(x)
          : unchecked_add(into_unsigned(unchecked_neg(unchecked_add(x, T{1}))),
                          U{1});
  const auto absy =
      y >= T{0}
          ? into_unsigned(y)
          : unchecked_add(into_unsigned(unchecked_neg(unchecked_add(y, T{1}))),
                          U{1});
  const bool mul_negative = (x ^ y) < 0;
  const auto mul_max =
      unchecked_add(into_unsigned(max_value<T>()), U{mul_negative});
  const bool overflow = absx > unchecked_div(mul_max, absy);
  const auto mul_val = unchecked_mul(into_unsigned(x), into_unsigned(y));
  return OverflowOut sus_clang_bug_56394(<T>){.overflow = overflow,
                                              .value = into_signed(mul_val)};
#else
  auto out = __int128_t{x} * __int128_t{y};
  return OverflowOut sus_clang_bug_56394(<T>){
      .overflow =
          out > __int128_t{max_value<T>()} || out < __int128_t{min_value<T>()},
      .value = static_cast<T>(out)};
#endif
}

template <class T>
  requires(std::is_integral_v<T> && ::sus::mem::size_of<T>() <= 8)
sus_pure_const inline constexpr OverflowOut<T> pow_with_overflow(
    T base, uint32_t exp) noexcept {
  if (exp == 0)
    return OverflowOut sus_clang_bug_56394(<T>){.overflow = false,
                                                .value = T{1}};
  auto acc = T{1};
  bool overflow = false;
  while (exp > 1) {
    if (exp & 1) {
      auto r = mul_with_overflow(acc, base);
      overflow |= r.overflow;
      acc = r.value;
    }
    exp /= 2;
    auto r = mul_with_overflow(base, base);
    overflow |= r.overflow;
    base = r.value;
  }
  auto r = mul_with_overflow(acc, base);
  return OverflowOut sus_clang_bug_56394(<T>){
      .overflow = overflow || r.overflow, .value = r.value};
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> &&
           (::sus::mem::size_of<T>() == 1 || ::sus::mem::size_of<T>() == 2 ||
            ::sus::mem::size_of<T>() == 4 || ::sus::mem::size_of<T>() == 8))
sus_pure_const inline constexpr OverflowOut<T> shl_with_overflow(
    T x, uint32_t shift) noexcept {
  // Using `num_bits<T>() - 1` as a mask only works if num_bits<T>() is a power
  // of two, so we verify that ::sus::mem::size_of<T>() is a power of 2, which
  // implies the number of bits is as well (since each byte is 2^3 bits).
  const bool overflow = shift >= num_bits<T>();
  if (overflow) [[unlikely]]
    shift = shift & (unchecked_sub(num_bits<T>(), uint32_t{1}));
  return OverflowOut sus_clang_bug_56394(<T>){.overflow = overflow,
                                              .value = unchecked_shl(x, shift)};
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> &&
           (::sus::mem::size_of<T>() == 1 || ::sus::mem::size_of<T>() == 2 ||
            ::sus::mem::size_of<T>() == 4 || ::sus::mem::size_of<T>() == 8))
sus_pure_const inline constexpr OverflowOut<T> shl_with_overflow(
    T x, uint32_t shift) noexcept {
  // Using `num_bits<T>() - 1` as a mask only works if num_bits<T>() is a power
  // of two, so we verify that ::sus::mem::size_of<T>() is a power of 2, which
  // implies the number of bits is as well (since each byte is 2^3 bits).
  const bool overflow = shift >= num_bits<T>();
  if (overflow) [[unlikely]]
    shift = shift & (unchecked_sub(num_bits<T>(), uint32_t{1}));
  return OverflowOut sus_clang_bug_56394(<T>){
      .overflow = overflow,
      .value = into_signed(unchecked_shl(into_unsigned(x), shift))};
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> &&
           (::sus::mem::size_of<T>() == 1 || ::sus::mem::size_of<T>() == 2 ||
            ::sus::mem::size_of<T>() == 4 || ::sus::mem::size_of<T>() == 8))
sus_pure_const inline constexpr OverflowOut<T> shr_with_overflow(
    T x, uint32_t shift) noexcept {
  // Using `num_bits<T>() - 1` as a mask only works if num_bits<T>() is a power
  // of two, so we verify that ::sus::mem::size_of<T>() is a power of 2, which
  // implies the number of bits is as well (since each byte is 2^3 bits).
  const bool overflow = shift >= num_bits<T>();
  if (overflow) [[unlikely]]
    shift = shift & (unchecked_sub(num_bits<T>(), uint32_t{1}));
  return OverflowOut sus_clang_bug_56394(<T>){.overflow = overflow,
                                              .value = unchecked_shr(x, shift)};
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> &&
           (::sus::mem::size_of<T>() == 1 || ::sus::mem::size_of<T>() == 2 ||
            ::sus::mem::size_of<T>() == 4 || ::sus::mem::size_of<T>() == 8))
sus_pure_const inline constexpr OverflowOut<T> shr_with_overflow(
    T x, uint32_t shift) noexcept {
  // Using `num_bits<T>() - 1` as a mask only works if num_bits<T>() is a power
  // of two, so we verify that ::sus::mem::size_of<T>() is a power of 2, which
  // implies the number of bits is as well (since each byte is 2^3 bits).
  const bool overflow = shift >= num_bits<T>();
  if (overflow) [[unlikely]]
    shift = shift & (unchecked_sub(num_bits<T>(), uint32_t{1}));
  return OverflowOut sus_clang_bug_56394(<T>){
      .overflow = overflow,
      .value = into_signed(unchecked_shr(into_unsigned(x), shift))};
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const inline constexpr T saturating_add(T x, T y) noexcept {
  // TODO: Optimize this? Use intrinsics?
  const auto out = add_with_overflow(x, y);
  if (!out.overflow) [[likely]]
    return out.value;
  else
    return max_value<T>();
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const inline constexpr T saturating_add(T x, T y) noexcept {
  // TODO: Optimize this? Use intrinsics?
  if (y >= 0) {
    if (x <= max_value<T>() - y) [[likely]]
      return x + y;
    else
      return max_value<T>();
  } else {
    if (x >= min_value<T>() - y) [[likely]]
      return x + y;
    else
      return min_value<T>();
  }
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const inline constexpr T saturating_sub(T x, T y) noexcept {
  // TODO: Optimize this? Use intrinsics?
  const auto out = sub_with_overflow(x, y);
  if (!out.overflow) [[likely]]
    return out.value;
  else
    return min_value<T>();
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const inline constexpr T saturating_sub(T x, T y) noexcept {
  // TODO: Optimize this? Use intrinsics?
  if (y <= 0) {
    if (x <= max_value<T>() + y) [[likely]]
      return x - y;
    else
      return max_value<T>();
  } else {
    if (x >= min_value<T>() + y) [[likely]]
      return x - y;
    else
      return min_value<T>();
  }
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const inline constexpr T saturating_mul(T x, T y) noexcept {
  // TODO: Optimize this? Use intrinsics?
  const auto out = mul_with_overflow(x, y);
  if (!out.overflow) [[likely]]
    return out.value;
  else
    return max_value<T>();
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const inline constexpr T saturating_mul(T x, T y) noexcept {
  // TODO: Optimize this? Use intrinsics?
  const auto out = mul_with_overflow(x, y);
  if (!out.overflow) [[likely]]
    return out.value;
  else if (x > 0 == y > 0)
    return max_value<T>();
  else
    return min_value<T>();
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline constexpr T wrapping_add(T x, T y) noexcept {
  return x + y;
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline constexpr T wrapping_add(T x, T y) noexcept {
  // TODO: Are there cheaper intrinsics?
  return add_with_overflow(x, y).value;
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline constexpr T wrapping_sub(T x, T y) noexcept {
  return unchecked_sub(x, y);
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline constexpr T wrapping_sub(T x, T y) noexcept {
  // TODO: Are there cheaper intrinsics?
  return sub_with_overflow(x, y).value;
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline constexpr T wrapping_mul(T x, T y) noexcept {
  return x * y;
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline constexpr T wrapping_mul(T x, T y) noexcept {
  // TODO: Are there cheaper intrinsics?
  return mul_with_overflow(x, y).value;
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline constexpr T wrapping_pow(
    T base, uint32_t exp) noexcept {
  // TODO: Don't need to track overflow and unsigned wraps by default, so this
  // can be cheaper.
  return pow_with_overflow(base, exp).value;
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline constexpr T wrapping_pow(
    T base, uint32_t exp) noexcept {
  // TODO: Are there cheaper intrinsics?
  return pow_with_overflow(base, exp).value;
}

// Returns one less than next power of two.
// (For 8u8 next power of two is 8u8 and for 6u8 it is 8u8)
//
// 8u8.one_less_than_next_power_of_two() == 7
// 6u8.one_less_than_next_power_of_two() == 7
//
// This method cannot overflow, as in the `next_power_of_two`
// overflow cases it instead ends up returning the maximum value
// of the type, and can return 0 for 0.
template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const inline constexpr T one_less_than_next_power_of_two(
    T x) noexcept {
  if (x <= 1u) {
    return 0u;
  } else {
    const auto p = unchecked_sub(x, T{1});
    // SAFETY: Because `p > 0`, it cannot consist entirely of leading zeros.
    // That means the shift is always in-bounds, and some processors (such as
    // intel pre-haswell) have more efficient ctlz intrinsics when the argument
    // is non-zero.
    const auto z = leading_zeros_nonzero(::sus::marker::unsafe_fn, p);
    return unchecked_shr(max_value<T>(), z);
  }
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const inline constexpr bool div_overflows(T x, T y) noexcept {
  // Using `&` helps LLVM see that it is the same check made in division.
  return y == T{0} || ((x == min_value<T>()) & (y == T{-1}));
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const inline constexpr bool div_overflows_nonzero(
    ::sus::marker::UnsafeFnMarker, T x, T y) noexcept {
  // Using `&` helps LLVM see that it is the same check made in division.
  return ((x == min_value<T>()) & (y == T{-1}));
}

// SAFETY: Requires that !div_overflows(x, y) or Undefined Behaviour results.
template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const inline constexpr T div_euclid(::sus::marker::UnsafeFnMarker, T x,
                                             T y) noexcept {
  const auto q = unchecked_div(x, y);
  if (x % y >= 0)
    return q;
  else if (y > 0)
    return unchecked_sub(q, T{1});
  else
    return unchecked_add(q, T{1});
}

// SAFETY: Requires that !div_overflows(x, y) or Undefined Behaviour results.
template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> &&
           ::sus::mem::size_of<T>() <= 8)
sus_pure_const inline constexpr T rem_euclid(::sus::marker::UnsafeFnMarker, T x,
                                             T y) noexcept {
  const auto r = unchecked_rem(x, y);
  if (r < 0) {
    if (y < 0)
      return unchecked_sub(r, y);
    else
      return unchecked_add(r, y);
  } else {
    return r;
  }
}

template <class T>
  requires(std::is_floating_point_v<T> && ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline constexpr auto into_unsigned_integer(
    T x) noexcept {
  if constexpr (::sus::mem::size_of<T>() == ::sus::mem::size_of<float>())
    return std::bit_cast<uint32_t>(x);
  else
    return std::bit_cast<uint64_t>(x);
}

// Prefer the non-constexpr `into_float()` to avoid problems where you get a
// different value in a constexpr context from a runtime context. It's safe to
// call this function if the argument is not a NaN. Otherwise, you must ensure
// the NaN is exactly in the form that would be produced in a constexpr context
// in order to avoid problems.
template <class T>
  requires(std::is_integral_v<T> && ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline constexpr auto into_float_constexpr(
    ::sus::marker::UnsafeFnMarker, T x) noexcept {
  if constexpr (::sus::mem::size_of<T>() == ::sus::mem::size_of<float>())
    return std::bit_cast<float>(x);
  else
    return std::bit_cast<double>(x);
}

// This is NOT constexpr because it can produce different results in a constexpr
// context than in a runtime one. For example.
//
// ```
// constexpr float x = into_float(uint32_t{0x7f800001});
// const float y = into_float(uint32_t{0x7f800001});
// ```
// In this case `x` is `7fc00001` (the quiet bit became set), but `y` is
// `0x7f800001`.
template <class T>
  requires(std::is_integral_v<T> && ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline auto into_float(T x) noexcept {
  // SAFETY: Since this isn't a constexpr context, we're okay.
  return into_float_constexpr(::sus::marker::unsafe_fn, x);
}

template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr T min_positive_value() noexcept {
  if constexpr (::sus::mem::size_of<T>() == ::sus::mem::size_of<float>())
    return 1.17549435E-38f;
  else
    return 0.22250738585072014E-307;
}

template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr int32_t min_exp() noexcept {
  if constexpr (::sus::mem::size_of<T>() == ::sus::mem::size_of<float>())
    return int32_t{-125};
  else
    return int32_t{-1021};
}

template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr int32_t max_exp() noexcept {
  if constexpr (::sus::mem::size_of<T>() == ::sus::mem::size_of<float>())
    return int32_t{128};
  else
    return int32_t{1024};
}

template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr int32_t min_10_exp() noexcept {
  if constexpr (::sus::mem::size_of<T>() == ::sus::mem::size_of<float>())
    return int32_t{-37};
  else
    return int32_t{-307};
}

template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr int32_t max_10_exp() noexcept {
  if constexpr (::sus::mem::size_of<T>() == ::sus::mem::size_of<float>())
    return int32_t{38};
  else
    return int32_t{308};
}

template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr uint32_t radix() noexcept {
  return 2;
}

template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr uint32_t
num_mantissa_digits() noexcept {
  if constexpr (::sus::mem::size_of<T>() == ::sus::mem::size_of<float>())
    return uint32_t{24};
  else
    return uint32_t{53};
}

template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr uint32_t num_digits() noexcept {
  if constexpr (::sus::mem::size_of<T>() == ::sus::mem::size_of<float>())
    return 6;
  else
    return 15;
}

template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr T nan() noexcept {
  // SAFETY: We must take care that the value returned here is the same in both
  // a constexpr and non-constexpr context. The quiet bit is always set in a
  // constexpr context, so we return a quiet bit here.
  if constexpr (::sus::mem::size_of<T>() == ::sus::mem::size_of<float>())
    return into_float_constexpr(::sus::marker::unsafe_fn, uint32_t{0x7fc00000});
  else
    return into_float_constexpr(::sus::marker::unsafe_fn,
                                uint64_t{0x7ff8000000000000});
}

template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr T infinity() noexcept {
  // SAFETY: The value being constructed is not a NaN so we can do this in a
  // constexpr way.
  if constexpr (::sus::mem::size_of<T>() == ::sus::mem::size_of<float>())
    return into_float_constexpr(::sus::marker::unsafe_fn, uint32_t{0x7f800000});
  else
    return into_float_constexpr(::sus::marker::unsafe_fn,
                                uint64_t{0x7ff0000000000000});
}

template <class T>
  requires(std::is_floating_point_v<T>)
sus_pure_const sus_always_inline constexpr T negative_infinity() noexcept {
  // SAFETY: The value being constructed is non a NaN so we can do this in a
  // constexpr way.
  if constexpr (::sus::mem::size_of<T>() == ::sus::mem::size_of<float>())
    return into_float_constexpr(::sus::marker::unsafe_fn, uint32_t{0xff800000});
  else
    return into_float_constexpr(::sus::marker::unsafe_fn,
                                uint64_t{0xfff0000000000000});
}

sus_pure_const sus_always_inline constexpr int32_t exponent_bits(
    float x) noexcept {
  constexpr uint32_t mask = 0b01111111100000000000000000000000;
  return static_cast<int32_t>(
      unchecked_shr(into_unsigned_integer(x) & mask, 23));
}

sus_pure_const sus_always_inline constexpr int32_t exponent_bits(
    double x) noexcept {
  constexpr uint64_t mask =
      0b0111111111110000000000000000000000000000000000000000000000000000;
  return static_cast<int32_t>(
      unchecked_shr(into_unsigned_integer(x) & mask, 52));
}

/// This function requires that `x` is a normal value to produce a value result.
sus_pure_const sus_always_inline constexpr int32_t float_normal_exponent_value(
    float x) noexcept {
  return exponent_bits(x) - int32_t{127};
}

/// This function requires that `x` is a normal value to produce a value result.
sus_pure_const sus_always_inline constexpr int32_t float_normal_exponent_value(
    double x) noexcept {
  return exponent_bits(x) - int32_t{1023};
}

constexpr sus_always_inline uint32_t mantissa(float x) noexcept {
  constexpr uint32_t mask = 0b00000000011111111111111111111111;
  return into_unsigned_integer(x) & mask;
}

sus_pure_const sus_always_inline constexpr uint64_t mantissa(
    double x) noexcept {
  constexpr uint64_t mask =
      0b0000000000001111111111111111111111111111111111111111111111111111;
  return into_unsigned_integer(x) & mask;
}

template <class T>
  requires(std::is_floating_point_v<T> && ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline constexpr bool float_is_zero(T x) noexcept {
  return (into_unsigned_integer(x) & ~high_bit<T>()) == 0;
}

sus_pure_const inline constexpr bool float_is_inf(float x) noexcept {
#if __has_builtin(__builtin_isinf)
  return __builtin_isinf(x);
#else
  constexpr auto inf = uint32_t{0x7f800000};
  constexpr auto mask = uint32_t{0x7fffffff};
  const auto y = into_unsigned_integer(x);
  return (y & mask) == inf;
#endif
}

sus_pure_const inline constexpr bool float_is_inf(double x) noexcept {
#if __has_builtin(__builtin_isinf)
  return __builtin_isinf(x);
#else
  constexpr auto inf = uint64_t{0x7ff0000000000000};
  constexpr auto mask = uint64_t{0x7fffffffffffffff};
  return (into_unsigned_integer(x) & mask) == inf;
#endif
}

sus_pure_const inline constexpr bool float_is_inf_or_nan(float x) noexcept {
  constexpr auto mask = uint32_t{0x7f800000};
  return (into_unsigned_integer(x) & mask) == mask;
}

sus_pure_const inline constexpr bool float_is_inf_or_nan(double x) noexcept {
  constexpr auto mask = uint64_t{0x7ff0000000000000};
  return (into_unsigned_integer(x) & mask) == mask;
}

sus_pure_const inline constexpr bool float_is_nan(float x) noexcept {
#if __has_builtin(__builtin_isnan)
  return __builtin_isnan(x);
#else
  constexpr auto inf_mask = uint32_t{0x7f800000};
  constexpr auto nan_mask = uint32_t{0x7fffffff};
  return (into_unsigned_integer(x) & nan_mask) > inf_mask;
#endif
}

sus_pure_const inline constexpr bool float_is_nan(double x) noexcept {
#if __has_builtin(__builtin_isnan)
  return __builtin_isnan(x);
#else
  constexpr auto inf_mask = uint64_t{0x7ff0000000000000};
  constexpr auto nan_mask = uint64_t{0x7fffffffffffffff};
  return (into_unsigned_integer(x) & nan_mask) > inf_mask;
#endif
}

// Assumes that x is a NaN.
sus_pure_const inline constexpr bool float_is_nan_quiet(float x) noexcept {
  // The quiet bit is the highest bit in the mantissa.
  constexpr auto quiet_mask = uint32_t{uint32_t{1} << (23 - 1)};
  return (into_unsigned_integer(x) & quiet_mask) != 0;
}

// Assumes that x is a NaN.
sus_pure_const inline constexpr bool float_is_nan_quiet(double x) noexcept {
  // The quiet bit is the highest bit in the mantissa.
  constexpr auto quiet_mask = uint64_t{uint64_t{1} << (52 - 1)};
  return (into_unsigned_integer(x) & quiet_mask) != 0;
}

// This is only valid if the argument is not (positive or negative) zero.
template <class T>
  requires(std::is_floating_point_v<T> && ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline constexpr bool float_nonzero_is_subnormal(
    T x) noexcept {
  return exponent_bits(x) == int32_t{0};
}

sus_pure_const inline constexpr bool float_is_normal(float x) noexcept {
  // If the exponent is 0, the number is zero or subnormal. If the exponent is
  // all ones, the number is infinite or NaN.
  const auto e = exponent_bits(x);
  return e != 0 && e != int32_t{0b011111111};
}

sus_pure_const inline constexpr bool float_is_normal(double x) noexcept {
  // If the exponent is 0, the number is zero or subnormal. If the exponent is
  // all ones, the number is infinite or NaN.
  const auto e = exponent_bits(x);
  return e != 0 && e != int32_t{0b011111111111};
}

template <class T>
  requires(std::is_floating_point_v<T> && ::sus::mem::size_of<T>() <= 8)
sus_pure_const inline constexpr T truncate_float(T x) noexcept {
  if (x <= max_int64_float<T>() && x >= min_int64_float<T>())
    return static_cast<T>(static_cast<int64_t>(x));

  constexpr auto mantissa_width =
      ::sus::mem::size_of<T>() == ::sus::mem::size_of<float>() ? uint32_t{23}
                                                               : uint32_t{52};

  if (float_is_inf_or_nan(x) || float_is_zero(x)) return x;
  if (float_nonzero_is_subnormal(x)) [[unlikely]]
    return T{0};

  const int32_t exponent = float_normal_exponent_value(x);

  // If the exponent is greater than the most negative mantissa
  // exponent, then x is already an integer.
  if (exponent >= static_cast<int32_t>(mantissa_width)) return x;

  // If the exponent is such that abs(x) is less than 1, then return 0.
  if (exponent <= -1) {
    if ((into_unsigned_integer(x) & high_bit<T>()) != 0)
      return T{-0.0};
    else
      return T{0.0};
  }

  const uint32_t trim_bits = mantissa_width - static_cast<uint32_t>(exponent);
  const auto shr = unchecked_shr(into_unsigned_integer(x), trim_bits);
  const auto shl = unchecked_shl(shr, trim_bits);
  // SAFETY: The value here is not a NaN, so will give the same value in
  // constexpr and non-constexpr contexts.
  return into_float_constexpr(::sus::marker::unsafe_fn, shl);
}

template <class T>
  requires(std::is_floating_point_v<T> && ::sus::mem::size_of<T>() <= 8)
sus_pure_const inline constexpr bool float_signbit(T x) noexcept {
  return unchecked_and(into_unsigned_integer(x), high_bit<T>()) != 0;
}

template <class T>
  requires(std::is_floating_point_v<T> && ::sus::mem::size_of<T>() <= 8)
sus_pure_const inline T float_signum(T x) noexcept {
  // TODO: Can this be done without a branch? Beware nan values in constexpr
  // context are rewritten so this function is not constexpr.
  if (float_is_nan(x)) [[unlikely]]
    return x;
  const auto signbit = unchecked_and(into_unsigned_integer(x), high_bit<T>());
  // SAFETY: The value passed in is constructed here and is not a NaN.
  return into_float_constexpr(
      ::sus::marker::unsafe_fn,
      unchecked_add(into_unsigned_integer(T{1}), signbit));
}

template <class T>
  requires(std::is_floating_point_v<T> && ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline T float_round(T x) noexcept {
  // MSVC round(float) is returning a double for some reason.
  const auto out = into_unsigned_integer(static_cast<T>(std::round(x)));
  // `round()` doesn't preserve the sign bit for -0, so we need to restore it,
  // for (-0.5, -0.0].
  return into_float((out & ~high_bit<T>()) |
                    (into_unsigned_integer(x) & high_bit<T>()));
}

template <class T>
  requires(std::is_floating_point_v<T> && ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline T float_round_ties_by_mode(T x) noexcept {
  return std::nearbyint(x);
}

template <class I, class T>
  requires(std::is_floating_point_v<T> && ::sus::mem::size_of<T>() <= 8)
sus_pure_const sus_always_inline I float_round_to(T x) noexcept {
  static_assert(::sus::mem::size_of<I>() <= ::sus::mem::size_of<long long>());

  if (float_is_nan(x)) [[unlikely]]
    return I{0};
  if constexpr (::sus::mem::size_of<I>() == ::sus::mem::size_of<long long>()) {
    if (x > max_int_float<I, T>()) [[unlikely]]
      return max_value<I>();
    if (x < min_int_float<I, T>()) [[unlikely]]
      return min_value<I>();
    return std::llrint(x);
  } else {
    if (x > max_int_float<I, T>()) [[unlikely]]
      return max_value<I>();
    if (x < min_int_float<I, T>()) [[unlikely]]
      return min_value<I>();
    return std::lrint(x);
  }
}

#if __has_builtin(__builtin_fpclassify)
template <class T>
  requires(std::is_floating_point_v<T> && ::sus::mem::size_of<T>() <= 8)
constexpr inline ::sus::num::FpCategory float_category(T x) noexcept {
  constexpr auto nan = 1;
  constexpr auto inf = 2;
  constexpr auto norm = 3;
  constexpr auto subnorm = 4;
  constexpr auto zero = 5;
  switch (__builtin_fpclassify(nan, inf, norm, subnorm, zero, x)) {
    case nan: return ::sus::num::FpCategory::Nan;
    case inf: return ::sus::num::FpCategory::Infinite;
    case norm: return ::sus::num::FpCategory::Normal;
    case subnorm: return ::sus::num::FpCategory::Subnormal;
    case zero: return ::sus::num::FpCategory::Zero;
    default: ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
  }
}
#else
template <class T>
  requires(std::is_floating_point_v<T> && ::sus::mem::size_of<T>() <= 8)
sus_pure_const constexpr inline ::sus::num::FpCategory float_category(
    T x) noexcept {
  if (std::is_constant_evaluated()) {
    if (float_is_nan(x)) return ::sus::num::FpCategory::Nan;
    if (float_is_inf_or_nan(x)) return ::sus::num::FpCategory::Infinite;
    if (float_is_zero(x)) return ::sus::num::FpCategory::Zero;
    if (float_nonzero_is_subnormal(x)) return ::sus::num::FpCategory::Subnormal;
    return ::sus::num::FpCategory::Normal;
  } else {
    // C++23 requires a constexpr way to do this.
    switch (::fpclassify(x)) {
      case FP_NAN: return ::sus::num::FpCategory::Nan;
      case FP_INFINITE: return ::sus::num::FpCategory::Infinite;
      case FP_NORMAL: return ::sus::num::FpCategory::Normal;
      case FP_SUBNORMAL: return ::sus::num::FpCategory::Subnormal;
      case FP_ZERO: return ::sus::num::FpCategory::Zero;
      default: ::sus::unreachable_unchecked(::sus::marker::unsafe_fn);
    }
  }
}
#endif

// Requires that `min <= max` and that `min` and `max` are not NaN or else this
// function produces Undefined Behaviour.
template <class T>
  requires(std::is_floating_point_v<T> && ::sus::mem::size_of<T>() <= 8)
sus_pure_const constexpr inline T float_clamp(T x, T min, T max) noexcept {
  if (float_is_nan(x)) [[unlikely]]
    return nan<T>();
  else if (x < min)
    return min;
  else if (x > max)
    return max;
  return x;
}

// TODO: constexpr in C++23.
template <class T>
  requires(std::is_floating_point_v<T> && ::sus::mem::size_of<T>() <= 8)
sus_pure_const inline T next_toward(T from, T to) {
  if constexpr (::sus::mem::size_of<T>() == 4u)
    return std::nexttowardf(from, to);
  else
    return std::nexttoward(from, to);
}

#pragma warning(push)
// MSVC claims that "overflow in constant arithmetic" occurs on the static_cast
// to float/double, but we check for overflow first, the conversion is in range.
#pragma warning(disable : 4756)

template <class Out, class T>
  requires(std::is_integral_v<T> && std::is_floating_point_v<Out> &&
           (::sus::mem::size_of<T>() <= 8) &&
           (::sus::mem::size_of<Out>() == 4 || ::sus::mem::size_of<Out>() == 8))
sus_pure_const constexpr inline Out static_cast_int_to_float(T x) noexcept {
  // C++20 Section 7.3.10: A prvalue of an integer type or of an unscoped
  // enumeration type can be converted to a prvalue of a floatingpoint type. The
  // result is exact if possible. If the value being converted is in the range
  // of values that can be represented but the value cannot be represented
  // exactly, it is an implementation-defined choice of either the next lower or
  // higher representable value. [Note: Loss of precision occurs if the integral
  // value cannot be represented exactly as a value of the floating-point type.
  // — end note] If the value being converted is outside the range of values
  // that can be represented, the behavior is undefined.
  //
  // SAFETY: The output is a floating point 32 or 64 bits. The input is an
  // integer of size <= 64 bits.
  //
  // i64::MIN = -9223372036854775808.
  // u64::MAX = 18446744073709551615.
  // f32::MIN = -340282346638528859811704183484516925440.
  // f32::MAX = 340282346638528859811704183484516925440.
  //
  // Thus the integers of the largest magnitude can be represented by f32, and
  // since f64 is larger they can be represented there too. So no static_cast
  // input here will cause UB.
  return static_cast<Out>(x);
}

template <class Out, class T>
  requires(std::is_floating_point_v<T> && std::is_floating_point_v<Out> &&
           ::sus::mem::size_of<T>() == 8 && ::sus::mem::size_of<Out>() == 4)
sus_pure_const constexpr inline Out static_cast_to_smaller_float(T x) noexcept {
  if (x <= T{max_value<Out>()} && x >= T{min_value<Out>()}) [[likely]] {
    // C++20 Section 7.3.9: A prvalue of floating-point type can be converted to
    // a prvalue of another floating-point type. If the source value can be
    // exactly represented in the destination type, the result of the conversion
    // is that exact representation. If the source value is between two adjacent
    // destination values, the result of the conversion is an
    // implementation-defined choice of either of those values. Otherwise, the
    // behavior is undefined.
    //
    // SAFETY: Because the value `x` is at or between two valid values of type
    // `Out`, the static_cast does not cause UB.
    return static_cast<Out>(x);  // Handles values in range.
  }
  if (x > T{max_value<Out>()}) {
    return infinity<Out>();  // Handles large values and INFINITY.
  }
  if (x < T{min_value<Out>()}) {
    return negative_infinity<Out>();  // Handles small values and NEG_INFINITY.
  }
  return nan<Out>();  // All that's left are NaNs.
}

#pragma warning(pop)

}  // namespace sus::num::__private
