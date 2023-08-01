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

#include "subspace/containers/vec.h"
#include "subspace/marker/unsafe.h"
#include "subspace/mem/copy.h"
#include "subspace/mem/move.h"
#include "subspace/num/types.h"
#include "subspace/ops/range_literals.h"

// Imports all the things that are pulled into the top-level namespace.
// TODO: Make a compile-time option for this.

using sus::containers::Vec;
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
using sus::num::usize;
using sus::num::uptr;
