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

#include "sus/collections/compat_map.h"

#include <tuple>

#include "googletest/include/gtest/gtest.h"
#include "sus/iter/compat_ranges.h"
#include "sus/prelude.h"

namespace {

// For any `usize`.
static_assert(
    sus::collections::compat::Pair<std::tuple<usize, usize>, usize, usize>);

// For any `usize`.
static_assert(
    sus::iter::FromIterator<std::map<usize, usize>, std::tuple<usize, usize>>);

TEST(CompatMap, FromIterator) {
  auto in =
      std::vector<std::tuple<i32, u32>>{{3, 4u}, {4, 5u}, {5, 6u}, {6, 7u}};
  auto out = sus::iter::from_range(sus::move(in))
                 .filter([](const std::tuple<i32, u32>& i) {
                   return get<0>(i) % 2 == 0;
                 })
                 .collect<std::map<i32, u32>>();
  sus_check(out == std::map<i32, u32>{{4, 5u}, {6, 7u}});
}

TEST(CompatMap, FromIteratorSusTuple) {
  auto in =
      std::vector<sus::Tuple<i32, u32>>{sus::tuple(3, 4u), sus::tuple(4, 5u),
                                        sus::tuple(5, 6u), sus::tuple(6, 7u)};
  auto out = sus::iter::from_range(sus::move(in))
                 .filter([](const sus::Tuple<i32, u32>& i) {
                   return i.at<0>() % 2 == 0;
                 })
                 .collect<std::map<i32, u32>>();
  sus_check(out == std::map<i32, u32>{{4, 5u}, {6, 7u}});
}

TEST(CompatMultiMap, FromIterator) {
  auto in = std::vector<std::tuple<i32, u32>>{{3, 4u}, {4, 5u}, {4, 4u},
                                              {5, 6u}, {6, 7u}, {4, 6u}};
  auto out = sus::iter::from_range(sus::move(in))
                 .filter([](const std::tuple<i32, u32>& i) {
                   return get<0>(i) % 2 == 0;
                 })
                 .collect<std::multimap<i32, u32>>();
  sus_check(out ==
             std::multimap<i32, u32>{{4, 5u}, {4, 4u}, {6, 7u}, {4, 6u}});
}

}  // namespace
