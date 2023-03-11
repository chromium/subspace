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

template <class ToItem, class InnerSizedIter>
class [[nodiscard]] [[sus_trivial_abi]] Map final : public IteratorBase<Map<ToItem, InnerSizedIter>, ToItem> {
  using FromItem = InnerSizedIter::Item;
  using MapFn = ::sus::fn::FnMutBox<ToItem(FromItem&&)>;

 public:
  using Item = ToItem;

  static Map with(MapFn fn, InnerSizedIter&& next_iter) noexcept {
    return Map(::sus::move(fn), ::sus::move(next_iter));
  }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    Option<FromItem> item = next_iter_.next();
    if (item.is_none()) {
      return sus::none();
    } else {
      return sus::some(
          fn_(sus::move(item).unwrap_unchecked(::sus::marker::unsafe_fn)));
    }
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept
    requires(InnerSizedIter::DoubleEnded)
  {
    Option<FromItem> item = next_iter_.next_back();
    if (item.is_none()) {
      return sus::none();
    } else {
      return sus::some(
          fn_(sus::move(item).unwrap_unchecked(::sus::marker::unsafe_fn)));
    }
  }

 private:
  Map(MapFn fn, InnerSizedIter&& next_iter)
      : fn_(::sus::move(fn)), next_iter_(::sus::move(next_iter)) {}

  MapFn fn_;
  InnerSizedIter next_iter_;

  // The InnerSizedIter is trivially relocatable. Likewise, the predicate is
  // known to be trivially relocatable because the FnMutBox will either be a
  // function pointer or a heap allocation itself.
  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(fn_),
                                  decltype(next_iter_));
};

}  // namespace sus::iter
