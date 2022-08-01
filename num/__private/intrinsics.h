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

#include <stddef.h>
#include <stdint.h>

#include <type_traits>

#if _MSC_VER
#include <intrin.h>
#endif

#include "macros/always_inline.h"
#include "marker/unsafe.h"

namespace sus::num::__private {

template <class T>
struct OverflowOut final {
  bool overflow;
  T value;
};

template <class T>
sus_always_inline constexpr uint32_t unchecked_sizeof() noexcept {
  static_assert(sizeof(T) <= 0xfffffff);
  return sizeof(T);
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T>)
sus_always_inline constexpr T unchecked_neg(T x) noexcept {
  return static_cast<T>(-x);
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T>)
sus_always_inline constexpr T unchecked_not(T x) noexcept {
  return static_cast<T>(~x);
}

template <class T>
  requires(std::is_integral_v<T>)
sus_always_inline constexpr T unchecked_add(T x, T y) noexcept {
  return static_cast<T>(x + y);
}

template <class T>
  requires(std::is_integral_v<T>)
sus_always_inline constexpr T unchecked_sub(T x, T y) noexcept {
  return static_cast<T>(x - y);
}

template <class T>
  requires(std::is_integral_v<T>)
sus_always_inline constexpr T unchecked_mul(T x, T y) noexcept {
  return static_cast<T>(x * y);
}

template <class T>
  requires(std::is_integral_v<T>)
sus_always_inline constexpr T unchecked_div(T x, T y) noexcept {
  return static_cast<T>(x / y);
}

template <class T>
  requires(std::is_integral_v<T>)
sus_always_inline constexpr T unchecked_rem(T x, T y) noexcept {
  return static_cast<T>(x % y);
}

template <class T>
  requires(std::is_integral_v<T>)
sus_always_inline constexpr T unchecked_and(T x, T y) noexcept {
  return static_cast<T>(x & y);
}

template <class T>
  requires(std::is_integral_v<T>)
sus_always_inline constexpr T unchecked_or(T x, T y) noexcept {
  return static_cast<T>(x | y);
}

template <class T>
  requires(std::is_integral_v<T>)
sus_always_inline constexpr T unchecked_xor(T x, T y) noexcept {
  return static_cast<T>(x ^ y);
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T>)
sus_always_inline constexpr T unchecked_shl(T x, uint32_t y) noexcept {
  return static_cast<T>(x << y);
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T>)
sus_always_inline constexpr T unchecked_shr(T x, uint32_t y) noexcept {
  return static_cast<T>(x >> y);
}

template <class T>
  requires(std::is_integral_v<T>)
sus_always_inline constexpr uint32_t num_bits() noexcept {
  return unchecked_mul(unchecked_sizeof<T>(), uint32_t{8});
}

template <class T>
  requires(std::is_integral_v<T> && sizeof(T) <= 8)
sus_always_inline constexpr T high_bit() noexcept {
  if constexpr (sizeof(T) == 1)
    return static_cast<T>(0x80);
  else if constexpr (sizeof(T) == 2)
    return static_cast<T>(0x8000);
  else if constexpr (sizeof(T) == 4)
    return static_cast<T>(0x80000000);
  else
    return static_cast<T>(0x8000000000000000);
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T>)
sus_always_inline constexpr T max_value() noexcept {
  return unchecked_not(T{0});
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8)
sus_always_inline constexpr T max_value() noexcept {
  if constexpr (sizeof(T) == 1)
    return T{0x7f};
  else if constexpr (sizeof(T) == 2)
    return T{0x7fff};
  else if constexpr (sizeof(T) == 4)
    return T{0x7fffffff};
  else
    return T{0x7fffffffffffffff};
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T>)
sus_always_inline constexpr T min_value() noexcept {
  return T{0};
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8)
sus_always_inline constexpr T min_value() noexcept {
  return -max_value<T>() - T{1};
}

template <class T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T> && sizeof(T) <= 8)
sus_always_inline constexpr uint32_t count_ones(T value) noexcept {
#if _MSC_VER
  if (std::is_constant_evaluated()) {
    // Algorithm to count the number of bits in parallel, up to a 128 bit value.
    // http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
    value = value - ((value >> 1) & (~T{0} / T{3}));
    value = (value & (~T{0} / T{15} * T{3})) +
            ((value >> 2) & (~T{0} / T{15} * T{3}));
    value = (value + (value >> 4)) & (~T{0} / T{255} * T{15});
    auto count = (value * (~T{0} / T{255})) >>
                 (unchecked_sizeof<T>() - uint32_t{1}) * uint32_t{8};
    return static_cast<uint32_t>(count);
  } else if constexpr (sizeof(value) <= 2) {
    return uint32_t{__popcnt16(uint16_t{value})};
  } else if constexpr (sizeof(value) == 8) {
    return static_cast<uint32_t>(__popcnt64(uint64_t{value}));
  } else {
    return uint32_t{__popcnt(uint32_t{value})};
  }
#else
  if constexpr (sizeof(value) <= sizeof(unsigned int)) {
    using U = unsigned int;
    return static_cast<uint32_t>(__builtin_popcount(U{value}));
  } else if constexpr (sizeof(value) <= sizeof(unsigned long)) {
    using U = unsigned long;
    return static_cast<uint32_t>(__builtin_popcountl(U{value}));
  } else {
    using U = unsigned long long;
    return static_cast<uint32_t>(__builtin_popcountll(U{value}));
  }
#endif
}

