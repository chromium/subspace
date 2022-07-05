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

#include <concepts>
#include <type_traits>

#if _MSC_VER
#include <intrin.h>
#endif

namespace sus::num::__private {

// TODO: Move to a (constexpr) compiler intrinsics library?
template <class T>
  requires(std::same_as<T, uint32_t> || std::same_as<T, uint16_t> ||
           std::same_as<T, uint8_t>)
constexpr uint32_t count_ones(T value) noexcept {
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

// TODO: Move to a (constexpr) compiler intrinsics library?
template <class T>
  requires(std::same_as<T, uint32_t> || std::same_as<T, uint16_t> ||
           std::same_as<T, uint8_t>)
constexpr uint32_t leading_zeros(T value) noexcept {
  if (value == 0) return sizeof(T) * 8;

#if _MSC_VER
  if constexpr (sizeof(value) == 8) {
#if 1
    unsigned long index;
    _BitScanReverse64(&index, value);
    return index;
#else
    // TODO: Enable this when target CPU is appropriate:
    // - AMD: Advanced Bit Manipulation (ABM)
    // - Intel: Haswell
    return __lzcnt64(&count, int64_t{value});
#endif
  } else if constexpr (sizeof(value) == 4) {
#if 1
    unsigned long index;
    _BitScanReverse(&index, uint32_t{value});
    return 31 ^ index;
#else
    // TODO: Enable this when target CPU is appropriate:
    // - AMD: Advanced Bit Manipulation (ABM)
    // - Intel: Haswell
    return __lzcnt(&count, uint32_t{value});
#endif
  } else {
#if 1
    unsigned long index;
    _BitScanReverse(&index, uint32_t{value});
    return 31 ^ index;
#else
    // TODO: Enable this when target CPU is appropriate:
    // - AMD: Advanced Bit Manipulation (ABM)
    // - Intel: Haswell
    return __lzcnt16(&count, uint16_t{value});
#endif
  }
#else
  if constexpr (sizeof(value) <= sizeof(unsigned int)) {
    using U = unsigned int;
    return __builtin_clz(U{value});
  } else if constexpr (sizeof(value) <= sizeof(unsigned long)) {
    using U = unsigned long;
    return __builtin_clzl(U{value});
  } else {
    using U = unsigned long long;
    return __builtin_clzll(U{value});
  }
#endif
}

// TODO: Move to a (constexpr) compiler intrinsics library?
template <class T>
  requires(std::same_as<T, uint32_t> || std::same_as<T, uint16_t> ||
           std::same_as<T, uint8_t>)
constexpr T reverse_bits(T value) noexcept {
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

// TODO: Move to a (constexpr) compiler intrinsics library?
template <class T>
  requires(std::same_as<T, uint32_t> || std::same_as<T, uint16_t> ||
           std::same_as<T, uint8_t>)
constexpr T rotate_left(T value, uint32_t n) noexcept {
  n %= sizeof(value) * 8;
  return (value << n) | (value >> (sizeof(value) * 8 - n));
}

// TODO: Move to a (constexpr) compiler intrinsics library?
template <class T>
  requires(std::same_as<T, uint32_t> || std::same_as<T, uint16_t> ||
           std::same_as<T, uint8_t>)
constexpr T rotate_right(T value, uint32_t n) noexcept {
  n %= sizeof(value) * 8;
  return (value >> n) | (value << (sizeof(value) * 8 - n));
}

// TODO: Move to a (constexpr) compiler intrinsics library?
template <class T>
  requires(std::same_as<T, uint32_t> || std::same_as<T, uint16_t> ||
           std::same_as<T, uint8_t>)
constexpr T swap_bytes(T value) noexcept {
#if _MSC_VER
  if constexpr(sizeof(T) <= sizeof(unsigned short)) {
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

}  // namespace sus::num::__private
