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

#include "subspace/construct/as_bits.h"
#include "subspace/num/float.h"
#include "subspace/num/signed_integer.h"
#include "subspace/num/unsigned_integer.h"

// # ================ From signed integers. ============================

// ## === Into `Integer`

// sus::construct::AsBits<Integer, Integer> trait.
template <sus::num::Integer T, sus::num::Integer F>
struct sus::construct::AsBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    return T(::sus::as_bits<decltype(std::declval<T>().primitive_value)>(
        from.primitive_value));
  }
};

// sus::construct::AsBits<Integer, PrimitiveInteger> trait.
template <sus::num::Integer T, sus::num::PrimitiveInteger F>
struct sus::construct::AsBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    return T(::sus::as_bits<decltype(std::declval<T>().primitive_value)>(from));
  }
};

// sus::construct::AsBits<Integer, std::byte> trait.
template <sus::num::Integer T>
struct sus::construct::AsBitsImpl<T, std::byte> {
  constexpr static T from_bits(const std::byte& from) noexcept {
    return T(::sus::as_bits<decltype(std::declval<T>().primitive_value)>(from));
  }
};

// sus::construct::AsBits<Integer, Float> trait.
template <sus::num::Integer T, sus::num::Float F>
struct sus::construct::AsBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    return T(::sus::as_bits<decltype(std::declval<T>().primitive_value)>(
        from.primitive_value));
  }
};

// sus::construct::AsBits<Integer, PrimitiveFloat> trait.
template <sus::num::Integer T, sus::num::PrimitiveFloat F>
struct sus::construct::AsBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    return T(::sus::as_bits<decltype(std::declval<T>().primitive_value)>(from));
  }
};

// ## === Into `PrimitiveInteger`

// sus::construct::AsBits<PrimitiveInteger, Integer> trait.
template <sus::num::PrimitiveInteger T, sus::num::Integer F>
struct sus::construct::AsBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    return ::sus::as_bits<T>(from.primitive_value);
  }
};

// sus::construct::AsBits<PrimitiveInteger, PrimitiveInteger> trait.
template <sus::num::PrimitiveInteger T, sus::num::PrimitiveInteger F>
struct sus::construct::AsBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    return static_cast<T>(from);
  }
};

// sus::construct::AsBits<PrimitiveInteger, std::byte> trait.
template <sus::num::PrimitiveInteger T>
struct sus::construct::AsBitsImpl<T, std::byte> {
  constexpr static T from_bits(const std::byte& from) noexcept {
    return static_cast<T>(from);
  }
};

// sus::construct::AsBits<PrimitiveInteger, Float> trait.
template <sus::num::PrimitiveInteger T, sus::num::Float F>
struct sus::construct::AsBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    return ::sus::as_bits<T>(from.primitive_value);
  }
};

// sus::construct::AsBits<PrimitiveInteger, PrimitiveFloat> trait.
template <sus::num::PrimitiveInteger T, sus::num::PrimitiveFloat F>
struct sus::construct::AsBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    if constexpr (::sus::mem::size_of<F>() == 4u) {
      return static_cast<T>(
          static_cast<std::make_unsigned_t<T>>(std::bit_cast<uint32_t>(from)));
    } else {
      static_assert(::sus::mem::size_of<F>() == 8u);
      return static_cast<T>(
          static_cast<std::make_unsigned_t<T>>(std::bit_cast<uint64_t>(from)));
    }
  }
};

// ## === Into `std::byte`

// sus::construct::AsBits<std::byte, Integer> trait.
template <sus::num::Integer F>
struct sus::construct::AsBitsImpl<std::byte, F> {
  constexpr static std::byte from_bits(const F& from) noexcept {
    return ::sus::as_bits<std::byte>(from.primitive_value);
  }
};

// sus::construct::AsBits<std::byte, PrimitiveInteger> trait.
template <sus::num::PrimitiveInteger F>
struct sus::construct::AsBitsImpl<std::byte, F> {
  constexpr static std::byte from_bits(const F& from) noexcept {
    return static_cast<std::byte>(from);
  }
};

// sus::construct::AsBits<std::byte, Float> trait.
template <sus::num::Float F>
struct sus::construct::AsBitsImpl<std::byte, F> {
  constexpr static std::byte from_bits(const F& from) noexcept {
    return ::sus::as_bits<std::byte>(from.primitive_value);
  }
};

// sus::construct::AsBits<PrimitiveInteger, PrimitiveFloat> trait.
template <sus::num::PrimitiveFloat F>
struct sus::construct::AsBitsImpl<std::byte, F> {
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

// sus::construct::AsBits<Float, Integer> trait.
template <sus::num::Float T, sus::num::Integer F>
struct sus::construct::AsBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    return T(::sus::as_bits<decltype(std::declval<T>().primitive_value)>(
        from.primitive_value));
  }
};

// sus::construct::AsBits<Float, PrimitiveInteger> trait.
template <sus::num::Float T, sus::num::PrimitiveInteger F>
struct sus::construct::AsBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    return T(::sus::as_bits<decltype(std::declval<T>().primitive_value)>(from));
  }
};

// sus::construct::AsBits<Float, std::byte> trait.
template <sus::num::Float T>
struct sus::construct::AsBitsImpl<T, std::byte> {
  constexpr static T from_bits(const std::byte& from) noexcept {
    return T(::sus::as_bits<decltype(std::declval<T>().primitive_value)>(from));
  }
};

// sus::construct::AsBits<Float, Float> trait.
template <sus::num::Float T, sus::num::Float F>
struct sus::construct::AsBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    return T(::sus::as_bits<decltype(std::declval<T>().primitive_value)>(
        from.primitive_value));
  }
};

// sus::construct::AsBits<Float, PrimitiveFloat> trait.
template <sus::num::Float T, sus::num::PrimitiveFloat F>
struct sus::construct::AsBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    return T(::sus::as_bits<decltype(std::declval<T>().primitive_value)>(from));
  }
};

// ## === Into `PrimitiveFloat`

// sus::construct::AsBits<PrimitiveFloat, Integer> trait.
template <sus::num::PrimitiveFloat T, sus::num::Integer F>
struct sus::construct::AsBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    return ::sus::as_bits<T>(from.primitive_value);
  }
};

// sus::construct::AsBits<PrimitiveFloat, PrimitiveInteger> trait.
template <sus::num::PrimitiveFloat T, sus::num::PrimitiveInteger F>
struct sus::construct::AsBitsImpl<T, F> {
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

// sus::construct::AsBits<PrimitiveFloat, std::byte> trait.
template <sus::num::PrimitiveFloat T>
struct sus::construct::AsBitsImpl<T, std::byte> {
  constexpr static T from_bits(const std::byte& from) noexcept {
    if constexpr (::sus::mem::size_of<T>() == 4u) {
      return std::bit_cast<float>(static_cast<uint32_t>(from));
    } else {
      static_assert(::sus::mem::size_of<T>() == 8u);
      return std::bit_cast<double>(static_cast<uint64_t>(from));
    }
  }
};

// sus::construct::AsBits<PrimitiveFloat, Float> trait.
template <sus::num::PrimitiveFloat T, sus::num::Float F>
struct sus::construct::AsBitsImpl<T, F> {
  constexpr static T from_bits(const F& from) noexcept {
    return ::sus::as_bits<T>(from.primitive_value);
  }
};

// sus::construct::AsBits<PrimitiveFloat, PrimitiveFloat> trait.
template <sus::num::PrimitiveFloat T, sus::num::PrimitiveFloat F>
struct sus::construct::AsBitsImpl<T, F> {
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
