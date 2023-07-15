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

#define SUS_ITERATOR_INVALIDATION 0

#include "subspace/containers/array.h"
#include "subspace/containers/slice.h"
#include "subspace/macros/compiler.h"
#include "subspace/prelude.h"

using sus::Array;
using sus::Slice;

constexpr usize array_padding = 0u
    // No support for no_unique_address in clang-cl.
    sus_clang_bug_49358(+sizeof(i32))
    // MSVC bug with no_unique_address and arrays, so not used here.
    sus_clang_bug_49358_else(sus_msvc_bug_10416202(+sizeof(i32)));

static_assert(alignof(Array<i32, 5>) == alignof(i32));
static_assert(sizeof(Array<i32, 5>) == sizeof(i32) * 5 + array_padding);

constexpr usize slice_padding = 0u
    // No support for no_unique_address in clang-cl.
    sus_clang_bug_49358(+sizeof(i32*));

static_assert(alignof(Slice<i32>) == alignof(i32*));
static_assert(sizeof(Slice<i32*>) ==
              sizeof(i32*) + sizeof(usize) + slice_padding);
