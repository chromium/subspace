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

#include "traits/iter/iterator.h"

#include "assertions/unreachable.h"
#include "containers/array.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

using ::sus::containers::Array;
using ::sus::option::Option;
using ::sus::traits::iter::Iterator;

namespace {

template <class Item, size_t N>
class ArrayIterator : public Iterator<Item> {
 public:
  ArrayIterator(Item (&items)[N])
      : Iterator<Item>(Iterator<Item>::SubclassWillPopulate::FirstItem),
        items_(Array<Option<Item>, N>::with_initializer(
            [&items, i = 0]() mutable -> Option<Item> {
              return Option<Item>::some(items[i++]);
            })) {
    Iterator<Item>::PopulateFirstItem(next());
  }

  Option<Item> next() noexcept final {
    if (++index_ < N) {
      return items_.get_mut(index_).take();
    } else {
      --index_;
      return Option<Item>::none();
    }
  }

 private:
  size_t index_ = static_cast<size_t>(-1);
  Array<Option<Item>, N> items_;
};

template <class Item>
class EmptyIterator : public Iterator<Item> {
 public:
  EmptyIterator() : Iterator<Item>(Option<Item>::none()) {}

  Option<Item> next() noexcept final { ::sus::unreachable(); }
};

TEST(IteratorAll, All) {
  {
    int nums[5] = {1, 2, 3, 4, 5};
    ArrayIterator<int, 5> it(nums);
    EXPECT_TRUE(it.all([](int i) { return i <= 5; }));
  }
  {
    int nums[5] = {1, 2, 3, 4, 5};
    ArrayIterator<int, 5> it(nums);
    EXPECT_FALSE(it.all([](int i) { return i <= 4; }));
  }
  {
    int nums[5] = {1, 2, 3, 4, 5};
    ArrayIterator<int, 5> it(nums);
    EXPECT_FALSE(it.all([](int i) { return i <= 0; }));
  }

  // Shortcuts at the first failure.
  {
    int nums[5] = {1, 2, 3, 4, 5};
    ArrayIterator<int, 5> it(nums);
    EXPECT_FALSE(it.all([](int i) { return i <= 3; }));
    Option<int> n = it.next();
    ASSERT_TRUE(n.is_some());
    // The `all()` function stopped when it consumed 4, so we can still consume
    // 5.
    EXPECT_EQ(n.as_ref().unwrap(), 5);
  }
}

TEST(IteratorAll, EmptyIsTrue) {
  {
    EmptyIterator<int> it;
    EXPECT_TRUE(it.all([](int i) { return false; }));
  }
}

}  // namespace
