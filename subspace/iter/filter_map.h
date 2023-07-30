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

/// An iterator that uses a function to both filter and map elements from
/// another `Iterator`.
///
/// This type is returned from `Iterator::filter_map()`.
template <class ToItem, class InnerSizedIter>
class [[nodiscard]] [[sus_trivial_abi]] FilterMap final
    : public IteratorBase<FilterMap<ToItem, InnerSizedIter>, ToItem> {
  using FromItem = InnerSizedIter::Item;
  using FilterMapFn = ::sus::fn::FnMutBox<::sus::Option<ToItem>(FromItem&&)>;

 public:
  using Item = ToItem;

  // sus::mem::Clone trait.
  FilterMap clone() const noexcept
    requires(InnerSizedIter::Clone)
  {
    return FilterMap(sus::clone(fn_), sus::clone(next_iter_));
  }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    while (true) {
      Option<FromItem> in = next_iter_.next();
      Option<ToItem> out;
      if (in.is_none()) return out;
      out = ::sus::fn::call_mut(fn_, sus::move(in).unwrap());
      if (out.is_some()) return out;
    }
  }

  /// sus::iter::Iterator trait.
  ::sus::iter::SizeHint size_hint() const noexcept {
    // Can't know a lower bound, due to the filter function.
    return ::sus::iter::SizeHint(0u, next_iter_.size_hint().upper);
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept
    requires(InnerSizedIter::DoubleEnded)
  {
    while (true) {
      Option<FromItem> in = next_iter_.next_back();
      Option<ToItem> out;
      if (in.is_none()) return out;
      out = ::sus::fn::call_mut(fn_, sus::move(in).unwrap());
      if (out.is_some()) return out;
    }
  }

 private:
  template <class U, class V>
  friend class IteratorBase;

  static FilterMap with(FilterMapFn&& fn, InnerSizedIter&& next_iter) noexcept {
    return FilterMap(::sus::move(fn), ::sus::move(next_iter));
  }

  FilterMap(FilterMapFn&& fn, InnerSizedIter&& next_iter) noexcept
      : fn_(::sus::move(fn)), next_iter_(::sus::move(next_iter)) {}

  FilterMapFn fn_;
  InnerSizedIter next_iter_;

  // The InnerSizedIter is trivially relocatable. Likewise, the FilterMapFn
  // function is known to be trivially relocatable because FnMutBox is.
  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(fn_),
                                  decltype(next_iter_));
};

}  // namespace sus::iter
