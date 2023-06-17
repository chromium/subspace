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

#include "subspace/iter/successors.h"

#include "googletest/include/gtest/gtest.h"
#include "subspace/prelude.h"

TEST(Successors, GoGoGo) {
  auto powers_of_10 = sus::iter::Successors<u16>::with(
      sus::some(1_u16), [](const u16& n) { return n.checked_mul(10_u16); });
  EXPECT_EQ(
      sus::move(powers_of_10).collect<Vec<u16>>(),
      sus::Vec<u16>::with_values(1_u16, 10_u16, 100_u16, 1000_u16, 10000_u16));
}
