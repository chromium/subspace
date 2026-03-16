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

#ifdef TEST_MODULE
import sus;
#else
#include "sus/marker/unsafe.h"
#endif

#include <sstream>

#include "fmt/format.h"
#include "googletest/include/gtest/gtest.h"

namespace {

using sus::marker::unsafe_fn;

TEST(UnsafeFnMarker, fmt) {
  EXPECT_EQ(fmt::format("{}", unsafe_fn), "unsafe_fn");
}

TEST(UnsafeFnMarker, Stream) {
  std::stringstream s;
  s << unsafe_fn;
  EXPECT_EQ(s.str(), "unsafe_fn");
}

TEST(UnsafeFnMarker, GTest) {
  EXPECT_EQ(testing::PrintToString(unsafe_fn), "unsafe_fn");
}

}  // namespace
