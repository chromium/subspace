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

#include "sus/ptr/as_ref.h"

#include <concepts>

#include "googletest/include/gtest/gtest.h"
#include "sus/prelude.h"

namespace {
using sus::Option;

TEST(PtrAsRef, Test) {
  // Mutable.
  {
    i32 i = 2;
    i32* p = &i;
    i32* n = nullptr;
    auto o = sus::ptr::as_ref(p);
    static_assert(std::same_as<decltype(o), Option<i32&>>);
    EXPECT_EQ(&*o, &i);
    auto o2 = sus::ptr::as_ref(n);
    static_assert(std::same_as<decltype(o), Option<i32&>>);
    EXPECT_EQ(o2.is_none(), true);
  }
  // Const.
  {
    const i32 i = 2;
    const i32* p = &i;
    const i32* n = nullptr;
    auto o = sus::ptr::as_ref(p);
    static_assert(std::same_as<decltype(o), Option<const i32&>>);
    EXPECT_EQ(&*o, &i);
    auto o2 = sus::ptr::as_ref(n);
    static_assert(std::same_as<decltype(o), Option<const i32&>>);
    EXPECT_EQ(o2.is_none(), true);
  }

  // Constexpr.
  static_assert(sus::ptr::as_ref<i32>(nullptr).is_none());
  static_assert([]() {
    const i32 i = 2;
    auto o = sus::ptr::as_ref(&i);
    return o.is_some();
  }());
}

}  // namespace
