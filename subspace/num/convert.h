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

#include <bit>
#include <type_traits>

#include "subspace/construct/to_bits.h"
#include "subspace/num/float.h"
#include "subspace/num/signed_integer.h"
#include "subspace/num/unsigned_integer.h"

/// Casting from a float to an integer will round the float towards zero,
/// except:
/// * NaN will return 0.
/// * Values larger than the maximum integer value, including `INFINITY`, will
///   saturate to the maximum value of the integer type.
/// * Values smaller than the minimum integer value, including `NEG_INFINITY`,
///   will saturate to the minimum value of the integer type.

// # ================ From signed integers. ============================

// ## === Into `Integer`

// sus::construct::ToBits<Integer, Integer> trait.
template <sus::num::Integer T, sus::num::Integer F>
struct sus::construct::ToBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    return T(::sus::to_bits<decltype(std::declval<T>().primitive_value)>(
        from.primitive_value));
  }
};

// sus::construct::ToBits<Integer, PrimitiveInteger> trait.
template <sus::num::Integer T, sus::num::PrimitiveInteger F>
struct sus::construct::ToBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    return T(::sus::to_bits<decltype(std::declval<T>().primitive_value)>(from));
  }
};

// sus::construct::ToBits<Integer, std::byte> trait.
template <sus::num::Integer T>
struct sus::construct::ToBitsImpl<T, std::byte> {
  constexpr static T from_bits(const std::byte& from) noexcept {
    return T(::sus::to_bits<decltype(std::declval<T>().primitive_value)>(from));
  }
};

// sus::construct::ToBits<Integer, Float> trait.
template <sus::num::Integer T, sus::num::Float F>
struct sus::construct::ToBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    return T(::sus::to_bits<decltype(std::declval<T>().primitive_value)>(
        from.primitive_value));
  }
};

// sus::construct::ToBits<Integer, PrimitiveFloat> trait.
template <sus::num::Integer T, sus::num::PrimitiveFloat F>
struct sus::construct::ToBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    return T(::sus::to_bits<decltype(std::declval<T>().primitive_value)>(from));
  }
};

// ## === Into `PrimitiveInteger`

// sus::construct::ToBits<PrimitiveInteger, Integer> trait.
template <sus::num::PrimitiveInteger T, sus::num::Integer F>
struct sus::construct::ToBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    return ::sus::to_bits<T>(from.primitive_value);
  }
};

// sus::construct::ToBits<PrimitiveInteger, PrimitiveInteger> trait.
template <sus::num::PrimitiveInteger T, sus::num::PrimitiveInteger F>
struct sus::construct::ToBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    return static_cast<T>(from);
  }
};

// sus::construct::ToBits<PrimitiveInteger, std::byte> trait.
template <sus::num::PrimitiveInteger T>
struct sus::construct::ToBitsImpl<T, std::byte> {
  constexpr static T from_bits(const std::byte& from) noexcept {
    return static_cast<T>(from);
  }
};

