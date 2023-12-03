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

#include "sus/mem/copy.h"

namespace sus::construct {

/// Specializing this class for `To` and `From` allows `To` to satisfy
/// `Cast<To, From>`.
///
/// # Examples
///
/// To allow lossy type conversion to `Goat` from any type satisying a
/// concept `GoatLike`:
/// ```cpp
/// // Satisfies Cast<Goat, GoatLike>.
/// template <class Goat, GoatLike G>
/// struct CastImpl<Goat, G> {
///   constexpr static Goat cast_from(const G& g) noexcept { return ...; }
/// };
/// ```
///
/// To receive something that can be lossily converted to a `u32`.
/// ```cpp
/// auto add = [](u32 a, const sus::construct::Cast<u32> auto& b) -> u32
/// {
///   return a.wrapping_add(sus::cast<u32>(b));
/// };
/// sus::check(add(3_u32, -'1_i32) == u32::MIN + 2);
/// ```
template <class To, class From>
struct CastImpl;

// sus::construct::Cast<T, T> trait for identity conversion when `T` is
// `Copy`.
template <::sus::mem::Copy T>
struct CastImpl<T, T> {
  constexpr static T cast_from(const T& from) noexcept { return from; }
};

/// When a pair of types `T` and `F` satisfy `Cast<T, F>`, it means that
/// `F` can be cast to `T` through a conversion that will always succeed in
/// producing _some_ value, but may be lossy or produce a value with a
/// different meaning. The conversion may truncate or extend `F` in order to do
/// the conversion to `T`.
///
/// This operation is commonly known as type casting or type coercion. The
/// conversion to `T` can be done by calling
/// [`sus::cast<T>(from)`]($sus::construct::cast).
///
/// The conversion is defined for the identity conversion where both the input
/// and output are the same type, if the type is [`Copy`]($sus::mem::Copy), in
/// which case the input is copied and returned.
/// As casting is meant to be a cheap conversion, primarily for moving between
/// primitive types, it does not support
/// [`Clone`]($sus::mem::Clone) types, and [`Into`]($sus::construct::Into)
/// should be used in more complex cases.
///
/// # Casting numeric types
///
/// For numeric and primitive types, `Cast` is defined to provide a
/// mechanism like `static_cast<T>` but it is much safer than `static_cast<T>`
/// as it has defined behaviour for all inputs:
///
/// * Casting from a float to an integer will perform a static_cast, which
///   rounds the float towards zero, except:
///   * `NAN` will return 0.
///   * Values larger than the maximum integer value, including
///     [`f32::INFINITY`]($sus::num::f32::INFINITY),
///     will saturate to the maximum value of the integer type.
///   * Values smaller than the minimum integer value, including
///     [`f32::NEG_INFINITY`]($sus::num::f32::NEG_INFINITY), will saturate to
///     the minimum value of the integer type.
/// * Casting from an integer to a float will perform a `static_cast`, which
///   converts to the nearest floating point value. The rounding direction for
///   values that land between representable floating point values is
///   implementation defined (per C++20 Section 7.3.10).
/// * Casting from an [`f32`]($sus::num::f32) (or `float`) to an
///   [`f64`]($sus::num::f64) (or `double`) preserves the
///   value unchanged.
/// * Casting from an [`f64`]($sus::num::f64) (or `double`) to an
///   [`f32`]($sus::num::f32) (or float) performs the same action as a
///   `static_cast` if the value is in range for [`f32`]($sus::num::f32),
///   otherwise:
///   * `NAN` will return a `NAN`.
///   * Values outside of [`f32`]($sus::num::f32)'s range will return
///     [`f32::INFINITY`]($sus::num::f32::INFINITY) or
///     [`f32::NEG_INFINITY`]($sus::num::f32::NEG_INFINITY) for positive and
///     negative values respectively.
/// * Casting to and from [`std::byte`](
///   https://en.cppreference.com/w/cpp/types/byte) produces the same values
///   as casting to and from [`u8`]($sus::num::u8).
///
/// These conversions are all defined in `sus/num/types.h`.
///
/// # Extending to other types
///
/// Types can participate in defining their [`Cast`]($sus::construct::Cast)
/// strategy by providing a specialization of
/// `sus::convert::CastImpl<To, From>`.
/// The conversions should always produce a value of type `T`, should not panic,
/// and should not cause Undefined Behaviour.
///
/// The `CastImpl` specialization needs a (typically constexpr) static method
/// `cast_from` that receives `const From&` and returns `To`.
template <class To, class From>
concept Cast = requires(const From& from) {
  {
    ::sus::construct::CastImpl<
        std::remove_const_t<To>, std::remove_const_t<From>>::cast_from(from)
  } noexcept -> std::same_as<std::remove_const_t<To>>;
};

/// An infallible conversion (cast) that may lose the original
/// value in the process. If the input can not be represented in the output,
/// some other value will be produced, which may lead to application bugs and
/// memory unsafety if used incorrectly. This behaves like `static_cast<To>()`
/// but without Undefined Behaviour.
///
/// The [`cast`]($sus::construct::cast) operation is supported for types
/// `To` and `From` that satisfy [`Cast<To, From>`](
/// $sus::construct::Cast).
///
/// Usually prefer to convert between types with the value-preserving methods
/// of [`From`]($sus::construct::From) and
/// [`Into`]($sus::construct::Into) and [`TryInto`]($sus::construct::TryInto)
/// when possible. [`Cast`]($sus::construct::Cast) is required for converting
/// from floating point to integer values, and from larger integer types to
/// floating point, as these are lossy conversions.
///
/// | Concept | Usage | Infallible | Preserves values |
/// | ------- | ----- | ---------- | ---------------- |
/// | [`From`]($sus::construct::From) / [`Into`]($sus::construct::Into) | `T::from(x)` / [`sus::into(x)`]($sus::construct::into) | ✅ | ✅ |
/// | [`TryInto`]($sus::construct::TryInto) | [`sus::try_into<T>(x)`]($sus::construct::try_into) | ❌ | ✅ |
/// | [`Cast`]($sus::construct::Cast) | `sus::cast<T>(x)` | ✅ | ❌ |
///
/// See [`Cast`]($sus::construct::Cast) for how numeric and
/// primitive values are converted.
///
/// It is best practive to place a `// SAFETY:` comment on use of [`sus::cast`](
/// $sus::construct::cast) in order to explain why the code intends to change
/// the value during the cast.
///
/// # Examples
///
/// This converts `-1_i64` into a `u32`, which both changes its meaning,
/// becoming a large positive number, and truncates the high 32 bits, losing the
/// original bits.
/// ```cpp
/// // SAFETY: We're intending to convert negative numbers into large positive
/// // values for this example.
/// sus::check(u32::MAX == sus::cast<u32>(-1_i64));
/// ```
template <class To, class From>
  requires(Cast<To, From>)
constexpr inline To cast(const From& from) {
  return CastImpl<To, From>::cast_from(from);
}

}  // namespace sus::construct

// Bring the cast() function into the `sus` namespace.
namespace sus {
using ::sus::construct::cast;
}
