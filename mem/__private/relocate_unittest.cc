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

#include "mem/__private/relocate.h"

namespace sus::mem::__private {
namespace {

static_assert(relocate_one_by_memcpy_v<int>, "");
static_assert(relocate_array_by_memcpy_v<int>, "");
static_assert(relocate_one_by_memcpy_v<char>, "");
static_assert(relocate_array_by_memcpy_v<char>, "");

struct A {};
static_assert(relocate_one_by_memcpy_v<A>, "");
static_assert(relocate_array_by_memcpy_v<A>, "");

// TODO: why it it false here? will it stay false when we can use
// __is_trivially_relocatable()?
static_assert(!relocate_one_by_memcpy_v<volatile A>, "");
static_assert(!relocate_array_by_memcpy_v<volatile A>, "");

struct B {
  B(B &&) = default;
  ~B() = default;
};
static_assert(relocate_one_by_memcpy_v<B>, "");
static_assert(relocate_array_by_memcpy_v<B>, "");

struct C {
  C(C &&) = default;
  ~C() {}
};
static_assert(!relocate_one_by_memcpy_v<C>, "");
static_assert(!relocate_array_by_memcpy_v<C>, "");

struct D {
  D(D &&) {}
  ~D() = default;
};
static_assert(!relocate_one_by_memcpy_v<D>, "");
static_assert(!relocate_array_by_memcpy_v<D>, "");

struct [[clang::trivial_abi]] T {
  T(T &&) {}
  ~T() {}
};
#if __has_extension(trivially_relocatable)
static_assert(relocate_one_by_memcpy_v<T>, "");
static_assert(relocate_array_by_memcpy_v<T>, "");
#else
static_assert(!relocate_one_by_memcpy_v<T>, "");
static_assert(!relocate_array_by_memcpy_v<T>, "");
#endif

} // namespace
} // namespace sus::mem::__private