template <class T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T> && sizeof(T) <= 8)
constexpr sus_always_inline uint32_t
    leading_zeros_nonzero(::sus::marker::UnsafeFnMarker, T value) noexcept {
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
  if constexpr (sizeof(value) == 8u) {
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
  } else if constexpr (sizeof(value) == 4u) {
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
    static_assert(sizeof(value) <= 2u);
#if 1
    unsigned long index;
    _BitScanReverse(&index, uint32_t{value});
    return static_cast<uint32_t>((31ul ^ index) -
                                 ((sizeof(unsigned int) - sizeof(value)) * 8u));
#else
    // TODO: Enable this when target CPU is appropriate:
    // - AMD: Advanced Bit Manipulation (ABM)
    // - Intel: Haswell
    // TODO: On Arm ARMv5T architecture and later use `_arm_clz`
    return static_cast<uint32_t>(__lzcnt16(&count, uint16_t{value}) -
                                 ((sizeof(unsigned int) - sizeof(value)) * 8u));
#endif
  }
#else
  if constexpr (sizeof(value) <= sizeof(unsigned int)) {
    using U = unsigned int;
    return static_cast<uint32_t>(__builtin_clz(U{value}) -
                                 ((sizeof(unsigned int) - sizeof(value)) * 8u));
  } else if constexpr (sizeof(value) <= sizeof(unsigned long)) {
    using U = unsigned long;
    return static_cast<uint32_t>(__builtin_clzl(U{value}));
  } else {
    using U = unsigned long long;
    return static_cast<uint32_t>(__builtin_clzll(U{value}));
  }
#endif
}

template <class T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T> && sizeof(T) <= 8)
constexpr sus_always_inline uint32_t leading_zeros(T value) noexcept {
  if (value == 0) return unchecked_mul(unchecked_sizeof<T>(), uint32_t{8});
  return leading_zeros_nonzero(unsafe_fn, value);
}

/** Counts the number of trailing zeros in a non-zero input.
 *
 * # Safety
 * This function produces Undefined Behaviour if passed a zero value.
 */