// sus::construct::ToBits<PrimitiveInteger, Float> trait.
template <sus::num::PrimitiveInteger T, sus::num::Float F>
struct sus::construct::ToBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    if (from.is_nan()) [[unlikely]]
      return T(0);

    struct MinMax {
      F min, max;
    };
    constexpr MinMax minmax = []() {
      if constexpr (::sus::mem::size_of<T>() == 1u) {
        if constexpr (std::is_signed_v<T>) {
          if constexpr (::sus::mem::size_of<F>() == 4u) {
            return MinMax(-128.f, 127.f);
          } else {
            return MinMax(-128.0, 127.0);
          }
        } else {
          if constexpr (::sus::mem::size_of<F>() == 4u) {
            return MinMax(0.f, 255.f);
          } else {
            return MinMax(0.0, 255.0);
          }
        }
      } else if constexpr (::sus::mem::size_of<T>() == 2u) {
        if constexpr (std::is_signed_v<T>) {
          if constexpr (::sus::mem::size_of<F>() == 4u) {
            return MinMax(-32768.f, 32767.f);
          } else {
            return MinMax(-32768.0, 32767.0);
          }
        } else {
          if constexpr (::sus::mem::size_of<F>() == 4u) {
            return MinMax(0.f, 65535.f);
          } else {
            return MinMax(0.0, 65535.0);
          }
        }
      } else if constexpr (::sus::mem::size_of<T>() == 4u) {
        if constexpr (std::is_signed_v<T>) {
          if constexpr (::sus::mem::size_of<F>() == 4u) {
            return MinMax(-2147483648.f, 2147483647.f);
          } else {
            return MinMax(-2147483648.0, 2147483647.0);
          }
        } else {
          if constexpr (::sus::mem::size_of<F>() == 4u) {
            return MinMax(0.f, 4294967295.f);
          } else {
            return MinMax(0.0, 4294967295.0);
          }
        }
      } else {
        static_assert(::sus::mem::size_of<T>() == 8u);
        if constexpr (std::is_signed_v<T>) {
          if constexpr (::sus::mem::size_of<F>() == 4u) {
            return MinMax(-9223372036854775808.f, 9223372036854775807.f);
          } else {
            return MinMax(-9223372036854775808.0, 9223372036854775807.0);
          }
        } else {
          if constexpr (::sus::mem::size_of<F>() == 4u) {
            return MinMax(0.f, 18446744073709551615.f);
          } else {
            return MinMax(0.0, 18446744073709551615.0);
          }
        }
      }
    }();
    if (from >= minmax.max) [[unlikely]]
      return ::sus::num::__private::max_value<T>();
    if (from <= minmax.min) [[unlikely]]
      return ::sus::num::__private::min_value<T>();
    return static_cast<T>(from.primitive_value);
  }
};

// sus::construct::ToBits<PrimitiveInteger, PrimitiveFloat> trait.
template <sus::num::PrimitiveInteger T, sus::num::PrimitiveFloat F>
struct sus::construct::ToBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    if constexpr (::sus::mem::size_of<F>() == 4u) {
      return ::sus::to_bits<T>(::sus::num::f32(from));
    } else {
      static_assert(::sus::mem::size_of<F>() == 8u);
      return ::sus::to_bits<T>(::sus::num::f64(from));
    }
  }
};

// ## === Into `std::byte`

// sus::construct::ToBits<std::byte, Integer> trait.
template <sus::num::Integer F>
struct sus::construct::ToBitsImpl<std::byte, F> {
  constexpr static std::byte from_bits(const F& from) noexcept {
    return ::sus::to_bits<std::byte>(from.primitive_value);
  }
};

// sus::construct::ToBits<std::byte, PrimitiveInteger> trait.
template <sus::num::PrimitiveInteger F>
struct sus::construct::ToBitsImpl<std::byte, F> {
  constexpr static std::byte from_bits(const F& from) noexcept {
    return static_cast<std::byte>(from);
  }
};

// sus::construct::ToBits<std::byte, Float> trait.
template <sus::num::Float F>
struct sus::construct::ToBitsImpl<std::byte, F> {
  constexpr static std::byte from_bits(const F& from) noexcept {
    return ::sus::to_bits<std::byte>(from.primitive_value);
  }
};

// sus::construct::ToBits<PrimitiveInteger, PrimitiveFloat> trait.
template <sus::num::PrimitiveFloat F>
struct sus::construct::ToBitsImpl<std::byte, F> {
  constexpr static std::byte from_bits(const F& from) noexcept {
    if constexpr (::sus::mem::size_of<F>() == 4u) {
      return static_cast<std::byte>(std::bit_cast<uint32_t>(from));
    } else {
      static_assert(::sus::mem::size_of<F>() == 8u);
      return static_cast<std::byte>(std::bit_cast<uint64_t>(from));
    }
  }
};

// ## === Into `Float`

// sus::construct::ToBits<Float, Integer> trait.
template <sus::num::Float T, sus::num::Integer F>
struct sus::construct::ToBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    return T(::sus::to_bits<decltype(std::declval<T>().primitive_value)>(
        from.primitive_value));
  }
};

