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

///////////////////////////////////////////////////////////////////////////
//
// Defines constants in an integer type.
//
// TO USE THIS INC FILE:
//
// Include it beside the class, in the same namespace.
//
// Define `_self` to the name of the integer type.
// Define `_primitive` to the primitive it holds inside.
///////////////////////////////////////////////////////////////////////////

inline constexpr _self _self::MIN = _self(_self::MIN_PRIMITIVE);
inline constexpr _self _self::MAX = _self(_self::MAX_PRIMITIVE);
inline constexpr u32 _self::BITS = u32(__private::num_bits<_primitive>());

#undef _self
#undef _primitive
