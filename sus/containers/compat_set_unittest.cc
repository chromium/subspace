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

#include "sus/containers/compat_set.h"

#include "googletest/include/gtest/gtest.h"
#include "sus/iter/compat_ranges.h"
#include "sus/option/option.h"
#include "sus/prelude.h"

namespace {

// For any `usize`.
static_assert(sus::iter::FromIterator<std::set<usize>, usize>);

TEST(CompatSet, FromIterator) {
  auto in = std::vector<i32>{3, 4, 2, 7, 6, 1, 5};
  auto out = sus::iter::from_range(sus::move(in))
                 .filter([](const i32& i) { return i % 2 == 0; })
                 .collect<std::set<i32>>();
  static_assert(std::same_as<decltype(out), std::set<i32>>);
  sus::check(out == std::set<i32>{2, 4, 6});
}

TEST(CompaMultiSet, FromIterator) {
  auto in = std::vector<i32>{3, 4, 2, 7, 6, 2, 1, 5, 2};
  auto out = sus::iter::from_range(sus::move(in))
                 .filter([](const i32& i) { return i % 2 == 0; })
                 .collect<std::multiset<i32>>();
  static_assert(std::same_as<decltype(out), std::multiset<i32>>);
  sus::check(out == std::multiset<i32>{2, 2, 2, 4, 6});
}

}  // namespace
