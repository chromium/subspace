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

#include "subspace/containers/compat_deque.h"

#include "googletest/include/gtest/gtest.h"
#include "subspace/iter/compat_ranges.h"
#include "subspace/prelude.h"

namespace {

// For any `usize`.
static_assert(sus::iter::FromIterator<std::deque<usize>, usize>);

TEST(CompatDeque, FromIterator) {
  auto in = std::deque<i32>{1, 2, 3, 4, 5, 6, 7};
  auto out = sus::iter::from_range(sus::move(in))
                 .filter([](const i32& i) { return i % 2 == 0; })
                 .collect<std::deque<i32>>();
  sus::check(out == std::deque<i32>{2, 4, 6});
}

}  // namespace
