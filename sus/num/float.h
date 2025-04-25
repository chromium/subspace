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

#include <concepts>
#include <functional>  // TODO: remove this but we need to hash things > size_t.

#include "fmt/format.h"
#include "sus/iter/iterator_concept.h"
#include "sus/macros/__private/compiler_bugs.h"
#include "sus/macros/eval_and_concat.h"
#include "sus/macros/pure.h"
#include "sus/marker/unsafe.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"
#include "sus/mem/size_of.h"
#include "sus/num/__private/float_ordering.h"
#include "sus/num/__private/intrinsics.h"
#include "sus/num/__private/literals.h"
#include "sus/num/float_concepts.h"
#include "sus/num/fp_category.h"
#include "sus/num/signed_integer.h"
#include "sus/num/unsigned_integer.h"
#include "sus/string/__private/format_to_stream.h"

namespace sus::num {

/// A 32-bit floating point type.
///
/// This type can represent a wide range of decimal numbers, like 3.5, 27,
/// -113.75, 0.0078125, 34359738368, 0, -1. So unlike integer types (such as
/// `i32`), floating point types can represent non-integer numbers, too.
/// Specifically, this type holds the same values
/// as the `float` type specified by the C++ standard.
///
/// See the [namespace level documentation]($sus::num) for more.
struct [[_sus_trivial_abi]] f32 final {
#define _self f32
#define _primitive float
#define _unsigned u32
#include "sus/num/__private/float_methods.inc"
};
#define _self f32
#define _primitive float
#define _suffix f
#include "sus/num/__private/float_consts.inc"

/// A 64-bit floating point type.
///
/// This type is very similar to [`f32`]($sus::num::f32), but has increased precision by using
/// twice as many bits.
/// Specifically, this type holds the same values
/// as the `double` type specified by the C++ standard.
///
/// See the [namespace level documentation]($sus::num) for more.
struct [[_sus_trivial_abi]] f64 final {
#define _self f64
#define _primitive double
#define _unsigned u64
#include "sus/num/__private/float_methods.inc"
};
#define _self f64
#define _primitive double
#define _suffix 0
#include "sus/num/__private/float_consts.inc"

}  // namespace sus::num

/// For writing [`f32`]($sus::num::f32) literals.
///
/// Floating point literals qualified with `f` are also 32 bits large, but the
/// `_f32` suffix forces a safe numeric type instead of a primitive value when
/// this is needed (such as for templates or member function access).
///
/// Integer values that are not representable by [`f32`]($sus::num::f32) will
/// converted by the same rules as for [`Cast`]($sus::construct::Cast).
/// Floating point values out of range for [`f32`]($sus::num::f32) will fail to
/// compile.
///
/// # Examples
/// ```
/// auto i = 123_f32 - (5_f32).abs();
/// sus_check(i == 118_f32);
/// ```
_sus__float_literal(f32, ::sus::num::f32);
/// For writing [`f64`]($sus::num::f64) literals.
///
/// Floating point literals without a qualifier are also 64 bits large, but the
/// `_f64` suffix forces a safe numeric type instead of a primitive value when
/// this is needed (such as for templates or member function access).
///
/// Integer values that are not representable by [`f64`]($sus::num::f64) will
/// converted by the same rules as for [`Cast`]($sus::construct::Cast).
///
/// # Examples
/// ```
/// auto i = 123_f64- (5_f64).abs();
/// sus_check(i == 118_f64);
/// ```
_sus__float_literal(f64, ::sus::num::f64);

// Promote floating point types into the `sus` namespace.
namespace sus {
using sus::num::f32;
using sus::num::f64;
}  // namespace sus
