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

#include "construct/make_default.h"

#include "googletest/include/gtest/gtest.h"

using sus::construct::make_default;
using sus::construct::MakeDefault;

namespace {

struct DefaultConstructible {
  const int i = 2;
};
struct NotDefaultConstructible {
  constexpr NotDefaultConstructible(int i) noexcept : i(i) {}
  const int i = 2;
};
struct WithDefaultConstructible {
  constexpr static WithDefaultConstructible with_default() noexcept {
    return WithDefaultConstructible(3);
  }

  const int i = 2;

 private:
  constexpr WithDefaultConstructible(int i) noexcept : i(i) {}
};
// DefaultConstructible and WithDefaultConstructible.
struct BothConstructible {
  constexpr BothConstructible() noexcept {}
  constexpr static BothConstructible with_default() noexcept {
    return BothConstructible(3);
  }

  const int i = 2;

 private:
  constexpr BothConstructible(int i) noexcept : i(i) {}
};

static_assert(MakeDefault<DefaultConstructible> == true);
static_assert(MakeDefault<NotDefaultConstructible> == false);
static_assert(MakeDefault<WithDefaultConstructible> == true);
static_assert(MakeDefault<BothConstructible> == false);

// Verify constexpr construction.
constexpr auto default_constructible = make_default<DefaultConstructible>();
static_assert(default_constructible.i == 2);
constexpr auto with_default_constructible =
    make_default<WithDefaultConstructible>();
static_assert(with_default_constructible.i == 3);

// Verify no type coersions are happening.
static_assert(std::is_same_v<decltype(make_default<DefaultConstructible>()),
                             DefaultConstructible>,
              "");
static_assert(std::is_same_v<decltype(make_default<WithDefaultConstructible>()),
                             WithDefaultConstructible>,
              "");

TEST(MakeDefault, NonConstexprConstruction) {
  auto default_constructible1 = make_default<DefaultConstructible>();
  EXPECT_EQ(default_constructible1.i, 2);
  auto with_default_constructible1 = make_default<WithDefaultConstructible>();
  EXPECT_EQ(with_default_constructible1.i, 3);
}

}  // namespace
