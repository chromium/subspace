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

#include "subspace/mem/take.h"

#include <type_traits>

#include "googletest/include/gtest/gtest.h"
#include "subspace/num/types.h"
#include "subspace/prelude.h"

namespace sus::mem {
namespace {

TEST(Take, Take) {
  static i32 take_destructors;

  struct S final {
    S() { default_constucted += 1_i32; }
    S(i32 i) : num(i) {}
    S(S&& other) : num(other.num), moved(other.moved + 1_i32) {}
    S& operator=(S&& other) {
      num = other.num;
      moved = other.moved + 1_i32;
      return *this;
    }
    ~S() { take_destructors += 1_i32; }

    i32 num = 101_i32;
    i32 default_constucted = 0_i32;
    i32 moved = 0_i32;
  };

  take_destructors = 0_i32;
  S s(404_i32);
  EXPECT_EQ(s.default_constucted, 0_i32);
  S out(::sus::mem::take(mref(s)));
  // `out` was moved from `s`. `s` was taken-from and default-constructed.
  EXPECT_EQ(out.num, 404_i32);
  EXPECT_EQ(s.num, 101_i32);

  // Destructions:
  // 1. `s` being destroyed before being default-constructed.
  // 2. The output of take() being destroyed when returned. However this one can
  //    be elided, in which case `out` would have been moved only once.
  EXPECT_EQ(take_destructors, 1_i32 + (out.moved - 1_i32));
}

TEST(Take, TakeConstexpr) {
  struct S final {
    constexpr S() { default_constucted += 1_i32; }
    constexpr S(i32 i) : num(i) {}
    constexpr S(S&& other) : num(other.num), moved(other.moved + 1_i32) {}
    constexpr S& operator=(S&& other) {
      num = other.num;
      moved = other.moved + 1_i32;
      return *this;
    }

    i32 num = 101_i32;
    i32 default_constucted = 0_i32;
    i32 moved = 0_i32;
  };

  auto out = []() constexpr {
    S s(404_i32);
    S out(::sus::mem::take(mref(s)));
    return out.num;
  };
  auto s = []() constexpr {
    S s(404);
    S out(::sus::mem::take(mref(s)));
    return s.num;
  };
  // `out` was moved from `s`. `s` was taken-from and default-constructed.
  EXPECT_EQ(out(), 404_i32);
  EXPECT_EQ(s(), 101_i32);
}

TEST(Take, TakeAndDestruct) {
  static i32 take_destructors;

  struct S final {
    S() { default_constucted += 1_i32; }
    S(i32 i) : num(i) {}
    S(S&& other) : num(other.num), moved(other.moved + 1_i32) {}
    S& operator=(S&& other) {
      num = other.num;
      moved = other.moved + 1_i32;
      return *this;
    }
    ~S() { take_destructors += 1_i32; }

    i32 num = 101_i32;
    i32 default_constucted = 0_i32;
    i32 moved = 0_i32;
  };

  union U final {
    U() {}
    ~U() {}
    S s;
  } u;

  take_destructors = 0_i32;
  new (&u.s) S(404_i32);
  EXPECT_EQ(u.s.default_constucted, 0_i32);
  EXPECT_EQ(u.s.num, 404_i32);
  S out(::sus::mem::take_and_destruct(unsafe_fn, mref(u.s)));
  // `out` was moved from `s`. `s` was taken-from and destroyed but not
  // reconstructed, so we can't test its values, only its destruction.
  EXPECT_EQ(out.num, 404_i32);

  // Destructions:
  // 1. `s` being destroyed before being default-constructed.
  // 2. The output of take() being destroyed when returned. However this one can
  //    be elided, in which case `out` would have been moved only once.
  EXPECT_EQ(take_destructors, 1_i32 + (out.moved - 1_i32));
}

TEST(Take, TakeAndDestructConstexpr) {
  struct S {
    constexpr S() { default_constucted += 1_i32; }
    constexpr S(i32 i) : num(i) {}
    constexpr S(S&& other) : num(other.num), moved(other.moved + 1_i32) {}
    S& operator=(S&& other) {
      num = other.num;
      moved = other.moved + 1_i32;
      return *this;
    }

    i32 num = 101_i32;
    i32 default_constucted = 0_i32;
    i32 moved = 0_i32;
  };

  auto out = []() constexpr {
    S s(404_i32);
    S out(::sus::mem::take_and_destruct(unsafe_fn, mref(s)));
    return out.num;
  };
  // `out` was moved from `s`. `s` was taken-from and destroyed, and we can not
  // use it anymore.
  EXPECT_EQ(out(), 404_i32);
}

}  // namespace
}  // namespace sus::mem
