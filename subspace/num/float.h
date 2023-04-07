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

#include <functional>  // TODO: remove this but we need to hash things > size_t.

#include "subspace/num/__private/float_consts.h"
#include "subspace/num/__private/float_macros.h"
#include "subspace/num/__private/literals.h"

namespace sus::num {

/// A 32-bit floating point type (specifically, the “binary32” type defined in
/// IEEE 754-2008).
///
/// This type can represent a wide range of decimal numbers, like 3.5, 27,
/// -113.75, 0.0078125, 34359738368, 0, -1. So unlike integer types (such as
/// i32), floating point types can represent non-integer numbers, too.
struct f32 final {
  _sus__float_consts_struct(f32);
  _sus__float(f32, float, u32);
};
_sus__float_consts_struct_out_of_line(f32, f);
_sus__float_constants_out_of_line(f32, float);

/// A 64-bit floating point type (specifically, the “binary64” type defined in
/// IEEE 754-2008).
///
/// This type is very similar to `f32`, but has increased precision by using
/// twice as many bits. Please see the documentation for `f32` for more
/// information.
struct f64 final {
  _sus__float_consts_struct(f64);
  _sus__float(f64, double, u64);
};
_sus__float_consts_struct_out_of_line(f64, );
_sus__float_constants_out_of_line(f64, double);

}  // namespace sus::num

namespace std {
_sus__float_hash_equal_to(::sus::num::f32);
_sus__float_hash_equal_to(::sus::num::f64);
}  // namespace std

_sus__float_literal(f32, ::sus::num::f32);
_sus__float_literal(f64, ::sus::num::f64);

// Promote floating point types into the `sus` namespace.
namespace sus {
using sus::num::f32;
using sus::num::f64;
}  // namespace sus