// TODO: Any way to make it constexpr?
template <class T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T> && sizeof(T) <= 8)
constexpr sus_always_inline uint32_t
    trailing_zeros_nonzero(::sus::marker::UnsafeFnMarker, T value) noexcept {
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
  if constexpr (sizeof(value) == 8u) {
    unsigned long index;
    _BitScanForward64(&index, value);
    return static_cast<uint32_t>(index);
  } else if constexpr (sizeof(value) == 4u) {
    unsigned long index;
    _BitScanForward(&index, uint32_t{value});
    return static_cast<uint32_t>(index);
  } else {
    static_assert(sizeof(value) <= 2u);
    unsigned long index;
    _BitScanForward(&index, uint32_t{value});
    return static_cast<uint32_t>(index);
  }
#else
  if constexpr (sizeof(value) <= sizeof(unsigned int)) {
    using U = unsigned int;
    return static_cast<uint32_t>(__builtin_ctz(U{value}));
  } else if constexpr (sizeof(value) <= sizeof(unsigned long)) {
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
  requires(std::is_integral_v<T> && std::is_unsigned_v<T> && sizeof(T) <= 8)
constexpr sus_always_inline uint32_t trailing_zeros(T value) noexcept {
  if (value == 0) return static_cast<uint32_t>(sizeof(T) * 8u);
  return trailing_zeros_nonzero(unsafe_fn, value);
}

template <class T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T> && sizeof(T) <= 8)
sus_always_inline constexpr T reverse_bits(T value) noexcept {
#if __clang__
  if constexpr (sizeof(T) == 1) {
    return __builtin_bitreverse8(value);
  } else if constexpr (sizeof(T) == 2) {
    return __builtin_bitreverse16(value);
  } else if constexpr (sizeof(T) == 4) {
    return __builtin_bitreverse32(value);
  } else {
    static_assert(sizeof(T) == 8);
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
sus_always_inline constexpr T rotate_left(T value, uint32_t n) noexcept {
  n %= sizeof(value) * 8;
  const auto rshift =
      unchecked_sub(unchecked_mul(unchecked_sizeof<T>(), uint32_t{8}), n);
  return unchecked_shl(value, n) | unchecked_shr(value, rshift);
}

template <class T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T>)
sus_always_inline constexpr T rotate_right(T value, uint32_t n) noexcept {
  n %= sizeof(value) * 8;
  const auto lshift =
      unchecked_sub(unchecked_mul(unchecked_sizeof<T>(), uint32_t{8}), n);
  return unchecked_shr(value, n) | unchecked_shl(value, lshift);
}

template <class T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T> && sizeof(T) <= 8)
sus_always_inline constexpr T swap_bytes(T value) noexcept {
  if (std::is_constant_evaluated()) {
    if constexpr (sizeof(T) == 1) {
      return value;
    } else if constexpr (sizeof(T) == 2) {
      unsigned char a = (value >> 0) & 0xff;
      unsigned char b = (value >> 8) & 0xff;
      return (a << 8) | (b << 0);
    } else if constexpr (sizeof(T) == 4) {
      unsigned char a = (value >> 0) & 0xff;
      unsigned char b = (value >> 8) & 0xff;
      unsigned char c = (value >> 16) & 0xff;
      unsigned char d = (value >> 24) & 0xff;
      return (a << 24) | (b << 16) | (c << 8) | (d << 0);
    } else if constexpr (sizeof(T) == 8) {
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
  if constexpr (sizeof(T) == 1) {
    return value;
  } else if constexpr (sizeof(T) == sizeof(unsigned short)) {
    using U = unsigned short;
    return _byteswap_ushort(U{value});
  } else if constexpr (sizeof(T) == sizeof(unsigned long)) {
    using U = unsigned long;
    return _byteswap_ulong(U{value});
  } else {
    static_assert(sizeof(T) == 8);
    return _byteswap_uint64(value);
  }
#else
  if constexpr (sizeof(T) == 1) {
    return value;
  } else if constexpr (sizeof(T) == 2) {
    return __builtin_bswap16(uint16_t{value});
  } else if constexpr (sizeof(T) == 4) {
    return __builtin_bswap32(value);
  } else {
    static_assert(sizeof(T) == 8);
    return __builtin_bswap64(value);
  }
#endif
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8)
sus_always_inline constexpr auto into_unsigned(T x) noexcept {
  if constexpr (sizeof(x) == 1)
    return static_cast<uint8_t>(x);
  else if constexpr (sizeof(x) == 2)
    return static_cast<uint16_t>(x);
  else if constexpr (sizeof(x) == 4)
    return static_cast<uint32_t>(x);
  else
    return static_cast<uint64_t>(x);
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> && sizeof(T) <= 4)
sus_always_inline constexpr auto into_widened(T x) noexcept {
  if constexpr (sizeof(x) == 1)
    return static_cast<uint16_t>(x);
  else if constexpr (sizeof(x) == 2)
    return static_cast<uint32_t>(x);
  else
    return static_cast<uint64_t>(x);
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 4)
sus_always_inline constexpr auto into_widened(T x) noexcept {
  if constexpr (sizeof(x) == 1)
    return static_cast<int16_t>(x);
  else if constexpr (sizeof(x) == 2)
    return static_cast<int32_t>(x);
  else
    return static_cast<int64_t>(x);
}

template <class T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T> && sizeof(T) <= 8)
sus_always_inline constexpr auto into_signed(T x) noexcept {
  if constexpr (sizeof(x) == 1)
    return static_cast<int8_t>(x);
  else if constexpr (sizeof(x) == 2)
    return static_cast<int16_t>(x);
  else if constexpr (sizeof(x) == 4)
    return static_cast<int32_t>(x);
  else
    return static_cast<int64_t>(x);
}

template <class T>
  requires(sizeof(T) <= 8)
sus_always_inline constexpr bool sign_bit(T x) noexcept {
  if constexpr (sizeof(x) == 1)
    return x & (T(1) << 7) != 0;
  else if constexpr (sizeof(x) == 2)
    return x & (T(1) << 15) != 0;
  else if constexpr (sizeof(x) == 4)
    return x & (T(1) << 31) != 0;
  else
    return x & (T(1) << 63) != 0;
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> && sizeof(T) <= 8)
sus_always_inline
    constexpr OverflowOut<T> add_with_overflow(T x, T y) noexcept {
  return OverflowOut<T>{
      .overflow = x > max_value<T>() - y,
      .value = unchecked_add(x, y),
  };
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8)
sus_always_inline
    constexpr OverflowOut<T> add_with_overflow(T x, T y) noexcept {
  const auto out =
      into_signed(unchecked_add(into_unsigned(x), into_unsigned(y)));
  return OverflowOut<T>{
      .overflow = y >= 0 != out >= x,
      .value = out,
  };
}

template <class T, class U = decltype(to_signed(std::declval<T>()))>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> && sizeof(T) <= 8 &&
           sizeof(T) == sizeof(U))
sus_always_inline
    constexpr OverflowOut<T> add_with_overflow_signed(T x, U y) noexcept {
  return OverflowOut<T>{
      .overflow = (y >= 0 && into_unsigned(y) > max_value<T>() - x) ||
                  (y < 0 && into_unsigned(-y) > x),
      .value = unchecked_add(x, into_unsigned(y)),
  };
}

template <class T, class U = decltype(to_unsigned(std::declval<T>()))>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8 &&
           sizeof(T) == sizeof(U))
