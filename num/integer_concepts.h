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

template <class T>
concept Unsigned =
    std::same_as<u8, std::decay_t<T>> || std::same_as<u16, std::decay_t<T>> ||
    std::same_as<u32, std::decay_t<T>> || std::same_as<u64, std::decay_t<T>> ||
    std::same_as<usize, std::decay_t<T>>;

template <class T>
concept Signed =
    std::same_as<i8, std::decay_t<T>> || std::same_as<i16, std::decay_t<T>> ||
    std::same_as<i32, std::decay_t<T>> || std::same_as<i64, std::decay_t<T>> ||
    std::same_as<isize, std::decay_t<T>>;

template <class T>
concept Integer = Unsigned<T> || Signed<T>;

template <class T>
concept UnsignedPrimitiveInteger =
    std::same_as<size_t, T> ||
    (std::is_unsigned_v<char> && std::same_as<char, T>) ||
    std::same_as<unsigned char, T> || std::same_as<unsigned short, T> ||
    std::same_as<unsigned int, T> || std::same_as<unsigned long, T> ||
    std::same_as<unsigned long long, T>;

template <class T>
concept SignedPrimitiveInteger =
    (!std::is_unsigned_v<char> && std::same_as<char, T>) ||
    std::same_as<signed char, T> || std::same_as<short, T> ||
    std::same_as<int, T> || std::same_as<long, T> || std::same_as<long long, T>;

template <class T>
concept PrimitiveInteger =
    UnsignedPrimitiveInteger<T> || SignedPrimitiveInteger<T>;

}  // namespace sus::num
