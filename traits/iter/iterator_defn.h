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

#pragma once

#include <functional>

#include "marker/unsafe.h"
#include "option/option.h"

namespace sus::traits::iter {

using ::sus::option::Option;

template <class Item>
class Iterator;

struct IteratorEnd {};
extern const IteratorEnd iterator_end;

template <class Item>
struct IteratorStep {
  Option<Item>& item_;
  Iterator<Item>& iter_;

  inline bool operator==(const IteratorEnd&) const { return item_.is_nome(); }
  inline bool operator!=(const IteratorEnd&) const { return item_.is_some(); }
  inline void operator++() & { item_ = iter_.next(); }
  inline Item operator*() & { return item_.take().unwrap(); }
};

template <class Item>
class Iterator {
 public:
  Iterator(Iterator&&) = default;
  Iterator& operator=(Iterator&&) = default;

  // Required methods.

  /// Gets the next element from the iterator, if there is one. Otherwise, it
  /// returns an Option holding #None.
  virtual Option<Item> next() noexcept = 0;

  // Provided methods.

  /// Tests whether all elements of the iterator match a predicate.
  ///
  /// If the predicate returns `true` for all elements in the iterator, this
  /// functions returns `true`, otherwise `false`. The function is
  /// short-circuiting; it stops iterating on the first `false` returned from
  /// the predicate.
  ///
  /// Returns `true` if the iterator is empty.
  virtual bool all(std::function<bool(Item)> f) noexcept;

  /// Tests whether any elements of the iterator match a predicate.
  ///
  /// If the predicate returns `true` for any elements in the iterator, this
  /// functions returns `true`, otherwise `false`. The function is
  /// short-circuiting; it stops iterating on the first `true` returned from the
  /// predicate.
  ///
  /// Returns `false` if the iterator is empty.
  virtual bool any(std::function<bool(Item)> f) noexcept;

  // Adaptors for range-based for loops.
  IteratorStep<Item> begin() & noexcept { return IteratorStep{item_, *this}; }
  IteratorEnd end() & noexcept { return iterator_end; }

 protected:
  /// The usual way for a subclass to initialize an Iterator. It must provide
  /// the first item when initializing. If it provides a None, the iterator will
  /// be empty.
  Iterator(Option<Item>&& first) noexcept
      : item_(static_cast<decltype(first)&&>(first)) {}

  enum class SubclassWillPopulate { FirstItem };
  /// The caller of this constructor is responsible for calling
  /// <PopulateFirstItem>() before using the Iterator, or the Iterator will
  /// never return anything. It can usually do so by calling
  /// `Iterator<Item>::PopulateFirstItem(next())`.
  Iterator(SubclassWillPopulate) : item_(Option<Item>::none()) {}

  void PopulateFirstItem(Option<Item>&& first) {
    item_ = static_cast<decltype(first)&&>(first);
  }

 private:
  Option<Item> item_;
};

}  // namespace sus::traits::iter
