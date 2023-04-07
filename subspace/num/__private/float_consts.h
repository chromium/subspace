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

#define _sus__float_consts_struct(T)       \
  struct consts {                        \
    /** Euler's number (e) */            \
    static const T E;                \
    /** 1/π */                          \
    static const T FRAC_1_PI;        \
    /** 1/sqrt(2) */                     \
    static const T FRAC_1_SQRT_2;    \
    /** 2/π */                          \
    static const T FRAC_2_PI;        \
    /** 2/sqrt(π) */                    \
    static const T FRAC_2_SQRT_PI;   \
    /** π/2 */                          \
    static const T FRAC_PI_2;        \
    /** π/3 */                          \
    static const T FRAC_PI_3;        \
    /** π/4 */                          \
    static const T FRAC_PI_4;        \
    /** π/6 */                          \
    static const T FRAC_PI_6;        \
    /** π/8 */                          \
    static const T FRAC_PI_8;        \
    /** ln(2) */                         \
    static const T LN_2;             \
    /** ln(10) */                        \
    static const T LN_10;            \
    /** log_2(10) */                     \
    static const T LOG2_10;          \
    /** log_2(e) */                      \
    static const T LOG2_E;           \
    /** log_10(2) */                     \
    static const T LOG10_2;          \
    /** log_10(e) */                     \
    static const T LOG10_E;          \
    /** Archimedes' constant (π) */     \
    static const T PI;               \
    /** sqrt(2) */                       \
    static const T SQRT_2;           \
    /** The full circle constant (τ) */ \
    static const T TAU;              \
  };                                     \
  static_assert(true)

#define _sus__float_consts_struct_out_of_line(T, Suffix)                                     \
  inline constexpr T T::consts::E =                                            \
      T(2.718281828459045235360287471352662##Suffix);                          \
  inline constexpr T T::consts::FRAC_1_PI =                                    \
      T(0.318309886183790671537767526745028##Suffix);                          \
  inline constexpr T T::consts::FRAC_1_SQRT_2 = T(0.7071067811865476##Suffix); \
  inline constexpr T T::consts::FRAC_2_PI = T(0.6366197723675814##Suffix);     \
  inline constexpr T T::consts::FRAC_2_SQRT_PI =                               \
      T(1.1283791670955126##Suffix);                                           \
  inline constexpr T T::consts::FRAC_PI_2 = T(1.5707963267948966##Suffix);     \
  inline constexpr T T::consts::FRAC_PI_3 = T(1.0471975511965979##Suffix);     \
  inline constexpr T T::consts::FRAC_PI_4 = T(0.7853981633974483##Suffix);     \
  inline constexpr T T::consts::FRAC_PI_6 = T(0.5235987755982989##Suffix);     \
  inline constexpr T T::consts::FRAC_PI_8 = T(0.39269908169872414##Suffix);    \
  inline constexpr T T::consts::LN_2 =                                         \
      T(0.693147180559945309417232121458176##Suffix);                          \
  inline constexpr T T::consts::LN_10 =                                        \
      T(2.302585092994045684017991454684364##Suffix);                          \
  inline constexpr T T::consts::LOG2_10 = T(3.321928094887362##Suffix);        \
  inline constexpr T T::consts::LOG2_E = T(1.4426950408889634##Suffix);        \
  inline constexpr T T::consts::LOG10_2 = T(0.3010299956639812##Suffix);       \
  inline constexpr T T::consts::LOG10_E = T(0.4342944819032518##Suffix);       \
  inline constexpr T T::consts::PI =                                           \
      T(3.141592653589793238462643383279502##Suffix);                          \
  inline constexpr T T::consts::SQRT_2 =                                       \
      T(1.414213562373095048801688724209698##Suffix);                          \
  inline constexpr T T::consts::TAU = T(6.283185307179586##Suffix);            \
  static_assert(true)
