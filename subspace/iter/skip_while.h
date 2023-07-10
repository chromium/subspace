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

#pragma once

#include "subspace/fn/fn_box_defn.h"
#include "subspace/iter/iterator_defn.h"
#include "subspace/iter/sized_iterator.h"
#include "subspace/mem/move.h"
#include "subspace/mem/relocate.h"

namespace sus::iter {

using ::sus::mem::relocate_by_memcpy;

/// An iterator that rejects elements while `pred` returns `true`.
///
/// This type is returned from `Iterator::skip()`.
template <class InnerSizedIter>
class [[nodiscard]] [[sus_trivial_abi]] SkipWhile final
    : public IteratorBase<SkipWhile<InnerSizedIter>,
                          typename InnerSizedIter::Item> {
  using Pred = ::sus::fn::FnMutBox<bool(
      // TODO: write a sus::const_ref<T>?
      const std::remove_reference_t<typename InnerSizedIter::Item>&)>;

 public:
  using Item = InnerSizedIter::Item;

  // sus::mem::Clone trait.
  SkipWhile clone() const noexcept
    requires(::sus::mem::Clone<Pred> &&  //
             InnerSizedIter::Clone)
  {
    return SkipWhile(::sus::clone(pred_), ::sus::clone(next_iter_));
  }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    while (true) {
      Option<Item> out = next_iter_.next();
      if (out.is_none() || pred_.is_none()) return out;
      // SAFETY: `out` and `pred_` are both checked for None above.
      if (!pred_.as_value_unchecked_mut(::sus::marker::unsafe_fn)(
              out.as_value_unchecked(::sus::marker::unsafe_fn))) {
        pred_ = Option<Pred>();
        return out;
      }
      // `pred_` returned true, skip this `out`.
    }
  }

  /// sus::iter::Iterator trait.
  ::sus::iter::SizeHint size_hint() const noexcept {
    // No lower bound is known, as we don't know how many will be skipped.
    return {0u, next_iter_.size_hint().upper};
  }

 private:
  template <class U, class V>
  friend class IteratorBase;

  static SkipWhile with(Pred && pred, InnerSizedIter && next_iter) noexcept {
    return SkipWhile(::sus::move(pred), ::sus::move(next_iter));
  }

  SkipWhile(Pred && pred, InnerSizedIter && next_iter) noexcept
      : pred_(Option<Pred>::with(::sus::move(pred))),
        next_iter_(::sus::move(next_iter)) {}

  Option<Pred> pred_;
  InnerSizedIter next_iter_;

  // The InnerSizedIter and FnMutBox are trivially relocatable.
  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(pred_),
                                  decltype(next_iter_));
};

}  // namespace sus::iter
