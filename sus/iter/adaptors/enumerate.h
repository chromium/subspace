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

// IWYU pragma: private, include "sus/iter/iterator.h"
// IWYU pragma: friend "sus/.*"
#pragma once

#include "sus/iter/iterator_defn.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"
#include "sus/tuple/tuple.h"

namespace sus::iter {

/// An iterator that yields the current count and the element during iteration.
///
/// This type is returned from `Iterator::enumerate()`.
template <class InnerSizedIter>
class [[nodiscard]] Enumerate final
    : public IteratorBase<Enumerate<InnerSizedIter>,
                          ::sus::Tuple<usize, typename InnerSizedIter::Item>> {
  using FromItem = typename InnerSizedIter::Item;

 public:
  using Item = ::sus::Tuple<usize, FromItem>;

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    Option<typename InnerSizedIter::Item> item = next_iter_.next();
    if (item.is_none()) {
      return sus::none();
    } else {
      usize count = count_;
      count_ += 1u;
      return sus::some(sus::tuple(
          count, sus::move(item).unwrap_unchecked(::sus::marker::unsafe_fn)));
    }
  }

  /// sus::iter::Iterator trait.
  SizeHint size_hint() const noexcept { return next_iter_.size_hint(); }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept
    requires(DoubleEndedIterator<InnerSizedIter, FromItem> &&
             ExactSizeIterator<InnerSizedIter, FromItem>)
  {
    Option<FromItem> item = next_iter_.next_back();
    if (item.is_none()) {
      return sus::none();
    } else {
      usize len = next_iter_.exact_size_hint();
      // Can safely add, `ExactSizeIterator` promises that the number of
      // elements fits into a `usize`.
      return sus::some(sus::tuple(
          count_ + len,
          sus::move(item).unwrap_unchecked(::sus::marker::unsafe_fn)));
    }
  }

  // sus::iter::ExactSizeIterator trait.
  usize exact_size_hint() const noexcept
    requires(ExactSizeIterator<InnerSizedIter, FromItem>)
  {
    return next_iter_.exact_size_hint();
  }

  // TODO: Implement nth(), nth_back(), etc...

 private:
  template <class U, class V>
  friend class IteratorBase;

  static Enumerate with(InnerSizedIter&& next_iter) noexcept {
    return Enumerate(::sus::move(next_iter));
  }

  Enumerate(InnerSizedIter&& next_iter) : next_iter_(::sus::move(next_iter)) {}

  usize count_ = 0u;
  InnerSizedIter next_iter_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(next_iter_));
};

}  // namespace sus::iter
