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

// IWYU pragma: private, include "sus/num/types.h"
// IWYU pragma: friend "sus/.*"
#pragma once

#include <stddef.h>

#include <concepts>

#include "sus/lib/__private/forward_decl.h"

namespace sus::num {

/// Unsigned Subspace numeric integer types. All of [`Unsigned`](
/// $sus::num::Unsigned) but excluding [`uptr`]($sus::num::uptr).
///
/// The [`uptr`]($sus::num::uptr) type is an integer but has a different API
/// that restricts how its used compared to other integer types. This can be
/// used to exclude it when it does not fit with a use case.
template <class T>
concept UnsignedNumeric =
    std::same_as<u8, T> || std::same_as<u16, T> || std::same_as<u32, T> ||
    std::same_as<u64, T> || std::same_as<usize, T>;

/// Unsigned Subspace pointer integer types. This is the rest of [`Unsigned`](
/// $sus::num::Unsigned) that is not included in [`UnsignedNumeric`](
/// $sus::num::UnsignedNumeric), which is just [`uptr`]($sus::num::uptr).
///
/// The [`uptr`]($sus::num::uptr) type is an integer but has a different API
/// that interacts with pointers in ways numeric integers can not. This can be
/// used to exclude numeric integers when they do not fit with a use case.
template <class T>
concept UnsignedPointer = std::same_as<uptr, T>;

/// Unsigned Subspace integer types: [`u8`]($sus::num::u8),
/// [`u16`]($sus::num::u16), [`u32`]($sus::num::u32), [`u64`]($sus::num::u64),
/// [`usize`]($sus::num::usize), and [`uptr`]($sus::num::uptr).
template <class T>
concept Unsigned = UnsignedNumeric<T> || UnsignedPointer<T>;

/// Signed Subspace integer types: [`i8`]($sus::num::i8),
/// [`i16`]($sus::num::i16), [`i32`]($sus::num::i32), [`i64`]($sus::num::i64),
/// and [`isize`]($sus::num::isize).
template <class T>
concept Signed =
    std::same_as<i8, T> || std::same_as<i16, T> || std::same_as<i32, T> ||
    std::same_as<i64, T> || std::same_as<isize, T>;

/// All Subspace numeric integer types. This includes all safe integer types
/// except [`uptr`]($sus::num::uptr) which represents pointers. See
/// [`UnsignedNumeric`]($sus::num::UnsignedNumeric) and
/// [`UnsignedPointer`]($sus::num::UnsignedPointer).
template <class T>
concept IntegerNumeric = UnsignedNumeric<T> || Signed<T>;

/// All Subspace pointer integer types. This includes safe integer types that
/// represent pointer values, which is just [`uptr`]($sus::num::uptr). See
/// [`UnsignedNumeric`]($sus::num::UnsignedNumeric) and
/// [`UnsignedPointer`]($sus::num::UnsignedPointer).
template <class T>
concept IntegerPointer = UnsignedPointer<T>;

/// Signed or unsigned Subspace integer types: [`u8`]($sus::num::u8),
/// [`u16`]($sus::num::u16), [`u32`]($sus::num::u32), [`u64`]($sus::num::u64),
/// [`usize`]($sus::num::usize), [`uptr`]($sus::num::uptr),
/// [`i8`]($sus::num::i8), [`i16`]($sus::num::i16), [`i32`]($sus::num::i32),
/// [`i64`]($sus::num::i64), and [`isize`]($sus::num::isize).
template <class T>
concept Integer = IntegerNumeric<T> || IntegerPointer<T>;

/// Unsigned primitive integer types (`unsigned char`, `unsigned int`, etc).
template <class T>
concept UnsignedPrimitiveInteger =
    std::same_as<size_t, T> || std::same_as<uintptr_t, T> ||
    std::same_as<bool, T> ||
    (std::is_unsigned_v<char> && std::same_as<char, T>) ||
#if defined(WIN32)
    std::same_as<wchar_t, T> ||
#endif
    std::same_as<unsigned char, T> || std::same_as<unsigned short, T> ||
    std::same_as<unsigned int, T> || std::same_as<unsigned long, T> ||
    std::same_as<unsigned long long, T>;

/// Signed primitive integer types (`signed char`, `int`, `long`, etc).
template <class T>
concept SignedPrimitiveInteger =
    (!std::is_unsigned_v<char> && std::same_as<char, T>) ||
    std::same_as<signed char, T> || std::same_as<short, T> ||
    std::same_as<int, T> || std::same_as<long, T> || std::same_as<long long, T>;

/// Signed or unsigned primitive integer types (`char`, `int`, `unsigned int`,
/// `unsigned long`, etc).
template <class T>
concept PrimitiveInteger =
    UnsignedPrimitiveInteger<T> || SignedPrimitiveInteger<T>;

/// Enum types that are backed by an unsigned value, excluding `enum class`
/// types.
template <class T>
concept UnsignedPrimitiveEnum =
    std::is_enum_v<T> && UnsignedPrimitiveInteger<std::underlying_type_t<T>> &&
    std::is_convertible_v<T, std::underlying_type_t<T>>;

/// Enum types that are backed by a signed value, excluding `enum class` types.
template <class T>
concept SignedPrimitiveEnum =
    std::is_enum_v<T> && SignedPrimitiveInteger<std::underlying_type_t<T>> &&
    std::is_convertible_v<T, std::underlying_type_t<T>>;

/// Enum types that are backed by a signed or unsigned value, excluding `enum
/// class` types.
template <class T>
concept PrimitiveEnum = UnsignedPrimitiveEnum<T> || SignedPrimitiveEnum<T>;

/// Enum class
/// ([scoped enumeration](https://en.cppreference.com/w/cpp/language/enum))
/// types that are backed by an unsigned value.
template <class T>
concept UnsignedPrimitiveEnumClass =
    !UnsignedPrimitiveEnum<T> && std::is_enum_v<T> &&
    UnsignedPrimitiveInteger<std::underlying_type_t<T>>;

/// Enum class
/// ([scoped enumeration](https://en.cppreference.com/w/cpp/language/enum))
/// types that are backed by a signed value.
template <class T>
concept SignedPrimitiveEnumClass =
    !SignedPrimitiveEnum<T> && std::is_enum_v<T> &&
    SignedPrimitiveInteger<std::underlying_type_t<T>>;

/// Enum class
/// ([scoped enumeration](https://en.cppreference.com/w/cpp/language/enum))
/// types that are backed by a signed or unsigned value.
template <class T>
concept PrimitiveEnumClass =
    UnsignedPrimitiveEnumClass<T> || SignedPrimitiveEnumClass<T>;

}  // namespace sus::num
