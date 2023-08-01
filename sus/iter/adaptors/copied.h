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
#include "sus/iter/sized_iterator.h"
#include "sus/mem/clone.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"

namespace sus::iter {

/// An iterator that copied the elements of an underlying iterator.
///
/// This type is returned from `Iterator::copied()`.
template <class InnerSizedIter>
class [[nodiscard]] [[sus_trivial_abi]] Copied final
    : public IteratorBase<Copied<InnerSizedIter>,
                          std::remove_cvref_t<typename InnerSizedIter::Item>> {
 public:
  using Item = std::remove_cvref_t<typename InnerSizedIter::Item>;

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    return next_iter_.next().map([](const Item& item) -> Item { return item; });
  }

  /// sus::iter::Iterator trait.
  ::sus::iter::SizeHint size_hint() const noexcept {
    return next_iter_.size_hint();
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept
    requires(InnerSizedIter::DoubleEnded)
  {
    return next_iter_.next_back().map(
        [](const Item& item) -> Item { return item; });
  }

  // sus::iter::ExactSizeIterator trait.
  usize exact_size_hint() const noexcept
    requires(InnerSizedIter::ExactSize)
  {
    return next_iter_.exact_size_hint();
  }

 private:
  template <class U, class V>
  friend class IteratorBase;

  static Copied with(InnerSizedIter && next_iter) noexcept {
    return Copied(::sus::move(next_iter));
  }

  Copied(InnerSizedIter && next_iter) : next_iter_(::sus::move(next_iter)) {}

  InnerSizedIter next_iter_;

  // The InnerSizedIter is trivially relocatable.
  sus_class_trivially_relocatable(::sus::marker::unsafe_fn,
                                  decltype(next_iter_));
};

}  // namespace sus::iter
