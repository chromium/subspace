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
/// `AsBits<To, From>`.
///
/// # Examples
///
/// To allow bitwise conversion to `Goat` from any type satisying a
/// concept `GoatLike`:
/// ```cpp
/// // Satisfies AsBits<Goat, GoatLike>.
/// template <class Goat, GoatLike G>
/// struct AsBitsImpl<Goat, G> {
///   constexpr static Goat from_bits(const G& g) noexcept { return ...; }
/// };
/// ```
///
/// To receive something that can be bitwise converted to an `u32`.
/// ```cpp
/// auto add = [](u32 a, const sus::construct::AsBits<u32> auto& b) -> u32 {
///   return a.wrapping_add(sus::as_bits<u32>(b));
/// };
/// sus::check(add(3_u32, -1_i32) == u32::MIN + 2);
/// ```
template <class To, class From>
struct AsBitsImpl;

/// A type `T` that satisfies `AsBits<T, F>` can be constructed from `F` through
/// a bitwise conversion that preserves the bits but not the conceptual value of
/// `F`. The conversion may also truncate or extend `F` in order to do the
/// bitwise conversion to `T`.
///
/// This provides a similar mechanism to `std::bit_cast<T>` but also allowing
/// conversion to larger or smaller types, like `static_cast<T>`, while allowing
/// types to participate in deciding how and when bitwise converstions can be
/// performed.
template <class To, class From>
concept AsBits = requires(const From& from) {
  {
    ::sus::construct::AsBitsImpl<To, From>::from_bits(from)
  } noexcept -> std::same_as<To>;
};

/// Bitwise conversion from `From` to `To`. The conversion is done in a way
/// that attempts to preserve the bits of `From` without necessarily preserving
/// the value or meaning.
///
/// To convert between types while preserving the meaning of the value, use
/// `sus::construct::Into` or `sus::construct::TryInto`. Usually prefer using
/// `sus::into(x)` or `sus::try_into(x)` over `sus::as_bits<Y>(x)` as most code
/// is not doing bitwise manipulation.
///
/// The result of `as_bits()` may be lossy. It may truncate bits from the input,
/// or extend it.
///
/// # Examples
///
/// This converts `-1_i64` into a `u32`, which changes its meaning, becoming a
/// large positive number, and truncates 32 bits which loses the original value
/// as well.
/// ```cpp
/// sus::check(u32::MAX == sus::as_bits<u32>(-1_i64));
/// ```
template <class To, class From>
constexpr inline To as_bits(const From& from) {
  return AsBitsImpl<To, From>::from_bits(from);
}

}  // namespace sus::construct

// Bring the as_bits() function into the `sus` namespace.
namespace sus {
using ::sus::construct::as_bits;
}
