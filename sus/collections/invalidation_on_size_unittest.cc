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

#define SUS_ITERATOR_INVALIDATION 1

#include "sus/collections/array.h"
#include "sus/collections/slice.h"
#include "sus/prelude.h"

using sus::Array;
using sus::Slice;

namespace {

constexpr usize round_up(usize n, usize multiple) {
  if (n % multiple == 0u) return n;
  return n + multiple - ((n + multiple) % multiple);
}

}

// The Array has a ref-count for iterator invalidation. So it's size is 5 *
// sizeof(i32) = 20 + sizeof(pointer) + padding up to a multiple of
// sizeof(pointer).
static_assert(alignof(Array<i32, 5>) == alignof(usize*));
static_assert(sizeof(Array<i32, 5>) ==
              round_up(sizeof(i32) * 5 + sizeof(usize*), alignof(usize*)));

static_assert(alignof(Slice<i32>) == alignof(usize*));
static_assert(sizeof(Slice<i32>) ==
              round_up(sizeof(i32*) + sizeof(usize) + sizeof(usize*),
                       alignof(usize*)));
