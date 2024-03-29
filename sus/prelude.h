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

// IWYU pragma: always_keep
#pragma once

#include "sus/collections/vec.h"
#include "sus/marker/unsafe.h"
#include "sus/mem/copy.h"
#include "sus/mem/move.h"
#include "sus/num/types.h"
#include "sus/ops/range_literals.h"
#include "sus/option/option.h"

namespace sus {

/// Commonly used things that can be pulled into the global top level namespace.
/// This is done by default when including the `sus/prelude.h` header.
///
/// If `SUS_PRELUDE_NO_IMPORT` is defined to be `true` the prelude will not be
/// imported into the global top level namespace. Then the prelude can be
/// manually brought into scope with `using namespace sus::prelude;`.
///
/// ## Examples
/// ```
/// #include "sus/prelude.h"
/// 
/// int main() {
///     auto v = Vec<i32>(1, 2, 3, 4, 5);
///     return sus::move(v).into_iter().sum();
/// }
/// ```
namespace prelude {
using sus::collections::Vec;
using sus::marker::unsafe_fn;
using sus::num::f32;
using sus::num::f64;
using sus::num::i16;
using sus::num::i32;
using sus::num::i64;
using sus::num::i8;
using sus::num::isize;
using sus::num::u16;
using sus::num::u32;
using sus::num::u64;
using sus::num::u8;
using sus::num::uptr;
using sus::num::usize;
using sus::option::Option;
}  // namespace prelude

}  // namespace sus

#if !defined(SUS_PRELUDE_NO_IMPORT) || SUS_PRELUDE_NO_IMPORT
using sus::prelude::Vec;
using sus::prelude::unsafe_fn;
using sus::prelude::f32;
using sus::prelude::f64;
using sus::prelude::i16;
using sus::prelude::i32;
using sus::prelude::i64;
using sus::prelude::i8;
using sus::prelude::isize;
using sus::prelude::u16;
using sus::prelude::u32;
using sus::prelude::u64;
using sus::prelude::u8;
using sus::prelude::uptr;
using sus::prelude::usize;
using sus::prelude::Option;
#endif