// sus::construct::ToBits<Float, PrimitiveInteger> trait.
template <sus::num::Float T, sus::num::PrimitiveInteger F>
struct sus::construct::ToBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    return T(::sus::to_bits<decltype(std::declval<T>().primitive_value)>(from));
  }
};

// sus::construct::ToBits<Float, std::byte> trait.
template <sus::num::Float T>
struct sus::construct::ToBitsImpl<T, std::byte> {
  constexpr static T from_bits(const std::byte& from) noexcept {
    return T(::sus::to_bits<decltype(std::declval<T>().primitive_value)>(from));
  }
};

// sus::construct::ToBits<Float, Float> trait.
template <sus::num::Float T, sus::num::Float F>
struct sus::construct::ToBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    return T(::sus::to_bits<decltype(std::declval<T>().primitive_value)>(
        from.primitive_value));
  }
};

// sus::construct::ToBits<Float, PrimitiveFloat> trait.
template <sus::num::Float T, sus::num::PrimitiveFloat F>
struct sus::construct::ToBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    return T(::sus::to_bits<decltype(std::declval<T>().primitive_value)>(from));
  }
};

// ## === Into `PrimitiveFloat`

// sus::construct::ToBits<PrimitiveFloat, Integer> trait.
template <sus::num::PrimitiveFloat T, sus::num::Integer F>
struct sus::construct::ToBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    return ::sus::to_bits<T>(from.primitive_value);
  }
};

// sus::construct::ToBits<PrimitiveFloat, PrimitiveInteger> trait.
template <sus::num::PrimitiveFloat T, sus::num::PrimitiveInteger F>
struct sus::construct::ToBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    if constexpr (::sus::mem::size_of<T>() == 4u) {
      return std::bit_cast<float>(
          static_cast<uint32_t>(static_cast<std::make_unsigned_t<F>>(from)));
    } else {
      static_assert(::sus::mem::size_of<T>() == 8u);
      return std::bit_cast<double>(
          static_cast<uint64_t>(static_cast<std::make_unsigned_t<F>>(from)));
    }
  }
};

// sus::construct::ToBits<PrimitiveFloat, std::byte> trait.
template <sus::num::PrimitiveFloat T>
struct sus::construct::ToBitsImpl<T, std::byte> {
  constexpr static T from_bits(const std::byte& from) noexcept {
    if constexpr (::sus::mem::size_of<T>() == 4u) {
      return std::bit_cast<float>(static_cast<uint32_t>(from));
    } else {
      static_assert(::sus::mem::size_of<T>() == 8u);
      return std::bit_cast<double>(static_cast<uint64_t>(from));
    }
  }
};

// sus::construct::ToBits<PrimitiveFloat, Float> trait.
template <sus::num::PrimitiveFloat T, sus::num::Float F>
struct sus::construct::ToBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    return ::sus::to_bits<T>(from.primitive_value);
  }
};

// sus::construct::ToBits<PrimitiveFloat, PrimitiveFloat> trait.
template <sus::num::PrimitiveFloat T, sus::num::PrimitiveFloat F>
struct sus::construct::ToBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    static_assert(::sus::mem::size_of<T>() == 4u ||
                  ::sus::mem::size_of<T>() == 8u);
    static_assert(::sus::mem::size_of<F>() == 4u ||
                  ::sus::mem::size_of<F>() == 8u);

    if constexpr (::sus::mem::size_of<T>() == 4u) {
      if constexpr (::sus::mem::size_of<F>() == 4u) {
        return from;
      } else {
        return std::bit_cast<float>(
            static_cast<uint32_t>(std::bit_cast<uint64_t>(from)));
      }
    } else {
      if constexpr (::sus::mem::size_of<F>() == 4u) {
        return std::bit_cast<double>(
            static_cast<uint64_t>(std::bit_cast<uint32_t>(from)));
      } else {
        return from;
      }
    }
  }
};
