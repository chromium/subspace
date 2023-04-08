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

#include <stddef.h>

#include <concepts>

namespace sus::num {

struct i8;
struct i16;
struct i32;
struct i64;
struct isize;
struct u8;
struct u16;
struct u32;
struct u64;
struct usize;

/// Unsigned Subspace integer types (u8, u16, u32, etc).
template <class T>
concept Unsigned =
    std::same_as<u8, T> || std::same_as<u16, T> || std::same_as<u32, T> ||
    std::same_as<u64, T> || std::same_as<usize, T>;

/// Signed Subspace integer types (i8, i16, i32, etc).
template <class T>
concept Signed =
    std::same_as<i8, T> || std::same_as<i16, T> || std::same_as<i32, T> ||
    std::same_as<i64, T> || std::same_as<isize, T>;

/// Signed or unsigned Subspace integer types (i8, u16, i32, u64, etc).
template <class T>
concept Integer = Unsigned<T> || Signed<T>;

/// Unsigned primitive integer types (unsigned char, unsigned int, etc).
template <class T>
concept UnsignedPrimitiveInteger =
    std::same_as<size_t, T> ||
    (std::is_unsigned_v<char> && std::same_as<char, T>) ||
    std::same_as<unsigned char, T> || std::same_as<unsigned short, T> ||
    std::same_as<unsigned int, T> || std::same_as<unsigned long, T> ||
    std::same_as<unsigned long long, T>;

/// Signed primitive integer types (char, int, long, etc).
template <class T>
concept SignedPrimitiveInteger =
    (!std::is_unsigned_v<char> && std::same_as<char, T>) ||
    std::same_as<signed char, T> || std::same_as<short, T> ||
    std::same_as<int, T> || std::same_as<long, T> || std::same_as<long long, T>;

/// Signed or unsigned primitive integer types (char, int, unsigned int,
/// unsigned long, etc).
template <class T>
concept PrimitiveInteger =
    UnsignedPrimitiveInteger<T> || SignedPrimitiveInteger<T>;

/// Enum types that are backed by an unsigned value, excluding `enum class`
/// types.
template <class T>
concept UnsignedPrimitiveEnum =
    std::is_enum_v<T> && UnsignedPrimitiveInteger<std::underlying_type_t<T>> &&
    requires(T t) {
      {
        [](std::underlying_type_t<T>) {}(t)
      };
    };

/// Enum types that are backed by a signed value, excluding `enum class` types.
template <class T>
concept SignedPrimitiveEnum =
    std::is_enum_v<T> && SignedPrimitiveInteger<std::underlying_type_t<T>> &&
    requires(T t) {
      {
        [](std::underlying_type_t<T>) {}(t)
      };
    };

/// Enum types that are backed by a signed or unsigned value, excluding `enum
/// class` types.
template <class T>
concept PrimitiveEnum = UnsignedPrimitiveEnum<T> || SignedPrimitiveEnum<T>;

/// Enum class (scoped enum) types that are backed by an unsigned value.
template <class T>
concept UnsignedPrimitiveEnumClass =
    !UnsignedPrimitiveEnum<T> && std::is_enum_v<T> &&
    UnsignedPrimitiveInteger<std::underlying_type_t<T>>;

/// Enum class (scoped enum) types that are backed by a signed value.
template <class T>
concept SignedPrimitiveEnumClass =
    !SignedPrimitiveEnum<T> && std::is_enum_v<T> &&
    SignedPrimitiveInteger<std::underlying_type_t<T>>;

/// Enum class (scoped enum) types that are backed by a signed or unsigned
/// value.
template <class T>
concept PrimitiveEnumClass =
    UnsignedPrimitiveEnumClass<T> || SignedPrimitiveEnumClass<T>;

}  // namespace sus::num
