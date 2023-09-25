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

// IWYU pragma: private, include "sus/num/types.h"
// IWYU pragma: friend "sus/.*"
#pragma once

#include <type_traits>

#include "sus/construct/transmogrify.h"
#include "sus/lib/__private/forward_decl.h"
#include "sus/num/__private/intrinsics.h"
#include "sus/num/float_concepts.h"
#include "sus/num/integer_concepts.h"

// # ================ From signed integers. ============================

// ## === Into `Integer`

// sus::construct::Transmogrify<Integer, Integer> trait.
template <sus::num::Integer T, sus::num::Integer F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    return T(::sus::mog<decltype(std::declval<T>().primitive_value)>(
        from.primitive_value));
  }
};

// sus::construct::Transmogrify<Integer, PrimitiveInteger> trait.
template <sus::num::Integer T, sus::num::PrimitiveInteger F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    return T(::sus::mog<decltype(std::declval<T>().primitive_value)>(from));
  }
};

// sus::construct::Transmogrify<Integer, PrimitiveEnum> trait.
template <sus::num::Integer T, sus::num::PrimitiveEnum F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    return T(::sus::mog<decltype(std::declval<T>().primitive_value)>(
        static_cast<std::underlying_type_t<F>>(from)));
  }
};

// sus::construct::Transmogrify<Integer, PrimitiveEnumClass> trait.
template <sus::num::Integer T, sus::num::PrimitiveEnumClass F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    return T(::sus::mog<decltype(std::declval<T>().primitive_value)>(
        static_cast<std::underlying_type_t<F>>(from)));
  }
};

// sus::construct::Transmogrify<Integer, Float> trait.
template <sus::num::Integer T, sus::num::Float F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    return T(::sus::mog<decltype(std::declval<T>().primitive_value)>(
        from.primitive_value));
  }
};

// sus::construct::Transmogrify<Integer, PrimitiveFloat> trait.
template <sus::num::Integer T, sus::num::PrimitiveFloat F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    return T(::sus::mog<decltype(std::declval<T>().primitive_value)>(from));
  }
};

// ## === Into `PrimitiveInteger`

// sus::construct::Transmogrify<PrimitiveInteger, Integer> trait.
template <sus::num::PrimitiveInteger T, sus::num::Integer F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    return ::sus::mog<T>(from.primitive_value);
  }
};

// sus::construct::Transmogrify<PrimitiveInteger, PrimitiveInteger> trait.
template <sus::num::PrimitiveInteger T, sus::num::PrimitiveInteger F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    return static_cast<T>(from);
  }
};

// sus::construct::Transmogrify<PrimitiveInteger, PrimitiveEnum> trait.
template <sus::num::PrimitiveInteger T, sus::num::PrimitiveEnum F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    return static_cast<T>(static_cast<std::underlying_type_t<F>>(from));
  }
};

// sus::construct::Transmogrify<PrimitiveInteger, PrimitiveEnumClass> trait.
template <sus::num::PrimitiveInteger T, sus::num::PrimitiveEnumClass F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    return static_cast<T>(static_cast<std::underlying_type_t<F>>(from));
  }
};

// sus::construct::Transmogrify<PrimitiveInteger, Float> trait.
template <sus::num::PrimitiveInteger T, sus::num::Float F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    return ::sus::mog<T>(from.primitive_value);
  }
};

// sus::construct::Transmogrify<PrimitiveInteger, PrimitiveFloat> trait.
template <sus::num::PrimitiveInteger T, sus::num::PrimitiveFloat F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    if (::sus::num::__private::float_is_nan(from)) [[unlikely]]
      return T(0);
    if (from > ::sus::num::__private::max_int_float<T, F>()) [[unlikely]]
      return ::sus::num::__private::max_value<T>();
    if (from < ::sus::num::__private::min_int_float<T, F>()) [[unlikely]]
      return ::sus::num::__private::min_value<T>();
    // SAFETY: The conversion from a float to an integer is UB if the value is
    // out of range for the integer. We have checked above that it is not out of
    // range by comparing with the floating point representation of the
    // integer's min/max values.
    return static_cast<T>(from);
  }
};

// ## === Into `PrimitiveEnum`

// sus::construct::Transmogrify<PrimitiveEnum, Integer> trait.
template <sus::num::PrimitiveEnum T, sus::num::Integer F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    return static_cast<T>(::sus::mog<std::underlying_type_t<T>>(from));
  }
};

// sus::construct::Transmogrify<PrimitiveEnum, PrimitiveInteger> trait.
template <sus::num::PrimitiveEnum T, sus::num::PrimitiveInteger F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    return static_cast<T>(::sus::mog<std::underlying_type_t<T>>(from));
  }
};

