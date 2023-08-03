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
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"

namespace sus::iter {

using ::sus::mem::relocate_by_memcpy;

/// An iterator that skips over `n` elements of another iterator.
///
/// This type is returned from `Iterator::skip()`.
template <class InnerSizedIter>
class [[nodiscard]] [[sus_trivial_abi]] Skip final
    : public IteratorBase<Skip<InnerSizedIter>, typename InnerSizedIter::Item> {
 public:
  using Item = InnerSizedIter::Item;

  // sus::mem::Clone trait.
  Skip clone() const noexcept
    requires(::sus::mem::Clone<InnerSizedIter>)
  {
    return Skip(skip_, ::sus::clone(next_iter_));
  }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    while (skip_ > 0u) {
      next_iter_.next();
      skip_ -= 1u;
    }
    return next_iter_.next();
  }
  /// sus::iter::Iterator trait.
  ::sus::iter::SizeHint size_hint() const noexcept {
    auto [lower, upper] = next_iter_.size_hint();
    lower = lower.saturating_sub(skip_);
    upper =
        upper.map([this](usize upper) { return upper.saturating_sub(skip_); });
    return {lower, upper};
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept
    requires(DoubleEndedIterator<InnerSizedIter, Item> &&  //
             ExactSizeIterator<InnerSizedIter, Item>)
  {
    if (exact_size_hint() > 0u) {
      return next_iter_.next_back();
    } else {
      return Option<Item>();
    }
  }

  // sus::iter::ExactSizeIterator trait.
  usize exact_size_hint() const noexcept {
    return next_iter_.exact_size_hint().saturating_sub(skip_);
  }

 private:
  template <class U, class V>
  friend class IteratorBase;

  static Skip with(usize n, InnerSizedIter && next_iter) noexcept {
    return Skip(n, ::sus::move(next_iter));
  }

  Skip(usize n, InnerSizedIter && next_iter) noexcept
      : skip_(n), next_iter_(::sus::move(next_iter)) {}

  usize skip_;
  InnerSizedIter next_iter_;

  // The InnerSizedIter and usize are trivially relocatable.
  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(skip_),
                                  decltype(next_iter_));
};

}  // namespace sus::iter
