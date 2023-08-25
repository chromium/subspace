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

// IWYU pragma: private, include "sus/prelude.h"
// IWYU pragma: friend "sus/.*"
#pragma once

namespace sus {

/// Safe integer and floating point numerics, and numeric concepts.
///
/// This namespace contains safe integer types (i8, i16, u32, usize, etc.) and
/// floating point types (f32, f64).
///
/// Additionally, there are Concepts that match against safe numerics, C++
/// primitive types, and operations with numeric types.
///
/// # Conversions
///
/// To convert to and from integer values, use
/// [`sus::into`](sus-construct-fn.into.html) when
/// [`Into<From, To>`](sus-construct-Into.html) is satisfied between the two
/// types for lossless conversion. Otherwise use
/// [`sus::try_into`](sus-construct-fn.try_into.html) when
/// [`TryInto<From, To>`](sus-construct-TryInto.html) is satisfied to convert
/// and handle cases where the value can not be represented in the target type.
///
/// To convert between floating point types, use
/// [`sus::into(x)`](sus-construct-fn.into.html) to losslessly promote `x` to a
/// larger type ([`f32`](sus-num-f32.html) to [`f64`](sus-num-f32.html)) or
/// `sus::try_into(x)` to convert `x` to a smaller type
/// ([`f64`](sus-num-f32.html) to [`f32`](sus-num-f32.html)).
///
/// Use [`sus::mog<T>()`](sus-construct-fn.mog.html) to do a lossy type coercion
/// (like `static_cast<T>()`) between integer and floating point types, or C++
/// primitive integers, floating point, or enums. When converting to a larger
/// signed integer type, the value will be sign-extended.
namespace num {}

}  // namespace sus

// IWYU pragma: begin_exports
#include "sus/num/float.h"
#include "sus/num/float_impl.h"
#include "sus/num/signed_integer.h"
#include "sus/num/signed_integer_impl.h"
#include "sus/num/transmogrify.h"
#include "sus/num/try_from_int_error_impl.h"
#include "sus/num/unsigned_integer.h"
#include "sus/num/unsigned_integer_impl.h"
// IWYU pragma: end_exports
