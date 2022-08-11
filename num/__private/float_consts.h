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

#define _sus__float_consts(T, Suffix)                        \
  struct consts {                                            \
    /** Euler's number (e) */                                \
    static constexpr inline auto E() noexcept {              \
      return T(2.718281828459045235360287471352662##Suffix); \
    }                                                        \
    /** 1/π */                                              \
    static constexpr inline auto FRAC_1_PI() noexcept {      \
      return T(0.318309886183790671537767526745028##Suffix); \
    }                                                        \
    /** 1/sqrt(2) */                                         \
    static constexpr inline auto FRAC_1_SQRT_2() noexcept {  \
      return T(0.7071067811865476##Suffix);                  \
    }                                                        \
    /** 2/π */                                              \
    static constexpr inline auto FRAC_2_PI() noexcept {      \
      return T(0.6366197723675814##Suffix);                  \
    }                                                        \
    /** 2/sqrt(π) */                                        \
    static constexpr inline auto FRAC_2_SQRT_PI() noexcept { \
      return T(1.1283791670955126##Suffix);                  \
    }                                                        \
    /** π/2 */                                              \
    static constexpr inline auto FRAC_PI_2() noexcept {      \
      return T(1.5707963267948966##Suffix);                  \
    }                                                        \
    /** π/3 */                                              \
    static constexpr inline auto FRAC_PI_3() noexcept {      \
      return T(1.0471975511965979##Suffix);                  \
    }                                                        \
    /** π/4 */                                              \
    static constexpr inline auto FRAC_PI_4() noexcept {      \
      return T(0.7853981633974483##Suffix);                  \
    }                                                        \
    /** π/6 */                                              \
    static constexpr inline auto FRAC_PI_6() noexcept {      \
      return T(0.5235987755982989##Suffix);                  \
    }                                                        \
    /** π/8 */                                              \
    static constexpr inline auto FRAC_PI_8() noexcept {      \
      return T(0.39269908169872414##Suffix);                 \
    }                                                        \
    /** ln(2) */                                             \
    static constexpr inline auto LN_2() noexcept {           \
      return T(0.693147180559945309417232121458176##Suffix); \
    }                                                        \
    /** ln(10) */                                            \
    static constexpr inline auto LN_10() noexcept {          \
      return T(2.302585092994045684017991454684364##Suffix); \
    }                                                        \
    /** log_2(10) */                                         \
    static constexpr inline auto LOG2_10() noexcept {        \
      return T(3.321928094887362##Suffix);                   \
    }                                                        \
    /** log_2(e) */                                          \
    static constexpr inline auto LOG2_E() noexcept {         \
      return T(1.4426950408889634##Suffix);                  \
    }                                                        \
    /** log_10(2) */                                         \
    static constexpr inline auto LOG10_2() noexcept {        \
      return T(0.3010299956639812##Suffix);                  \
    }                                                        \
    /** log_10(e) */                                         \
    static constexpr inline auto LOG10_E() noexcept {        \
      return T(0.4342944819032518##Suffix);                  \
    }                                                        \
    /** Archimedes' constant (π) */                         \
    static constexpr inline auto PI() noexcept {             \
      return T(3.141592653589793238462643383279502##Suffix); \
    }                                                        \
    /** sqrt(2) */                                           \
    static constexpr inline auto SQRT_2() noexcept {         \
      return T(1.414213562373095048801688724209698##Suffix); \
    }                                                        \
    /** The full circle constant (τ) */                     \
    static constexpr inline auto TAU() noexcept {            \
      return T(6.283185307179586##Suffix);                   \
    }                                                        \
  };                                                         \
  static_assert(true)
