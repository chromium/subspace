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

#include "sus/fn/fn_dyn.h"

#include "googletest/include/gtest/gtest.h"
#include "sus/boxed/box.h"
#include "sus/prelude.h"

namespace {
using namespace sus::fn;

TEST(FnDyn, Fn) {
  auto x = [](const DynFn<i32(i32, i32)>& f) { return call(f, 1, 2); };
  i32 c = x(sus::dyn<DynFn<i32(i32, i32)>>([](i32 a, i32 b) { return a + b; }));
  EXPECT_EQ(c, 1 + 2);
  i32 d = x(sus::dyn<DynFn<i32(i32, i32)>>([](i32 a, i32 b) { return a * b; }));
  EXPECT_EQ(d, 1 * 2);
}

TEST(FnDyn, FnBox) {
  auto x = [](sus::Box<DynFn<i32(i32, i32)>> f) { return f(1, 2); };
  i32 c = x(sus::into([](i32 a, i32 b) { return a + b; }));
  EXPECT_EQ(c, 1 + 2);
  i32 d = x(sus::into([](i32 a, i32 b) { return a * b; }));
  EXPECT_EQ(d, 1 * 2);
}

TEST(FnMutDyn, FnMut) {
  auto x = [](DynFnMut<i32(i32, i32)>& f) { return call_mut(f, 1, 2); };
  i32 c =
      x(sus::dyn<DynFnMut<i32(i32, i32)>>([](i32 a, i32 b) { return a + b; }));
  EXPECT_EQ(c, 1 + 2);
  i32 d =
      x(sus::dyn<DynFnMut<i32(i32, i32)>>([](i32 a, i32 b) { return a * b; }));
  EXPECT_EQ(d, 1 * 2);
}

TEST(FnMutDyn, FnMutBox) {
  auto x = [](sus::Box<DynFnMut<i32(i32, i32)>> f) { return f(1, 2); };
  i32 c = x(sus::into([](i32 a, i32 b) { return a + b; }));
  EXPECT_EQ(c, 1 + 2);
  i32 d = x(sus::into([](i32 a, i32 b) { return a * b; }));
  EXPECT_EQ(d, 1 * 2);
}

TEST(FnDyn, FnOnce) {
  auto x = [](DynFnOnce<i32(i32, i32)>&& f) {
    return call_once(sus::move(f), 1, 2);
  };
  i32 c =
      x(sus::dyn<DynFnOnce<i32(i32, i32)>>([](i32 a, i32 b) { return a + b; }));
  EXPECT_EQ(c, 1 + 2);
  i32 d =
      x(sus::dyn<DynFnOnce<i32(i32, i32)>>([](i32 a, i32 b) { return a * b; }));
  EXPECT_EQ(d, 1 * 2);
}

TEST(FnOnceDyn, FnOnceBox) {
  auto x = [](sus::Box<DynFnOnce<i32(i32, i32)>> f) {
    return sus::move(f)(1, 2);
  };
  i32 c = x(sus::into([](i32 a, i32 b) { return a + b; }));
  EXPECT_EQ(c, 1 + 2);
  i32 d = x(sus::into([](i32 a, i32 b) { return a * b; }));
  EXPECT_EQ(d, 1 * 2);
}

TEST(Fn, Example_NonVoidDyn) {
  auto func = [](DynFnMut<i32(i32)>& f) {
    auto x = f(0);
    x += 3;
  };

  func(sus::dyn<DynFnMut<i32(i32)>>([](i32) { return 3; }));
}

}  // namespace