sus_always_inline
    constexpr OverflowOut<T> add_with_overflow_unsigned(T x, U y) noexcept {
  const auto out = into_signed(unchecked_add(into_unsigned(x), y));
  return OverflowOut<T>{
      .overflow = static_cast<U>(max_value<T>()) - static_cast<U>(x) < y,
      .value = out,
  };
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> && sizeof(T) <= 8)
sus_always_inline
    constexpr OverflowOut<T> sub_with_overflow(T x, T y) noexcept {
  return OverflowOut<T>{
      .overflow = x < unchecked_add(min_value<T>(), y),
      .value = unchecked_sub(x, y),
  };
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8)
sus_always_inline
    constexpr OverflowOut<T> sub_with_overflow(T x, T y) noexcept {
  const auto out =
      into_signed(unchecked_sub(into_unsigned(x), into_unsigned(y)));
  return OverflowOut<T>{
      .overflow = y >= 0 != out <= x,
      .value = out,
  };
}

template <class T, class U = decltype(to_unsigned(std::declval<T>()))>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8 &&
           sizeof(T) == sizeof(U))
sus_always_inline
    constexpr OverflowOut<T> sub_with_overflow_unsigned(T x, U y) noexcept {
  const auto out = into_signed(unchecked_sub(into_unsigned(x), y));
  return OverflowOut<T>{
      .overflow = static_cast<U>(x) - static_cast<U>(min_value<T>()) < y,
      .value = out,
  };
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> && sizeof(T) <= 4)
sus_always_inline
    constexpr OverflowOut<T> mul_with_overflow(T x, T y) noexcept {
  // TODO: Can we use compiler intrinsics?
  auto out = unchecked_mul(into_widened(x), into_widened(y));
  using Wide = decltype(out);
  return OverflowOut{.overflow = out > Wide{max_value<T>()},
                     .value = static_cast<T>(out)};
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> && sizeof(T) == 8)
sus_always_inline
    constexpr OverflowOut<T> mul_with_overflow(T x, T y) noexcept {
#if _MSC_VER
  if (std::is_constant_evaluated()) {
    const bool overflow =
        x > T{1} && y > T{1} && x > unchecked_div(max_value<T>(), y);
    return OverflowOut{.overflow = overflow, .value = unchecked_mul(x, y)};
  } else {
    // For MSVC, use _umul128, but what about constexpr?? If we can't do
    // it then make the whole function non-constexpr?
    uint64_t highbits;
    auto out = static_cast<T>(_umul128(x, y, &highbits));
    return OverflowOut{.overflow = highbits != 0, .value = out};
  }
#else
  auto out = __uint128_t{x} * __uint128_t{y};
  return OverflowOut{.overflow = out > __uint128_t{max_value<T>()},
                     .value = static_cast<T>(out)};
#endif
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 4)
sus_always_inline
    constexpr OverflowOut<T> mul_with_overflow(T x, T y) noexcept {
  // TODO: Can we use compiler intrinsics?
  auto out = into_widened(x) * into_widened(y);
  using Wide = decltype(out);
  return OverflowOut{
      .overflow = out > Wide{max_value<T>()} || out < Wide{min_value<T>()},
      .value = static_cast<T>(out)};
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) == 8)
sus_always_inline
    constexpr OverflowOut<T> mul_with_overflow(T x, T y) noexcept {
#if _MSC_VER
  if (std::is_constant_evaluated()) {
    if (x == T{0} || y == T{0})
      return OverflowOut{.overflow = false, .value = T{0}};

    using U = decltype(into_unsigned(x));
    const auto absx =
        x >= T{0} ? into_unsigned(x)
                  : unchecked_add(into_unsigned(unchecked_add(x, T{1})), U{1});
    const auto absy =
        y >= T{0} ? into_unsigned(y)
                  : unchecked_add(into_unsigned(unchecked_add(y, T{1})), U{1});
    const bool mul_negative = (x ^ y) < 0;
    const auto mul_max =
        unchecked_add(into_unsigned(max_value<T>()), U{mul_negative});
    const bool overflow = absx > unchecked_div(mul_max, absy);
    const auto mul_val = unchecked_mul(absx, absy);
    return OverflowOut{
        .overflow = overflow,
        .value = mul_negative
                     ? unchecked_sub(unchecked_neg(static_cast<T>(mul_val - 1)), T{1})
                     : static_cast<T>(mul_val)};
  } else {
    // For MSVC, use _mul128, but what about constexpr?? If we can't do
    // it then make the whole function non-constexpr?
    int64_t highbits;
    auto out = static_cast<T>(_mul128(x, y, &highbits));
    return OverflowOut{.overflow = highbits != 0, .value = out};
  }
#else
  auto out = __int128_t{x} * __int128_t{y};
  return OverflowOut{.overflow = out > __int128_t{max_value<T>()} ||
                                 out < __int128_t{min_value<T>()},
                     .value = static_cast<T>(out)};
#endif
}

