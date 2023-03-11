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

#include "subspace/fn/fn_box_defn.h"
#include "subspace/iter/iterator_defn.h"
#include "subspace/iter/sized_iterator.h"
#include "subspace/mem/move.h"
#include "subspace/mem/relocate.h"

namespace sus::iter {

using ::sus::mem::relocate_by_memcpy;

template <class InnerSizedIter>
class [[nodiscard]] [[sus_trivial_abi]] Filter final
    : public IteratorBase<Filter<InnerSizedIter>,
                          typename InnerSizedIter::Item> {
  using Pred = ::sus::fn::FnMutBox<bool(
      // TODO: write a sus::const_ref<T>?
      const std::remove_reference_t<typename InnerSizedIter::Item>&)>;

 public:
  using Item = InnerSizedIter::Item;

  static Filter with(Pred&& pred, InnerSizedIter&& next_iter) noexcept {
    return Filter(::sus::move(pred), ::sus::move(next_iter));
  }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    InnerSizedIter& iter = next_iter_;
    Pred& pred = pred_;

    // TODO: Just call find(pred) on itself?
    while (true) {
      Option<Item> item = iter.next();
      if (item.is_none() ||
          pred(item.as_ref().unwrap_unchecked(::sus::marker::unsafe_fn)))
        return item;
    }
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept
    requires(InnerSizedIter::DoubleEnded)
  {
    InnerSizedIter& iter = next_iter_;
    Pred& pred = pred_;

    // TODO: Just call find(pred) on itself?
    while (true) {
      Option<Item> item = iter.next_back();
      if (item.is_none() ||
          pred(item.as_ref().unwrap_unchecked(::sus::marker::unsafe_fn)))
        return item;
    }
  }

 private:
  Filter(Pred&& pred, InnerSizedIter&& next_iter)
      : pred_(::sus::move(pred)), next_iter_(::sus::move(next_iter)) {}

  Pred pred_;
  InnerSizedIter next_iter_;

  // The InnerSizedIter is trivially relocatable. Likewise, the predicate is
  // known to be trivially relocatable because FnMut is.
  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(pred_),
                                  decltype(next_iter_));
};

}  // namespace sus::iter
