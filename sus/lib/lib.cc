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

#include "sus/lib/lib.h"

#include <limits.h>
#include <stddef.h>

#include "sus/mem/size_of.h"
#include "sus/num/signed_integer.h"
#include "sus/num/signed_integer_impl.h"

// Architectural assumptions we make throughout the implementation of Subspace.
static_assert(CHAR_BIT == 8);
// Signed integers are allowed to have padding so they can have a larger size
// thus we don't compare the size of `int` but its max value instead.
static_assert(INT_MAX == INT32_MAX);
static_assert(sus::mem::size_of<size_t>() >= 4);
static_assert(sus::mem::size_of<size_t>() <= 8);

// TODO: Consider if we should only support little endian? We probably make this
// assumption. Support for endian *conversion* is still important for network
// byte order etc.

// The Vec class, along with any other class with pointer arithmetic, assumes
// isize::MAX == PTRDIFF_MAX.
static_assert(sus::isize::MAX == PTRDIFF_MAX);

#if defined(__clang__)
// Require clang 16 to avoid bugs and for required features.
static_assert(__clang_major__ >= 16, "requires at least clang 16");
#endif
