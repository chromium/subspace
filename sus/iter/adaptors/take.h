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

namespace sus::iter {

using ::sus::mem::relocate_by_memcpy;

/// An iterator that only iterates over the first n iterations of another
/// iterator.
///
/// This type is returned from `Iterator::take()`.
template <class InnerSizedIter>
class [[nodiscard]] Take final
    : public IteratorBase<Take<InnerSizedIter>, typename InnerSizedIter::Item> {
 public:
  using Item = InnerSizedIter::Item;

  // The type is Move and Clone.
  Take(Take&&) = default;
  Take& operator=(Take&&) = default;

  // sus::mem::Clone trait.
  constexpr Take clone() const noexcept
    requires(::sus::mem::Clone<InnerSizedIter>)
  {
    return Take(n_, ::sus::clone(next_iter_));
  }

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept {
    if (n_ != 0u) {
      n_ -= 1u;
      return next_iter_.next();
    } else {
      return Option<Item>();
    }
  }
  /// sus::iter::Iterator trait.
  constexpr SizeHint size_hint() const noexcept {
    if (n_ == 0u) {
      return {0u, ::sus::some(0u)};
    }

    auto [lower, upper] = next_iter_.size_hint();

    lower = ::sus::ops::min(lower, n_);
    upper = ::sus::move(upper)
                .map([this](usize upper) { return ::sus::ops::min(upper, n_); })
                .or_else([this] { return ::sus::some(n_); });

    return {lower, upper};
  }

  // sus::iter::DoubleEndedIterator trait.
  constexpr Option<Item> next_back() noexcept
    requires(DoubleEndedIterator<InnerSizedIter, Item> &&  //
             ExactSizeIterator<InnerSizedIter, Item>)
  {
    if (n_ == 0u) {
      return Option<Item>();
    } else {
      usize skip = next_iter_.exact_size_hint().saturating_sub(n_);
      n_ -= 1u;
      // If SizedIterator was an Iterator we could use nth_back(skip).
      while (true) {
        if (skip == 0u) return next_iter_.next_back();
        if (next_iter_.next_back().is_none()) return Option<Item>();
        skip -= 1u;
      }
    }
  }

  // sus::iter::ExactSizeIterator trait.
  constexpr usize exact_size_hint() const noexcept {
    return ::sus::ops::min(next_iter_.exact_size_hint(), n_);
  }

 private:
  template <class U, class V>
  friend class IteratorBase;

  static constexpr Take with(usize n, InnerSizedIter&& next_iter) noexcept {
    return Take(n, ::sus::move(next_iter));
  }

  constexpr Take(usize n, InnerSizedIter&& next_iter) noexcept
      : n_(n), next_iter_(::sus::move(next_iter)) {}

  usize n_;
  InnerSizedIter next_iter_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(n_), decltype(next_iter_));
};

}  // namespace sus::iter
