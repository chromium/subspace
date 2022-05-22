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

#include "mem/take.h"

#include <type_traits>

#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace sus::mem {
namespace {

TEST(Take, Take) {
  static /* TODO: usize */ size_t take_destructors;

  struct S {
    S() { ++default_constucted; }
    S(int i) : num(i) {}
    S(S&& other) : num(other.num), moved(other.moved + 1) {}
    ~S() { ++take_destructors; }

    int num = 101;
    int default_constucted = 0;
    int moved = 0;
  };

  take_destructors = 0;
  S s(404);
  EXPECT_EQ(s.default_constucted, 0);
  S out(::sus::mem::take(s));
  // `out` was moved from `s`. `s` was taken-from and default-constructed.
  EXPECT_EQ(out.num, 404);
  EXPECT_EQ(s.num, 101);

  // Destructions:
  // 1. `s` being destroyed before being default-constructed.
  // 2. The output of take() being destroyed when returned. However this one can
  //    be elided, in which case `out` would have been moved only once.
  EXPECT_EQ(take_destructors, 1 + (out.moved - 1));
}

TEST(Take, TakeConstexpr) {
  struct S {
    constexpr S() { ++default_constucted; }
    constexpr S(int i) : num(i) {}
    constexpr S(S&& other) : num(other.num), moved(other.moved + 1) {}

    int num = 101;
    int default_constucted = 0;
    int moved = 0;
  };

  auto out = []() constexpr {
    S s(404);
    S out(::sus::mem::take(s));
    return out.num;
  };
  auto s = []() constexpr {
    S s(404);
    S out(::sus::mem::take(s));
    return s.num;
  };
  // `out` was moved from `s`. `s` was taken-from and default-constructed.
  EXPECT_EQ(out(), 404);
  EXPECT_EQ(s(), 101);
}

TEST(Take, TakeAndDestruct) {
  static /* TODO: usize */ size_t take_destructors;

  struct S {
    S() { ++default_constucted; }
    S(int i) : num(i) {}
    S(S&& other) : num(other.num), moved(other.moved + 1) {}
    ~S() { ++take_destructors; }

    int num = 101;
    int default_constucted = 0;
    int moved = 0;
  };

  union U {
    U() {}
    ~U() {}
    S s;
  } u;

  take_destructors = 0;
  new (&u.s) S(404);
  EXPECT_EQ(u.s.default_constucted, 0);
  EXPECT_EQ(u.s.num, 404);
  S out(::sus::mem::take_and_destruct(unsafe_fn, u.s));
  // `out` was moved from `s`. `s` was taken-from and destroyed but not
  // reconstructed, so we can't test its values, only its destruction.
  EXPECT_EQ(out.num, 404);

  // Destructions:
  // 1. `s` being destroyed before being default-constructed.
  // 2. The output of take() being destroyed when returned. However this one can
  //    be elided, in which case `out` would have been moved only once.
  EXPECT_EQ(take_destructors, 1 + (out.moved - 1));
}

TEST(Take, TakeAndDestructConstexpr) {
  struct S {
    constexpr S() { ++default_constucted; }
    constexpr S(int i) : num(i) {}
    constexpr S(S&& other) : num(other.num), moved(other.moved + 1) {}

    int num = 101;
    int default_constucted = 0;
    int moved = 0;
  };

  auto out = []() constexpr {
    S s(404);
    S out(::sus::mem::take_and_destruct(unsafe_fn, s));
    return out.num;
  };
  // `out` was moved from `s`. `s` was taken-from and destroyed, and we can not
  // use it anymore.
  EXPECT_EQ(out(), 404);
}

}  // namespace
}  // namespace sus::mem
