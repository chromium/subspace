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

#include "num/__private/literals.h"
#include "num/__private/signed_integer_macros.h"
#include "num/integer_concepts.h"

namespace sus::num {

// TODO: from_str_radix(). Need Result type and Errors.

// TODO: div_ceil() and div_floor()? Lots of discussion still on
// https://github.com/rust-lang/rust/issues/88581 for signed types.

/// A 32-bit signed integer.
struct i32 {
  // TODO: Split apart the declarations and the definitions? Then they can be in
  // u32_defn.h and u32_impl.h, allowing most of the library to just use
  // u32_defn.h which will keep some headers smaller. But then the combined
  // headers are larger, is that worse?
  _sus__signed_impl(i32, int32_t,
                    /*UnsignedT=*/u32, /*LargerT=*/int64_t);
};

}  // namespace sus::num

_sus__signed_literal(i32, ::sus::num::i32, /*PrimitiveT=*/int32_t);

// Promote i32 into the top-level namespace.
using sus::num::i32;
