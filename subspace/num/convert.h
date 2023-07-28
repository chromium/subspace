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
    return ::sus::to_bits<T>(from.primitive_value);
  }
};

// sus::construct::ToBits<PrimitiveInteger, PrimitiveFloat> trait.
template <sus::num::PrimitiveInteger T, sus::num::PrimitiveFloat F>
struct sus::construct::ToBitsImpl<T, F> {
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
