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

#include "sus/boxed/box.h"

#include "googletest/include/gtest/gtest.h"
#include "sus/error/error.h"
#include "sus/prelude.h"

namespace test::box {
struct MyError {};
}  // namespace test::box

using namespace test::box;

template <>
struct sus::error::ErrorImpl<MyError> {
  constexpr static std::string display(const MyError&) noexcept {
    return "my error";
  }
};

namespace {
using sus::boxed::Box;

// For any T.
static_assert(sus::mem::NeverValueField<Box<i32>>);
// For any T.
static_assert(sus::mem::relocate_by_memcpy<Box<i32>>);

TEST(Box, Construct) {
  i32 i = 3;
  {
    auto b = Box<i32>(i);
    EXPECT_EQ(*b, 3);
  }
}
TEST(Box, FromT) {
  i32 i = 3;
  {
    auto b = Box<i32>::from(i);
    EXPECT_EQ(*b, 3);
  }
  {
    Box<i32> b = sus::into(i);
    EXPECT_EQ(*b, 3);
  }
}

TEST(Box, FromError) {
  {
    auto b = Box<sus::error::DynError>::from(MyError());
    static_assert(sus::error::Error<decltype(b)>);
    EXPECT_EQ(sus::error::error_display(*b), "my error");
    EXPECT_EQ(sus::error::error_display(b), "my error");
  }
  {
    Box<sus::error::DynError> b = sus::into(MyError());
    static_assert(sus::error::Error<decltype(b)>);
    EXPECT_EQ(sus::error::error_display(*b), "my error");
    EXPECT_EQ(sus::error::error_display(b), "my error");
  }
}

}  // namespace