template <class T>
  requires(std::is_integral_v<T> && sizeof(T) <= 8)
sus_always_inline
    constexpr OverflowOut<T> pow_with_overflow(T base, uint32_t exp) noexcept {
  if (exp == 0) return OverflowOut<T>{.overflow = false, .value = T{1}};
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
  return OverflowOut<T>{.overflow = overflow || r.overflow, .value = r.value};
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> &&
           (sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 ||
            sizeof(T) == 8))
sus_always_inline
    constexpr OverflowOut<T> shl_with_overflow(T x, uint32_t shift) noexcept {
  // Using `num_bits<T>() - 1` as a mask only works if num_bits<T>() is a power
  // of two, so we verify that sizeof(T) is a power of 2, which implies the
  // number of bits is as well (since each byte is 2^3 bits).
  const bool overflow = shift >= num_bits<T>();
  if (overflow) [[unlikely]]
    shift = shift & (unchecked_sub(num_bits<T>(), uint32_t{1}));
  return OverflowOut<T>{.overflow = overflow, .value = unchecked_shl(x, shift)};
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> &&
           (sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 ||
            sizeof(T) == 8))
sus_always_inline
    constexpr OverflowOut<T> shl_with_overflow(T x, uint32_t shift) noexcept {
  // Using `num_bits<T>() - 1` as a mask only works if num_bits<T>() is a power
  // of two, so we verify that sizeof(T) is a power of 2, which implies the
  // number of bits is as well (since each byte is 2^3 bits).
  const bool overflow = shift >= num_bits<T>();
  if (overflow) [[unlikely]]
    shift = shift & (unchecked_sub(num_bits<T>(), uint32_t{1}));
  return OverflowOut<T>{
      .overflow = overflow,
      .value = into_signed(unchecked_shl(into_unsigned(x), shift))};
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> &&
           (sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 ||
            sizeof(T) == 8))
