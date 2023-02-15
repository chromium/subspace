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

#include "subspace/fn/fn_defn.h"
#include "subspace/iter/iterator_defn.h"
#include "subspace/iter/sized_iterator.h"
#include "subspace/mem/move.h"
#include "subspace/mem/relocate.h"

namespace sus::iter {

template <class FromItem, class ToItem, size_t InnerIterSize,
          size_t InnerIterAlign>
class Map final
    : public IteratorImpl<Map<FromItem, ToItem, InnerIterSize, InnerIterAlign>,
                          ToItem> {
  using MapFn = ::sus::fn::FnMut<ToItem(FromItem&&)>;
  using InnerSizedIter = SizedIterator<FromItem, InnerIterSize, InnerIterAlign>;

 public:
  using Item = ToItem;

  static Map with(MapFn fn, InnerSizedIter&& next_iter) noexcept {
    return Map(::sus::move(fn), ::sus::move(next_iter));
  }

  Option<Item> next() noexcept final {
    Option<FromItem> item = next_iter_.iterator_mut().next();
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
  // known to be trivially relocatable because the FnMut will either be a
  // function pointer or a heap allocation itself.
  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(fn_),
                                  decltype(next_iter_));
};

}  // namespace sus::iter
