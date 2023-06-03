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

#include "subspace/string/compat_stream.h"

#include <sstream>

#include "googletest/include/gtest/gtest.h"
#include "subspace/prelude.h"

namespace {

TEST(Stream, SubspaceType) {
  std::stringstream s;
  // i32 is formattable, so it is also streamable with compat_stream.h.
  s << 1_i32 << " " << 2_i32 << " " << 3_i32;
  EXPECT_EQ(s.str(), "1 2 3");
}

TEST(Stream, GTest) {
  // i32 is formattable, so it is also printable by GTest.
  EXPECT_EQ(testing::PrintToString(123_i32), "123");
}

}  // namespace