sus_always_inline
    constexpr OverflowOut<T> shr_with_overflow(T x, uint32_t shift) noexcept {
  // Using `num_bits<T>() - 1` as a mask only works if num_bits<T>() is a power
  // of two, so we verify that sizeof(T) is a power of 2, which implies the
  // number of bits is as well (since each byte is 2^3 bits).
  const bool overflow = shift >= num_bits<T>();
  if (overflow) [[unlikely]]
    shift = shift & (unchecked_sub(num_bits<T>(), uint32_t{1}));
  return OverflowOut<T>{.overflow = overflow, .value = unchecked_shr(x, shift)};
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> &&
           (sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 ||
            sizeof(T) == 8))
sus_always_inline
    constexpr OverflowOut<T> shr_with_overflow(T x, uint32_t shift) noexcept {
  // Using `num_bits<T>() - 1` as a mask only works if num_bits<T>() is a power
  // of two, so we verify that sizeof(T) is a power of 2, which implies the
  // number of bits is as well (since each byte is 2^3 bits).
  const bool overflow = shift >= num_bits<T>();
  if (overflow) [[unlikely]]
    shift = shift & (unchecked_sub(num_bits<T>(), uint32_t{1}));
  return OverflowOut<T>{
      .overflow = overflow,
      .value = into_signed(unchecked_shr(into_unsigned(x), shift))};
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> && sizeof(T) <= 8)
sus_always_inline constexpr T saturating_add(T x, T y) noexcept {
  // TODO: Optimize this? Use intrinsics?
  const auto out = add_with_overflow(x, y);
  if (!out.overflow) [[likely]]
    return out.value;
  else
    return max_value<T>();
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8)
sus_always_inline constexpr T saturating_add(T x, T y) noexcept {
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
  requires(std::is_integral_v<T> && !std::is_signed_v<T> && sizeof(T) <= 8)
sus_always_inline constexpr T saturating_sub(T x, T y) noexcept {
  // TODO: Optimize this? Use intrinsics?
  const auto out = sub_with_overflow(x, y);
  if (!out.overflow) [[likely]]
    return out.value;
  else
    return min_value<T>();
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8)
sus_always_inline constexpr T saturating_sub(T x, T y) noexcept {
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
  requires(std::is_integral_v<T> && !std::is_signed_v<T> && sizeof(T) <= 8)
sus_always_inline constexpr T saturating_mul(T x, T y) noexcept {
  // TODO: Optimize this? Use intrinsics?
  const auto out = mul_with_overflow(x, y);
  if (!out.overflow) [[likely]]
    return out.value;
  else
    return max_value<T>();
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8)
sus_always_inline constexpr T saturating_mul(T x, T y) noexcept {
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
  requires(std::is_integral_v<T> && !std::is_signed_v<T> && sizeof(T) <= 8)
sus_always_inline constexpr T wrapping_add(T x, T y) noexcept {
  return x + y;
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8)
sus_always_inline constexpr T wrapping_add(T x, T y) noexcept {
  // TODO: Are there cheaper intrinsics?
  return add_with_overflow(x, y).value;
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> && sizeof(T) <= 8)
sus_always_inline constexpr T wrapping_sub(T x, T y) noexcept {
  return unchecked_sub(x, y);
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8)
sus_always_inline constexpr T wrapping_sub(T x, T y) noexcept {
  // TODO: Are there cheaper intrinsics?
  return sub_with_overflow(x, y).value;
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> && sizeof(T) <= 8)
sus_always_inline constexpr T wrapping_mul(T x, T y) noexcept {
  return x * y;
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8)
sus_always_inline constexpr T wrapping_mul(T x, T y) noexcept {
  // TODO: Are there cheaper intrinsics?
  return mul_with_overflow(x, y).value;
}

template <class T>
  requires(std::is_integral_v<T> && !std::is_signed_v<T> && sizeof(T) <= 8)
sus_always_inline constexpr T wrapping_pow(T base, uint32_t exp) noexcept {
  // TODO: Don't need to track overflow and unsigned wraps by default, so this
  // can be cheaper.
  return pow_with_overflow(base, exp).value;
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8)
sus_always_inline constexpr T wrapping_pow(T base, uint32_t exp) noexcept {
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
  requires(std::is_integral_v<T> && !std::is_signed_v<T> && sizeof(T) <= 8)
sus_always_inline constexpr T one_less_than_next_power_of_two(T x) noexcept {
  if (x <= 1u) {
    return 0u;
  } else {
    const auto p = unchecked_sub(x, T{1});
    // SAFETY: Because `p > 0`, it cannot consist entirely of leading zeros.
    // That means the shift is always in-bounds, and some processors (such as
    // intel pre-haswell) have more efficient ctlz intrinsics when the argument
    // is non-zero.
    const auto z = leading_zeros_nonzero(unsafe_fn, p);
    return unchecked_shr(max_value<T>(), z);
  }
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8)
sus_always_inline constexpr bool div_overflows(T x, T y) noexcept {
  // Using `&` helps LLVM see that it is the same check made in division.
  return y == T{0} || ((x == min_value<T>()) & (y == T{-1}));
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8)
sus_always_inline
    constexpr bool div_overflows_nonzero(::sus::marker::UnsafeFnMarker, T x,
                                         T y) noexcept {
  // Using `&` helps LLVM see that it is the same check made in division.
  return ((x == min_value<T>()) & (y == T{-1}));
}

// SAFETY: Requires that !div_overflows(x, y) or Undefined Behaviour results.
template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8)
constexpr T div_euclid(::sus::marker::UnsafeFnMarker, T x, T y) noexcept {
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
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8)
constexpr T rem_euclid(::sus::marker::UnsafeFnMarker, T x, T y) noexcept {
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

}  // namespace sus::num::__private
