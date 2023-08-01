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

#include "sus/iter/successors.h"

#include "googletest/include/gtest/gtest.h"
#include "sus/prelude.h"

namespace {

TEST(Successors, Example) {
  auto powers_of_10 = sus::iter::successors<u16>(
      sus::some(1_u16), [](const u16& n) { return n.checked_mul(10_u16); });
  sus::check(
      sus::move(powers_of_10).collect<Vec<u16>>() ==
      sus::Slice<u16>::from({1_u16, 10_u16, 100_u16, 1000_u16, 10000_u16}));
}

}  // namespace
