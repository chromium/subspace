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

#include <concepts>

namespace sus::construct {

/// Specializing this class for `To` and `From` allows `To` to satisfy
/// `ToBits<To, From>`.
///
/// # Examples
///
/// To allow bitwise conversion to `Goat` from any type satisying a
/// concept `GoatLike`:
/// ```cpp
/// // Satisfies ToBits<Goat, GoatLike>.
/// template <class Goat, GoatLike G>
/// struct ToBitsImpl<Goat, G> {
///   constexpr static Goat from_bits(const G& g) noexcept { return ...; }
/// };
/// ```
///
/// To receive something that can be bitwise converted to an `u32`.
/// ```cpp
/// auto add = [](u32 a, const sus::construct::ToBits<u32> auto& b) -> u32 {
///   return a.wrapping_add(sus::to_bits<u32>(b));
/// };
/// sus::check(add(3_u32, -1_i32) == u32::MIN + 2);
/// ```
template <class To, class From>
struct ToBitsImpl;

// sus::construct::ToBits<T, T> trait for identity conversion.
template <class T>
struct ToBitsImpl<T, T> {
  constexpr static T from_bits(const T& from) noexcept { return from; }
};

/// A type `T` that satisfies `ToBits<T, F>` can be constructed from `F` through
/// a conversion that will always succeed in producing _some_ value, but may be
/// lossy or produce a value with a different meaning. The conversion may
/// truncate or extend `F` in order to do the conversion to `T`.
///
/// For numeric and primitive types, this provides a mechanism like
/// `static_cast<T>` but it is much safer than `static_cast<T>` as it has
/// defined behaviour for all inputs:
///
/// * Casting from a float to an integer will perform a static_cast, which
///   rounds the float towards zero, except:
///   * `NAN` will return 0.
///   * Values larger than the maximum integer value, including `f32::INFINITY`,
///     will saturate to the maximum value of the integer type.
///   * Values smaller than the minimum integer value, including
///     `f32::NEG_INFINITY`, will saturate to the minimum value of the integer
///     type.
/// * Casting from an integer to a float will perform a static_cast, which
///   converts to the nearest floating point value. The rounding direction for
///   values that land between representable floating point values is
///   implementation defined (per C++20 Section 7.3.10).
/// * Casting from an `f32` (or `float`) to an `f64` (or `double`) preserves the
///   value unchanged.
/// * Casting from an `f64` (or `double`) to an `f32` (or float) performs the
///   same action as a static_cast if the value is in range for f32, otherwise:
///   * `NAN` will return a `NAN`.
///   * Values outside of f32's range will return `f32::INFINITY` or
///     `f32::NEG_INFINITY` for positive and negative values respectively.
/// * Casting to and from `std::byte` produces the same values as casting to and
///   from `u8`.
///
/// These conversions are all defined in `subspace/num/types.h`.
template <class To, class From>
concept ToBits = requires(const From& from) {
  {
    ::sus::construct::ToBitsImpl<To, From>::from_bits(from)
  } noexcept -> std::same_as<To>;
};

/// An infallible conversion that may lose the original value in the process. If
/// the input can not be represented in the output, some other value will be
/// produced, which may lead to application bugs and memory unsafety if used
/// incorrectly.
///
/// To convert between types while ensuring the values are preserved, use
/// `sus::construct::Into` or `sus::construct::TryInto`. Usually prefer using
/// `sus::into(x)` or `sus::try_into(x)` over `sus::to_bits<Y>(x)` as most code
/// should preserve values across type transitions.
///
/// See `AsBits` for how numeric and primitive values are converted.
///
/// # Examples
///
/// This converts `-1_i64` into a `u32`, which both changes its meaning,
/// becoming a large positive number, and truncates the high 32 bits, losing the
/// original.
/// ```cpp
/// sus::check(u32::MAX == sus::to_bits<u32>(-1_i64));
/// ```
template <class To, class From>
  requires(ToBits<To, From>)
constexpr inline To to_bits(const From& from) {
  return ToBitsImpl<To, From>::from_bits(from);
}

}  // namespace sus::construct

// Bring the to_bits() function into the `sus` namespace.
namespace sus {
using ::sus::construct::to_bits;
}
