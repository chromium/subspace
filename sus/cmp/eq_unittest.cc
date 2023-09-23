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

#include "sus/cmp/eq.h"

#include "googletest/include/gtest/gtest.h"

namespace {

using sus::cmp::Eq;

struct CComp {};
struct C {
  friend bool operator==(const C&, const C&) noexcept { return true; }
  friend bool operator==(const C&, const CComp&) noexcept { return true; }
};

struct E {
  // Not noexcept.
  friend bool operator==(const E&, const E&) { return true; }
  friend bool operator==(const E&, const CComp&) { return true; }
};

// These types are comparable.
static_assert(Eq<int>);
static_assert(Eq<char>);
static_assert(Eq<C>);
static_assert(Eq<C, C>);
static_assert(Eq<C, CComp>);

// Not noexcept. Allowed for compat with std.
static_assert(Eq<E>);
static_assert(Eq<E, CComp>);

struct S {};  // No operator==.

// These types are not comparable.
static_assert(!Eq<S>);

}  // namespace
