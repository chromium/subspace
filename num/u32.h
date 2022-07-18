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

#include "concepts/from.h"
#include "concepts/into.h"
#include "num/__private/literals.h"
#include "num/__private/unsigned_integer_macros.h"
#include "num/integer_concepts.h"

namespace sus::num {

// TODO: from_str_radix(). Need Result type and Errors.

/// A 32-bit unsigned integer.
struct u32 {
  // TODO: Split apart the declarations and the definitions? Then they can be in
  // u32_defn.h and u32_impl.h, allowing most of the library to just use
  // u32_defn.h which will keep some headers smaller. But then the combined
  // headers are larger, is that worse?
  _sus__unsigned_impl(u32, /*PrimitiveT=*/uint32_t, /*SignedT=*/i32,
                      /*LargerT=*/uint64_t);
};

}  // namespace sus::num

_sus__unsigned_literal(u32, ::sus::num::u32, /*PrimitiveT=*/uint32_t);

// Promote u32 into the top-level namespace.
using sus::num::u32;
