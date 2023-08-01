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

#include "sus/assertions/unreachable.h"

#include "googletest/include/gtest/gtest.h"
#include "sus/prelude.h"

// Incredibly, on Posix we can use [0-9] but on Windows we can't. Yet on Windows
// we can use `\d` and on Posix we can't (well, it causes UBSan to fire during
// compilation and then it doesn't match).
#if GTEST_USES_SIMPLE_RE
#define DIGIT "\\d"
#else
#define DIGIT "[0-9]"
#endif

namespace sus {
namespace {

TEST(UnreachableDeathTest, Unreachable) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(unreachable(),
               "^PANIC! at .*unreachable_unittest.cc:" DIGIT "+:" DIGIT "+\n$");
#endif
}

TEST(Unreachable, Unchecked) {
  switch (0) {
    case 0: return;
  }
  // We can't actually land here or we'd introduce UB, but the test confirms
  // we can write it and it compiles without warnings.
  unreachable_unchecked(unsafe_fn);
}

}  // namespace
}  // namespace sus
