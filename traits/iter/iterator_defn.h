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

#include "option/option.h"

namespace sus::traits::iter {

using ::sus::mem::__private::relocate_one_by_memcpy_v;
using ::sus::option::Option;

// TODO: Move forward decls somewhere?
template <class Item, size_t InnerIterSize, size_t InnerIterAlign>
class Filter;

template <class IteratorBase>
class IteratorLoop;
struct IteratorEnd;

template <class ItemT>
class IteratorBase {
 public:
  using Item = ItemT;

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
  /// short-circuiting; it stops iterating on the first `true` returned from
  /// the predicate.
  ///
  /// Returns `false` if the iterator is empty.
  virtual bool any(std::function<bool(Item)> f) noexcept;

  /// Consumes the iterator, and returns the number of elements that were in
  /// it.
  ///
  /// The function walks the iterator until it sees an Option holding #None.
  ///
  /// # Safety
  ///
  /// If the `usize` type does not have trapping arithmetic enabled, and the
  /// iterator has more than `usize::MAX` elements in it, the value will wrap
  /// and be incorrect. Otherwise, `usize` will catch overflow and panic.
  virtual /* TODO: usize */ size_t count() noexcept;

  /// Adaptor for use in ranged for loops.
  IteratorLoop<IteratorBase<Item>> begin() & noexcept;
  /// Adaptor for use in ranged for loops.
  IteratorEnd end() & noexcept;

 protected:
  IteratorBase() = default;
};

template <class I>
class Iterator final : public I {
 private:
  friend I;  // I::foo() can construct Iterator<I>.

  template <class J>
  friend class Iterator;  // Iterator<J>::foo() can construct Iterator<I>.

  // Option can't include Iterator, due to a circular dependency between
  // Option->Iterator->Option. So it forward declares Iterator, and needs
  // to use the constructor directly.
  template <class T>
  friend class Option;  // Option<T>::foo() can construct Iterator<I>.

  template <class... Args>
  Iterator(Args&&... args) : I(static_cast<Args&&>(args)...) {
    // We want to be able to use Iterator<I> and I interchangably, so that if an
    // `I` gets stored in SizedIterator, it doesn't misbehave.
    static_assert(sizeof(I) == sizeof(Iterator<I>), "");
  }

 public:
  // Adaptor methods.

  Iterator<Filter<typename I::Item, sizeof(I), alignof(I)>> filter(
      std::function<bool(const std::remove_reference_t<typename I::Item>&)>
          pred) && noexcept;
};

}  // namespace sus::traits::iter
