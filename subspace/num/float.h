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

#include <concepts>
#include <functional>  // TODO: remove this but we need to hash things > size_t.

#include "fmt/format.h"
#include "subspace/iter/iterator_concept.h"
#include "subspace/macros/__private/compiler_bugs.h"
#include "subspace/macros/eval_and_concat.h"
#include "subspace/macros/pure.h"
#include "subspace/marker/unsafe.h"
#include "subspace/mem/move.h"
#include "subspace/mem/relocate.h"
#include "subspace/mem/size_of.h"
#include "subspace/num/__private/float_ordering.h"
#include "subspace/num/__private/intrinsics.h"
#include "subspace/num/__private/literals.h"
#include "subspace/num/cmath_macros.h"
#include "subspace/num/float_concepts.h"
#include "subspace/num/fp_category.h"
#include "subspace/num/signed_integer.h"
#include "subspace/num/unsigned_integer.h"
#include "subspace/string/__private/format_to_stream.h"

namespace sus::num {

/// A 32-bit floating point type (specifically, this type holds the same values
/// as the `float` type specified by the C++ standard).
///
/// This type can represent a wide range of decimal numbers, like 3.5, 27,
/// -113.75, 0.0078125, 34359738368, 0, -1. So unlike integer types (such as
/// `i32`), floating point types can represent non-integer numbers, too.
///
/// # Conversions
///
/// To convert between floating point types, use `sus::into()` to losslessly
/// promote to larger types (`f32` to `f64`) `sus::try_into()` to try convert to
/// smaller types (`f64` to `f32`).
///
/// Use use `sus::mog<T>()` to do a lossy type coercion (like
/// `static_cast<T>()`) between integer and floating point types, or C++
/// primitive integers, floating point, or enums. When
/// converting to a larger signed integer type, the value will be sign-extended.
struct [[sus_trivial_abi]] f32 final {
#define _self f32
#define _primitive float
#define _unsigned u32
#include "subspace/num/__private/float_methods.inc"
};
#define _self f32
#define _primitive float
#define _suffix f
#include "subspace/num/__private/float_consts.inc"

/// A 64-bit floating point type (specifically, this type holds the same values
/// as the `double` type specified by the C++ standard).
///
/// This type is very similar to `f32`, but has increased precision by using
/// twice as many bits.
///
/// # Conversions
///
/// To convert between floating point types, use `sus::into()` to losslessly
/// promote to larger types (`f32` to `f64`) `sus::try_into()` to try convert to
/// smaller types (`f64` to `f32`).
///
/// Use use `sus::mog<T>()` to do a lossy type coercion (like
/// `static_cast<T>()`) between integer and floating point types, or C++
/// primitive integers, floating point, or enums. When
/// converting to a larger signed integer type, the value will be sign-extended.
struct [[sus_trivial_abi]] f64 final {
#define _self f64
#define _primitive double
#define _unsigned u64
#include "subspace/num/__private/float_methods.inc"
};
#define _self f64
#define _primitive double
#define _suffix 0
#include "subspace/num/__private/float_consts.inc"

}  // namespace sus::num

_sus__float_literal(f32, ::sus::num::f32);
_sus__float_literal(f64, ::sus::num::f64);

// Promote floating point types into the `sus` namespace.
namespace sus {
using sus::num::f32;
using sus::num::f64;
}  // namespace sus
