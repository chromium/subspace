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

#include <stdint.h>

#include <concepts>

namespace sus::num::__private {

template <class T>
struct OutOfBounds final {
  T t;
};

template <class T, bool start, unsigned long long radix, unsigned long long max,
          unsigned long long val, char... C>
struct BuildIntegerImpl;

// Hex: 0x prefix.
template <class T, unsigned long long radix, unsigned long long max, char... C>
struct BuildIntegerImpl<T, true, radix, max, 0, '0', 'x', C...> final {
  using _builder = BuildIntegerImpl<T, false, 16, max, 0, C...>;
  using type = _builder::type;
  static constexpr type value = _builder::value;
};
template <class T, unsigned long long radix, unsigned long long max, char... C>
struct BuildIntegerImpl<T, true, radix, max, 0, '0', 'X', C...> final {
  using _builder = BuildIntegerImpl<T, false, 16, max, 0, C...>;
  using type = _builder::type;
  static constexpr type value = _builder::value;
};

// Binary: 0b prefix.
template <class T, unsigned long long radix, unsigned long long max, char... C>
struct BuildIntegerImpl<T, true, radix, max, 0, '0', 'b', C...> final {
  using _builder = BuildIntegerImpl<T, false, 2, max, 0, C...>;
  using type = _builder::type;
  static constexpr type value = _builder::value;
};
template <class T, unsigned long long radix, unsigned long long max, char... C>
struct BuildIntegerImpl<T, true, radix, max, 0, '0', 'B', C...> final {
  using _builder = BuildIntegerImpl<T, false, 2, max, 0, C...>;
  using type = _builder::type;
  static constexpr type value = _builder::value;
};

// Octal: 0 prefix.
template <class T, unsigned long long radix, unsigned long long max, char... C>
struct BuildIntegerImpl<T, true, radix, max, 0, '0', C...> final {
  using _builder = BuildIntegerImpl<T, false, 8, max, 0, C...>;
  using type = _builder::type;
  static constexpr type value = _builder::value;
};

template <class T, unsigned long long radix, unsigned long long max,
          unsigned long long val, char... C>
struct BuildIntegerImpl<T, false, radix, max, val, '\'', C...> final {
  using _builder = BuildIntegerImpl<T, false, radix, max, val, C...>;
  using type = _builder::type;
  static constexpr type value = _builder::value;
};

template <class T, bool start, unsigned long long radix, unsigned long long max,
          unsigned long long val, char c, char... C>
struct BuildIntegerImpl<T, start, radix, max, val, c, C...> final {
  static constexpr auto digit =
      (c >= '0' && c <= '9'
           ? c - '0'
           : (c >= 'a' && c <= 'f'
                  ? 10 + c - 'a'
                  : (c >= 'A' && c <= 'F' ? 10 + c - 'A' : radix)));
  // We don't have to check that digits are in range because the compiler does
  // that for us.

  static constexpr bool valid = val * radix + digit <= max;
  using _builder =
      BuildIntegerImpl<std::conditional_t<valid, T, OutOfBounds<T>>, false,
                       radix, max, val * radix + digit, C...>;
  using type = _builder::type;
  static constexpr type value = _builder::value;
};

template <class T, bool start, unsigned long long radix, unsigned long long max,
          unsigned long long val>
struct BuildIntegerImpl<T, start, radix, max, val> final {
  using type = T;
  static constexpr type value = static_cast<type>(val);
};

template <class T, unsigned long long max, char... C>
struct BuildInteger final {
  using _builder = BuildIntegerImpl<T, true, 10, max, 0, C...>;
  using type = _builder::type;
  static constexpr type value = _builder::value;
};

}  // namespace sus::num::__private