// sus::construct::Transmogrify<PrimitiveEnum, PrimitiveEnum> trait.
template <sus::num::PrimitiveEnum T, sus::num::PrimitiveEnum F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    return static_cast<T>(::sus::mog<std::underlying_type_t<T>>(
        static_cast<std::underlying_type_t<F>>(from)));
  }
};

// sus::construct::Transmogrify<PrimitiveEnum, PrimitiveEnumClass> trait.
template <sus::num::PrimitiveEnum T, sus::num::PrimitiveEnumClass F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    return static_cast<T>(::sus::mog<std::underlying_type_t<T>>(
        static_cast<std::underlying_type_t<F>>(from)));
  }
};

// ## === Into `PrimitiveEnumClass`

// sus::construct::Transmogrify<PrimitiveEnumClass, Integer> trait.
template <sus::num::PrimitiveEnumClass T, sus::num::Integer F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    return static_cast<T>(::sus::mog<std::underlying_type_t<T>>(from));
  }
};

// sus::construct::Transmogrify<PrimitiveEnumClass, PrimitiveInteger> trait.
template <sus::num::PrimitiveEnumClass T, sus::num::PrimitiveInteger F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    return static_cast<T>(::sus::mog<std::underlying_type_t<T>>(from));
  }
};

// sus::construct::Transmogrify<PrimitiveEnumClass, PrimitiveEnum> trait.
template <sus::num::PrimitiveEnumClass T, sus::num::PrimitiveEnum F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    return static_cast<T>(::sus::mog<std::underlying_type_t<T>>(
        static_cast<std::underlying_type_t<F>>(from)));
  }
};

// sus::construct::Transmogrify<PrimitiveEnumClass, PrimitiveEnumClass> trait.
template <sus::num::PrimitiveEnumClass T, sus::num::PrimitiveEnumClass F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    return static_cast<T>(::sus::mog<std::underlying_type_t<T>>(
        static_cast<std::underlying_type_t<F>>(from)));
  }
};

// ## === Into `Float`

// sus::construct::Transmogrify<Float, Integer> trait.
template <sus::num::Float T, sus::num::Integer F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    return T(::sus::mog<decltype(std::declval<T>().primitive_value)>(
        from.primitive_value));
  }
};

// sus::construct::Transmogrify<Float, PrimitiveInteger> trait.
template <sus::num::Float T, sus::num::PrimitiveInteger F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    return T(::sus::mog<decltype(std::declval<T>().primitive_value)>(from));
  }
};

// sus::construct::Transmogrify<Float, Float> trait.
template <sus::num::Float T, sus::num::Float F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    return T(::sus::mog<decltype(std::declval<T>().primitive_value)>(
        from.primitive_value));
  }
};

// sus::construct::Transmogrify<Float, PrimitiveFloat> trait.
template <sus::num::Float T, sus::num::PrimitiveFloat F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    return T(::sus::mog<decltype(std::declval<T>().primitive_value)>(from));
  }
};

// ## === Into `PrimitiveFloat`

// sus::construct::Transmogrify<PrimitiveFloat, Integer> trait.
template <sus::num::PrimitiveFloat T, sus::num::Integer F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    return ::sus::mog<T>(from.primitive_value);
  }
};

// sus::construct::Transmogrify<PrimitiveFloat, PrimitiveInteger> trait.
template <sus::num::PrimitiveFloat T, sus::num::PrimitiveInteger F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    return ::sus::num::__private::static_cast_int_to_float<float>(from);
  }
};

// sus::construct::Transmogrify<PrimitiveFloat, Float> trait.
template <sus::num::PrimitiveFloat T, sus::num::Float F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    return ::sus::mog<T>(from.primitive_value);
  }
};

// sus::construct::Transmogrify<PrimitiveFloat, PrimitiveFloat> trait.
template <sus::num::PrimitiveFloat T, sus::num::PrimitiveFloat F>
struct sus::construct::TransmogrifyImpl<T, F> {
  constexpr static T mog_from(const F& from) noexcept {
    static_assert(::sus::mem::size_of<T>() == 4u ||
                  ::sus::mem::size_of<T>() == 8u);
    static_assert(::sus::mem::size_of<F>() == 4u ||
                  ::sus::mem::size_of<F>() == 8u);

    if constexpr (::sus::mem::size_of<T>() == 4u) {
      if constexpr (::sus::mem::size_of<F>() == 4u) {
        return from;
      } else {
        return ::sus::num::__private::static_cast_to_smaller_float<float>(from);
      }
    } else {
      if constexpr (::sus::mem::size_of<F>() == 4u) {
        // C++20 Section 7.3.7: A prvalue of type float can be converted to a
        // prvalue of type double. The value is unchanged.
        return T{from};
      } else {
        return from;
      }
    }
  }
};
