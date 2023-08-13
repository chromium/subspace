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

#include "sus/collections/compat_queue.h"

#include "googletest/include/gtest/gtest.h"
#include "sus/iter/compat_ranges.h"
#include "sus/prelude.h"

namespace {

// For any `usize`.
static_assert(sus::iter::FromIterator<std::queue<usize>, usize>);
static_assert(sus::iter::FromIterator<std::priority_queue<usize>, usize>);

TEST(CompatQueue, FromIterator) {
  auto in = std::vector<i32>{1, 2, 3, 4, 5, 6, 7};
  auto out = sus::iter::from_range(sus::move(in))
                 .filter([](const i32& i) { return i % 2 == 0; })
                 .collect<std::queue<i32>>();
  static_assert(std::same_as<decltype(out), std::queue<i32>>);
  sus::check(out == std::queue<i32>(std::deque<i32>{2, 4, 6}));
}

TEST(CompatQueue, FromIteratorRef) {
  auto in = std::vector<i32>{1, 2, 3, 4, 5, 6, 7};
  auto out = sus::iter::from_range(sus::move(in))
                 .filter([](const i32& i) { return i % 2 == 0; })
                 .rev()
                 .collect<std::queue<i32>>();
  static_assert(std::same_as<decltype(out), std::queue<i32>>);
  sus::check(out == std::queue<i32>(std::deque<i32>{6, 4, 2}));
}

TEST(CompatPriorityQueue, FromIterator) {
  auto in = std::vector<i32>{1, 2, 3, 4, 5, 6, 7};
  auto out = sus::iter::from_range(sus::move(in))
                 .filter([](const i32& i) { return i % 2 == 0; })
                 .collect<std::priority_queue<i32>>();
  static_assert(std::same_as<decltype(out), std::priority_queue<i32>>);
  auto cmp =
      std::priority_queue<i32>(std::less<i32>(), std::vector<i32>{2, 4, 6});
  sus::check(out.top() == cmp.top());
  out.pop(), cmp.pop();
  sus::check(out.top() == cmp.top());
  out.pop(), cmp.pop();
  sus::check(out.top() == cmp.top());
  out.pop(), cmp.pop();
  sus::check(out.empty() && cmp.empty());
}

TEST(CompatPriorityQueue, FromIteratorRef) {
  auto in = std::vector<i32>{1, 2, 3, 4, 5, 6, 7};
  auto out = sus::iter::from_range(sus::move(in))
                 .filter([](const i32& i) { return i % 2 == 0; })
                 .rev()
                 .collect<std::priority_queue<i32>>();
  static_assert(std::same_as<decltype(out), std::priority_queue<i32>>);
  auto cmp =
      std::priority_queue<i32>(std::less<i32>(), std::vector<i32>{2, 4, 6});
  sus::check(out.top() == cmp.top());
  out.pop(), cmp.pop();
  sus::check(out.top() == cmp.top());
  out.pop(), cmp.pop();
  sus::check(out.top() == cmp.top());
  out.pop(), cmp.pop();
  sus::check(out.empty() && cmp.empty());
}

}  // namespace
