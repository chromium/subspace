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

/// An iterator that only accepts elements while `pred` returns `true`.
///
/// This type is returned from `Iterator::take()`.
template <class InnerSizedIter>
class [[nodiscard]] [[sus_trivial_abi]] TakeWhile final
    : public IteratorBase<TakeWhile<InnerSizedIter>,
                          typename InnerSizedIter::Item> {
  using Pred = ::sus::fn::FnMutBox<bool(
      // TODO: write a sus::const_ref<T>?
      const std::remove_reference_t<typename InnerSizedIter::Item>&)>;

 public:
  using Item = InnerSizedIter::Item;

  // sus::mem::Clone trait.
  TakeWhile clone() const noexcept
    requires(InnerSizedIter::Clone)
  {
    return TakeWhile(::sus::clone(pred_), ::sus::clone(next_iter_));
  }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    Option<Item> out;
    if (pred_.is_none()) return out;
    out = next_iter_.next();
    if (out.is_none()) return out;
    // SAFETY: `pred_` and `out` have each been checked for None already.
    if (!::sus::fn::call_mut(
            pred_.as_value_unchecked_mut(::sus::marker::unsafe_fn),
            out.as_value_unchecked(::sus::marker::unsafe_fn))) {
      pred_ = Option<Pred>();
      out = Option<Item>();
    }
    return out;
  }

  /// sus::iter::Iterator trait.
  ::sus::iter::SizeHint size_hint() const noexcept {
    if (pred_.is_none()) return {0u, sus::some(0u)};
    // Can't know a lower bound, due to the predicate.
    return {0u, next_iter_.size_hint().upper};
  }

 private:
  template <class U, class V>
  friend class IteratorBase;

  static TakeWhile with(Pred&& pred, InnerSizedIter&& next_iter) noexcept {
    return TakeWhile(::sus::move(pred), ::sus::move(next_iter));
  }

  TakeWhile(Pred&& pred, InnerSizedIter&& next_iter) noexcept
      : pred_(::sus::some(::sus::move(pred))),
        next_iter_(::sus::move(next_iter)) {}

  ::sus::Option<Pred> pred_;
  InnerSizedIter next_iter_;

  // The InnerSizedIter and usize are trivially relocatable.
  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(pred_),
                                  decltype(next_iter_));
};

}  // namespace sus::iter
