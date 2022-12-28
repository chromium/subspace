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

#include "convert/subclass.h"

#include "googletest/include/gtest/gtest.h"

namespace {

using sus::convert::SameOrSubclassOf;

struct Base {};
struct Sub : public Base {};
struct Other {};

static_assert(SameOrSubclassOf<Base*, Base*>);
static_assert(SameOrSubclassOf<Sub*, Base*>);
static_assert(!SameOrSubclassOf<Other*, Base*>);

// Only works with pointers.
static_assert(!SameOrSubclassOf<Base, Base>);
static_assert(!SameOrSubclassOf<Sub, Base>);
static_assert(!SameOrSubclassOf<Base*, Base>);
static_assert(!SameOrSubclassOf<Base, Base*>);

template <SameOrSubclassOf<Base*> X>
void fn(X) {}

TEST(Subclass, ConceptGuard) {
  auto s = Sub();
  fn(&s);
}

}  // namespace
