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

#include "subspace/mem/relocate.h"

#include "subspace/num/types.h"
#include "subspace/prelude.h"

namespace {

using sus::mem::relocate_array_by_memcpy;
using sus::mem::relocate_one_by_memcpy;

static_assert(relocate_one_by_memcpy<int>);
static_assert(relocate_array_by_memcpy<int>);
static_assert(relocate_one_by_memcpy<char>);
static_assert(relocate_array_by_memcpy<char>);

struct A {
  i32 i;
};
static_assert(relocate_one_by_memcpy<A>);
static_assert(relocate_array_by_memcpy<A>);

// TODO: why it it false here? will it stay false when we can use
// __is_trivially_relocatable()?
static_assert(!relocate_one_by_memcpy<volatile A>);
static_assert(!relocate_array_by_memcpy<volatile A>);

struct B {
  B(B&&) = default;
  ~B() = default;
  i32 i;
};
static_assert(relocate_one_by_memcpy<B>);
static_assert(relocate_array_by_memcpy<B>);

struct C {
  C(C&&) = default;
  ~C() {}
  i32 i;
};
static_assert(!relocate_one_by_memcpy<C>);
static_assert(!relocate_array_by_memcpy<C>);

struct D {
  D(D&&) {}
  ~D() = default;
  i32 i;
};
static_assert(!relocate_one_by_memcpy<D>);
static_assert(!relocate_array_by_memcpy<D>);

struct [[sus_trivial_abi]] T {
  T(T&&) {}
  ~T() {}
  i32 i;
};
#if __has_extension(trivially_relocatable)
static_assert(relocate_one_by_memcpy<T>);
static_assert(relocate_array_by_memcpy<T>);
#else
static_assert(!relocate_one_by_memcpy<T>);
static_assert(!relocate_array_by_memcpy<T>);
#endif

struct [[sus_trivial_abi]] G {
  sus_class_trivial_relocatable_value(unsafe_fn, true);
  G(G&&) {}
  ~G() {}
  i32 i;
};
static_assert(relocate_one_by_memcpy<G>);
static_assert(relocate_array_by_memcpy<G>);

struct [[sus_trivial_abi]] H {
  sus_class_trivial_relocatable_value(unsafe_fn, false);
  H(H&&) {}
  ~H() {}
  i32 i;
};
static_assert(!relocate_one_by_memcpy<H>);
static_assert(!relocate_array_by_memcpy<H>);

// A mixture of explicit relocatable tag and trivial move + destroy.
static_assert(relocate_one_by_memcpy<G, int>);
static_assert(relocate_one_by_memcpy<int, G>);
static_assert(relocate_one_by_memcpy<int, G, int>);

union U {
  i32 i;
  i64 j;
};

// A union has unknown types with padding inside, so can't be considered
// trivially relocatable without knowing more. The author would need to verify
// all fields are trivially relocatable *and have the same data-size*.
static_assert(!relocate_one_by_memcpy<U>);
static_assert(!relocate_array_by_memcpy<U>);

}  // namespace
