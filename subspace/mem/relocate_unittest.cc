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

using sus::mem::relocate_by_memcpy;

static_assert(relocate_by_memcpy<int>);
static_assert(relocate_by_memcpy<char>);

struct A {
  i32 i;
};
static_assert(relocate_by_memcpy<A>);

// Volatile types are not trivially relocatable:
// https://quuxplusone.github.io/blog/2018/07/13/trivially-copyable-corner-cases/
static_assert(!relocate_by_memcpy<volatile A>);

// User defined default move operations and destructor for a trivially
// relocatable field means the outer type is also trivially relocatable.
struct B {
  B(B&&) = default;
  B& operator=(B&&) = default;
  ~B() = default;
  i32 i;
};
static_assert(relocate_by_memcpy<B>);

// A non-default assignment operator means the type needs to opt into being
// trivially relocatable, as it needs to be trivially relocatable for
// construction and assignment.
struct C {
  C(C&&) = default;
  C& operator=(C&&) { return *this; }
  ~C() = default;
  i32 i;
};
static_assert(!relocate_by_memcpy<C>);

// A non-default constructor means the type needs to opt into being trivially
// relocatable, as it needs to be trivially relocatable for construction and
// assignment.
struct D {
  D(D&&) {}
  D& operator=(D&&) = default;
  ~D() = default;
  i32 i;
};
static_assert(!relocate_by_memcpy<D>);

// A non-default destructor means the type needs to opt into being trivially
// relocatable.
struct E {
  E(E&&) = default;
  E& operator=(E&&) = default;
  ~E() {}
  i32 i;
};
static_assert(!relocate_by_memcpy<E>);

struct [[sus_trivial_abi]] F {
  F(F&&) {}
  ~F() {}
  i32 i;
};
#if defined(__clang__) && __has_extension(trivially_relocatable)
static_assert(relocate_by_memcpy<F>);
#else
static_assert(!relocate_by_memcpy<F>);
#endif

struct [[sus_trivial_abi]] G {
  sus_class_trivially_relocatable_if(unsafe_fn, true);
  G(G&&) {}
  ~G() {}
  i32 i;
};
static_assert(relocate_by_memcpy<G>);

struct [[sus_trivial_abi]] H {
  sus_class_trivially_relocatable_if(unsafe_fn, false);
  H(H&&) {}
  ~H() {}
  i32 i;
};
static_assert(!relocate_by_memcpy<H>);

// A mixture of explicit relocatable tag and trivial move + destroy.
static_assert(relocate_by_memcpy<G, int>);
static_assert(relocate_by_memcpy<int, G>);
static_assert(relocate_by_memcpy<int, G, int>);

union U {
  i32 i;
  i64 j;
};

// A union has unknown types with padding inside, so can't be considered
// trivially relocatable without knowing more. The author would need to verify
// all fields are trivially relocatable *and have the same data-size*.
static_assert(!relocate_by_memcpy<U>);

}  // namespace
