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

#if TEST_MODULE
import sus;

#include "sus/assertions/check.h"
#else
#include "sus/collections/compat_forward_list.h"

#include "sus/iter/compat_ranges.h"
#include "sus/prelude.h"
#endif // TEST_MODULE

#include <forward_list>
#include "googletest/include/gtest/gtest.h"

namespace sus::test::compat_forward_list {

template <sus::iter::Iterator<i32> Iter>
struct SingleEnded final
    : public sus::iter::IteratorBase<SingleEnded<Iter>, i32> {
  SingleEnded(Iter it) : it_(sus::move(it)) {}

  using Item = i32;
  Option<i32> next() noexcept { return it_.next(); }
  sus::iter::SizeHint size_hint() const noexcept { return it_.size_hint(); }

 private:
  Iter it_;
};

template <class T>
SingleEnded(T) -> SingleEnded<T>;

}  // namespace sus::test::compat_forward_list

namespace {
using namespace sus::test::compat_forward_list;

// For any `usize`.
static_assert(sus::iter::FromIterator<std::forward_list<usize>, usize>);

TEST(CompatForwardList, FromIterator) {
  auto in = std::vector<i32>{1, 2, 3, 4, 5, 6, 7};
  static_assert(sus::iter::DoubleEndedIterator<
                decltype(sus::iter::from_range(sus::move(in))), i32&>);
  auto out = sus::iter::from_range(in)
                 .moved(unsafe_fn)
                 .filter([](const i32& i) { return i % 2 == 0; })
                 .collect<std::forward_list<i32>>();
  sus_check(out == std::forward_list<i32>{2, 4, 6});
}

TEST(CompatForwardList, FromIteratorNotDoubleEnded) {
  auto in = std::vector<i32>{1, 2, 3, 4, 5, 6, 7};
  auto it = SingleEnded(sus::iter::from_range(in).moved(unsafe_fn));
  static_assert(sus::iter::Iterator<decltype(it), i32>);
  static_assert(!sus::iter::DoubleEndedIterator<decltype(it), i32>);
  auto out = sus::move(it)
                 .filter([](const i32& i) { return i % 2 == 0; })
                 .collect<std::forward_list<i32>>();
  sus_check(out == std::forward_list<i32>{2, 4, 6});
}

}  // namespace
