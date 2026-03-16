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

#include "sus/assertions/check.h"
#else
#include "sus/cmp/reverse.h"

#include "sus/prelude.h"
#endif

#include "googletest/include/gtest/gtest.h"

namespace {
using namespace sus::cmp;

TEST(Reverse, Example_ReverseKey) {
  using sus::cmp::Reverse;

  auto v = sus::Vec<i32>(1, 2, 3, 4, 5, 6);
  v.sort_by_key([](i32 num) { return sus::Tuple(num > 3, Reverse(num)); });
  sus_check(v == sus::Vec<i32>(3, 2, 1, 6, 5, 4));
}

}  // namespace
