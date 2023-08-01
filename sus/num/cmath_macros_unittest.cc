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

#include <cmath>

#include "googletest/include/gtest/gtest.h"
#include "sus/num/float.h"

TEST(CMathMacros, Nan) {
  // The NAN macro conflicts with a constant name. So we redefine it as STD_NAN.
  constexpr auto nan = STD_NAN;  // Must be a constant expression.
  EXPECT_TRUE(std::isnan(nan));  // constexpr in C++23.
}

TEST(CMathMacros, Inf) {
  // The INFINITY macro conflicts with a constant name. So we redefine it as
  // STD_INFINITY.
  constexpr auto inf = STD_INFINITY;  // Must be a constant expression.
  EXPECT_TRUE(std::isinf(inf));       // constexpr in C++23.
}
