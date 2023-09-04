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

#include "sus/assertions/check.h"

#include "googletest/include/gtest/gtest.h"

namespace sus {
namespace {

TEST(Check, CheckPasses) {
  check(true);
  check_with_message(true, "hello world");
}

TEST(Check, CheckFails) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(check(false), "");
#endif
}

TEST(Check, WithMessage) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(check_with_message(false, "hello world"), "'hello world'");
#endif
#if GTEST_HAS_DEATH_TEST
  // Verify it does not read past the string_view's end.
  EXPECT_DEATH(check_with_message(
                   false, std::string_view("hello world123").substr(0u, 11u)),
               "'hello world'");
#endif
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(check_with_message(false, std::string("hello world")),
               "'hello world'");
#endif
}

}  // namespace
}  // namespace sus
