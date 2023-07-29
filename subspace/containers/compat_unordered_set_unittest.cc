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

#include "subspace/containers/compat_unordered_set.h"

#include "googletest/include/gtest/gtest.h"
#include "subspace/iter/compat_ranges.h"
#include "subspace/option/option.h"
#include "subspace/prelude.h"

namespace {

// For any `usize`.
static_assert(sus::iter::FromIterator<std::unordered_set<usize>, usize>);

TEST(CompatUnorderedSet, FromIterator) {
  auto in = std::vector<i32>{3, 4, 2, 7, 6, 1, 5};
  auto out = sus::iter::from_range(sus::move(in))
                 .filter([](const i32& i) { return i % 2 == 0; })
                 .collect<std::unordered_set<i32>>();
  static_assert(std::same_as<decltype(out), std::unordered_set<i32>>);
  sus::check(out == std::unordered_set<i32>{2, 4, 6});
}

TEST(CompatUnorderedSet, Options) {
  auto with_none = std::vector<sus::Option<i32>>{
      sus::some(3), sus::some(4), sus::some(2), sus::some(7),
      sus::none(),  // None is present, so output is None.
      sus::some(6), sus::some(1), sus::some(5)};
  auto out_with_none = sus::iter::from_range(sus::move(with_none))
                           .collect<sus::Option<std::unordered_set<i32>>>();
  static_assert(std::same_as<decltype(out_with_none),
                             sus::Option<std::unordered_set<i32>>>);
  sus::check(out_with_none.is_none());

  auto all_some = std::vector<sus::Option<i32>>{
      sus::some(3), sus::some(4), sus::some(2), sus::some(7),
      sus::some(6), sus::some(1), sus::some(5)};
  auto out_all_some = sus::iter::from_range(sus::move(all_some))
                          .collect<sus::Option<std::unordered_set<i32>>>();
  static_assert(std::same_as<decltype(out_all_some),
                             sus::Option<std::unordered_set<i32>>>);
  sus::check(out_all_some ==
             sus::some(std::unordered_set<i32>{3, 4, 2, 7, 6, 1, 5}));
}

}  // namespace
