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

#if _MSC_VER
#include "sus/assertions/check.h"
#endif

#if _MSC_VER && !defined(__clang__)
/// Literal integer value.
#define _sus__integer_literal(Name, T)                                                               \
  /* A `constexpr` workaround for MSVC bug that doesn't constant-evaluate                            \
   * user-defined literals in all cases:                                                             \
   * https://developercommunity.visualstudio.com/t/User-defined-literals-not-constant-expre/10108165 \
   *                                                                                                 \
   * This obviously adds some runtime + codegen overhead. We could use a                             \
   * "numeric literal operator template" and construct the number from a                             \
   * `char...` template, which is what we used to do before moving to                                \
   * `consteval` (see commit 531ac278f5b96a63b39332a0b87ef207e0d40575).                              \
   * However that triggers a different MSVC bug when used with any                                   \
   * unary/binary operator in a templated function:                                                  \
   * https://developercommunity.visualstudio.com/t/MSVC-Compiler-bug-with:-numeric-literal/10108160  \
   */                                                                                                \
  T inline constexpr operator""_##Name(unsigned long long val) noexcept {                            \
    ::sus::check(val <= static_cast<unsigned long long>(T::MAX_PRIMITIVE));                          \
    return T(static_cast<decltype(T::primitive_value)>(val));                                        \
  }
#else
/// Literal integer value.
#define _sus__integer_literal(Name, T)                           \
  T inline consteval operator""_##Name(unsigned long long val) { \
    if (val > static_cast<unsigned long long>(T::MAX_PRIMITIVE)) \
      throw "Integer literal out of bounds for ##T##";           \
    return T(static_cast<decltype(T::primitive_value)>(val));    \
  }
#endif

#if _MSC_VER
/// Literal float value.
#define _sus__float_literal(Name, T)                                                                 \
  /* A `constexpr` workaround for MSVC bug that doesn't constant-evaluate                            \
   * user-defined literals in all cases:                                                             \
   * https://developercommunity.visualstudio.com/t/User-defined-literals-not-constant-expre/10108165 \
   *                                                                                                 \
   * This obviously adds some runtime + codegen overhead. We could use a                             \
   * "numeric literal operator template" and construct the number from a                             \
   * `char...` template, which is what we used to do before moving to                                \
   * `consteval` (see commit 531ac278f5b96a63b39332a0b87ef207e0d40575).                              \
   * However that triggers a different MSVC bug when used with any                                   \
   * unary/binary operator in a templated function:                                                  \
   * https://developercommunity.visualstudio.com/t/MSVC-Compiler-bug-with:-numeric-literal/10108160  \
   */                                                                                                \
  T inline constexpr operator""_##Name(long double val) noexcept {                                   \
    ::sus::check(val <= static_cast<long double>(T::MAX_PRIMITIVE));                                 \
    return T(static_cast<decltype(T::primitive_value)>(val));                                        \
  }                                                                                                  \
  T inline constexpr operator""_##Name(unsigned long long val) noexcept {                            \
    return T(static_cast<decltype(T::primitive_value)>(val));                                        \
  }

#else
/// Literal float value.
#define _sus__float_literal(Name, T)                             \
  T inline consteval operator""_##Name(long double val) {        \
    if (val > static_cast<long double>(T::MAX_PRIMITIVE))        \
      throw "Float literal out of bounds for ##T##";             \
    return T(static_cast<decltype(T::primitive_value)>(val));    \
  }                                                              \
  T inline consteval operator""_##Name(unsigned long long val) { \
    return T(static_cast<decltype(T::primitive_value)>(val));    \
  }
#endif
