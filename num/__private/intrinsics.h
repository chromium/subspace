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

#include <stdint.h>

#include <type_traits>

#if _MSC_VER
#include <intrin.h>
#endif

namespace sus::num::__private {

template <class T>
struct OverflowOut {
  bool overflow;
  T value;
};

template <class T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T> && sizeof(T) <= 8)
inline constexpr uint32_t count_ones(T value) noexcept {
#if _MSC_VER
  if (std::is_constant_evaluated()) {
    // Algorithm to count the number of bits in parallel, up to a 128 bit value.
    // http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
    value = value - (value >> 1) & ~T{0} / T{3};
    value =
        (value & ~T{0} / T{15} * T{3}) + ((value >> 2) & ~T{0} / T{15} * T{3});
    value = (value + (value >> 4)) & ~T{0} / T{255} * T{15};
    return T{value * (~T{0} / T{255})} >> (sizeof(T) - 1) * 8;
  } else if constexpr (sizeof(value) <= 2) {
    return __popcnt16(uint16_t{value});
  } else if constexpr (sizeof(value) == 8) {
    return __popcnt64(uint64_t{value});
  } else {
    return __popcnt(uint32_t{value});
  }
#else
  if constexpr (sizeof(value) <= sizeof(unsigned int)) {
    using U = unsigned int;
    return __builtin_popcount(U{value});
  } else if constexpr (sizeof(value) <= sizeof(unsigned long)) {
    using U = unsigned long;
    return __builtin_popcountl(U{value});
  } else {
    using U = unsigned long long;
    return __builtin_popcountll(U{value});
  }
#endif
}

template <class T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T> && sizeof(T) <= 8)
inline constexpr uint32_t leading_zeros(T value) noexcept {
  if (value == 0) return static_cast<uint32_t>(sizeof(T) * 8u);

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
    return static_cast<uint32_t>(31ul ^ index);
#else
    // TODO: Enable this when target CPU is appropriate:
    // - AMD: Advanced Bit Manipulation (ABM)
    // - Intel: Haswell
    // TODO: On Arm ARMv5T architecture and later use `_arm_clz`
    return static_cast<uint32_t>(__lzcnt16(&count, uint16_t{value}));
#endif
  }
#else
  if constexpr (sizeof(value) <= sizeof(unsigned int)) {
    using U = unsigned int;
    return static_cast<uint32_t>(__builtin_clz(U{value}));
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
inline constexpr uint32_t trailing_zeros(T value) noexcept {
  if (value == 0) return static_cast<uint32_t>(sizeof(T) * 8u);

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

template <class T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T> && sizeof(T) <= 8)
inline constexpr T reverse_bits(T value) noexcept {
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
  unsigned int bits = sizeof(value) * 8;
  auto mask = ~T(0);
  while ((bits >>= 1) > 0) {
    mask ^= mask << bits;
    value = ((value >> bits) & mask) | ((value << bits) & ~mask);
  }
  return value;
#endif
}

template <class T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T>)
inline constexpr T rotate_left(T value, uint32_t n) noexcept {
  n %= sizeof(value) * 8;
  return (value << n) | (value >> (sizeof(value) * 8 - n));
}

template <class T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T>)
inline constexpr T rotate_right(T value, uint32_t n) noexcept {
  n %= sizeof(value) * 8;
  return (value >> n) | (value << (sizeof(value) * 8 - n));
}

template <class T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T> && sizeof(T) <= 8)
inline constexpr T swap_bytes(T value) noexcept {
#if _MSC_VER
  if constexpr (sizeof(T) <= sizeof(unsigned short)) {
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
  if constexpr (sizeof(T) <= 2) {
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
inline constexpr auto into_unsigned(T x) noexcept {
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
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 4)
inline constexpr auto into_widened(T x) noexcept {
  if constexpr (sizeof(x) == 1)
    return static_cast<int16_t>(x);
  else if constexpr (sizeof(x) == 2)
    return static_cast<int32_t>(x);
  else
    return static_cast<int64_t>(x);
}

template <class T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T> && sizeof(T) <= 8)
inline constexpr auto into_signed(T x) noexcept {
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
inline constexpr bool sign_bit(T x) noexcept {
  if constexpr (sizeof(x) == 1)
    return x & (1 << 7) != 0;
  else if constexpr (sizeof(x) == 2)
    return x & (1 << 15) != 0;
  else if constexpr (sizeof(x) == 4)
    return x & (1 << 31) != 0;
  else
    return x & (1 << 63) != 0;
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8)
inline constexpr OverflowOut<T> add_with_overflow(T x, T y) noexcept {
  auto out = into_signed(into_unsigned(x) + into_unsigned(y));
  return OverflowOut{
      .overflow = y >= 0 != out >= x,
      .value = out,
  };
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8)
inline constexpr OverflowOut<T> sub_with_overflow(T x, T y) noexcept {
  auto out = into_signed(into_unsigned(x) - into_unsigned(y));
  return OverflowOut{
      .overflow = y >= 0 != out <= x,
      .value = out,
  };
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8)
inline constexpr auto max_value() noexcept {
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
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8)
inline constexpr auto min_value() noexcept {
  return -max_value<T>() - 1;
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 4)
inline constexpr OverflowOut<T> mul_with_overflow(T x, T y) noexcept {
  constexpr auto max = max_value<T>();
  constexpr auto min = min_value<T>();
  // TODO: Optimize this. Use compiler intrinsics.
  auto out = into_widened(x) * into_widened(y);
  using Wide = decltype(out);
  if (out <= max && out >= min) [[likely]] {
    return OverflowOut{.overflow = false, .value = static_cast<T>(out)};
  } else {
    // TODO: Do we really need to loop here though, we just a signed modulo.
    while (out > Wide{max}) out -= Wide{max} - Wide{min} + 1;
    while (out < Wide{min}) out += Wide{max} - Wide{min} + 1;
    return OverflowOut{.overflow = true, .value = static_cast<T>(out)};
  }
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) == 8)
inline constexpr OverflowOut<T> mul_with_overflow(T x, T y) noexcept {
  // TODO: For GCC/Clang, use __int128:
  // https://quuxplusone.github.io/blog/2019/02/28/is-int128-integral/
  // For MSVC, use _mult128, but what about constexpr?? If we can't do
  // it then make the whole function non-constexpr?
  // https://docs.microsoft.com/en-us/cpp/intrinsics/mul128?view=msvc-170
  static_assert(sizeof(T) != 8);
  return OverflowOut<T>(false, T(0));
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8)
inline constexpr T saturating_add(T x, T y) noexcept {
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
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8)
inline constexpr T saturating_sub(T x, T y) noexcept {
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
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8)
inline constexpr T saturating_mul(T x, T y) noexcept {
  // TODO: Optimize this? Use intrinsics?
  auto out = mul_with_overflow(x, y);
  if (!out.overflow) [[likely]]
    return out.value;
  else if (x > 0 == y > 0)
    return max_value<T>();
  else
    return min_value<T>();
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8)
inline constexpr T wrapping_add(T x, T y) noexcept {
  // TODO: Are there cheaper intrinsics?
  return add_with_overflow(x, y).value;
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8)
inline constexpr T wrapping_sub(T x, T y) noexcept {
  // TODO: Are there cheaper intrinsics?
  return sub_with_overflow(x, y).value;
}

template <class T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8)
inline constexpr T wrapping_mul(T x, T y) noexcept {
  // TODO: Are there cheaper intrinsics?
  return mul_with_overflow(x, y).value;
}

}  // namespace sus::num::__private
