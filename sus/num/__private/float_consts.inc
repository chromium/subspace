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

// IWYU pragma: private
// IWYU pragma: friend "sus/.*"

///////////////////////////////////////////////////////////////////////////
//
// Defines constants in a floating point type.
//
// TO USE THIS INC FILE:
//
// Include it beside the class, in the same namespace.
//
// Define `_self` to the name of the floating point type.
// Define `_primitive` to the primitive it holds inside.
// Define `_suffix` to the suffix to put on the end of literals.
///////////////////////////////////////////////////////////////////////////

inline constexpr _primitive _self::MIN_PRIMITIVE =
    __private::min_value<_primitive>();
inline constexpr _primitive _self::MAX_PRIMITIVE =
    __private::max_value<_primitive>();
inline constexpr _self _self::MIN = _self(_self::MIN_PRIMITIVE);
inline constexpr _self _self::MAX = _self(_self::MAX_PRIMITIVE);
inline constexpr u32 _self::RADIX = __private::radix<_primitive>();
inline constexpr u32 _self::MANTISSA_DIGITS =
    __private::num_mantissa_digits<_primitive>();
inline constexpr u32 _self::DIGITS = __private::num_digits<_primitive>();
inline constexpr _self _self::EPSILON = _self(__private::epsilon<_primitive>());
inline constexpr _self _self::MIN_POSITIVE =
    _self(__private::min_positive_value<_primitive>());
inline constexpr i32 _self::MIN_EXP = __private::min_exp<_primitive>();
inline constexpr i32 _self::MAX_EXP = __private::max_exp<_primitive>();
inline constexpr i32 _self::MIN_10_EXP = __private::min_10_exp<_primitive>();
inline constexpr i32 _self::MAX_10_EXP = __private::max_10_exp<_primitive>();
inline constexpr _self _self::NAN = _self(__private::nan<_primitive>());
inline constexpr _self _self::INFINITY =
    _self(__private::infinity<_primitive>());
inline constexpr _self _self::NEG_INFINITY =
    _self(__private::negative_infinity<_primitive>());

inline constexpr _self _self::consts::E =
    _self(sus_eval_and_concat(2.718281828459045235360287471352662, _suffix));
inline constexpr _self _self::consts::FRAC_1_PI =
    _self(sus_eval_and_concat(0.318309886183790671537767526745028, _suffix));
inline constexpr _self _self::consts::FRAC_1_SQRT_2 =
    _self(sus_eval_and_concat(0.7071067811865476, _suffix));
inline constexpr _self _self::consts::FRAC_2_PI =
    _self(sus_eval_and_concat(0.6366197723675814, _suffix));
inline constexpr _self _self::consts::FRAC_2_SQRT_PI =
    _self(sus_eval_and_concat(1.1283791670955126, _suffix));
inline constexpr _self _self::consts::FRAC_PI_2 =
    _self(sus_eval_and_concat(1.5707963267948966, _suffix));
inline constexpr _self _self::consts::FRAC_PI_3 =
    _self(sus_eval_and_concat(1.0471975511965979, _suffix));
inline constexpr _self _self::consts::FRAC_PI_4 =
    _self(sus_eval_and_concat(0.7853981633974483, _suffix));
inline constexpr _self _self::consts::FRAC_PI_6 =
    _self(sus_eval_and_concat(0.5235987755982989, _suffix));
inline constexpr _self _self::consts::FRAC_PI_8 =
    _self(sus_eval_and_concat(0.39269908169872414, _suffix));
inline constexpr _self _self::consts::LN_2 =
    _self(sus_eval_and_concat(0.693147180559945309417232121458176, _suffix));
inline constexpr _self _self::consts::LN_10 =
    _self(sus_eval_and_concat(2.302585092994045684017991454684364, _suffix));
inline constexpr _self _self::consts::LOG2_10 =
    _self(sus_eval_and_concat(3.321928094887362, _suffix));
inline constexpr _self _self::consts::LOG2_E =
    _self(sus_eval_and_concat(1.4426950408889634, _suffix));
inline constexpr _self _self::consts::LOG10_2 =
    _self(sus_eval_and_concat(0.3010299956639812, _suffix));
inline constexpr _self _self::consts::LOG10_E =
    _self(sus_eval_and_concat(0.4342944819032518, _suffix));
inline constexpr _self _self::consts::PI =
    _self(sus_eval_and_concat(3.141592653589793238462643383279502, _suffix));
inline constexpr _self _self::consts::SQRT_2 =
    _self(sus_eval_and_concat(1.414213562373095048801688724209698, _suffix));
inline constexpr _self _self::consts::TAU =
    _self(sus_eval_and_concat(6.283185307179586, _suffix));

#undef sus__literal

#undef _self
#undef _primitive
#undef _suffix
