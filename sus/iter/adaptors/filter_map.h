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

// IWYU pragma: private, include "sus/iter/iterator.h"
// IWYU pragma: friend "sus/.*"
#pragma once

#include "sus/fn/fn_concepts.h"
#include "sus/iter/iterator_defn.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"

namespace sus::iter {

using ::sus::mem::relocate_by_memcpy;

/// An iterator that uses a function to both filter and map elements from
/// another `Iterator`.
///
/// This type is returned from `Iterator::filter_map()`.
template <class ToItem, class InnerSizedIter, class FilterMapFn>
class [[nodiscard]] FilterMap final
    : public IteratorBase<FilterMap<ToItem, InnerSizedIter, FilterMapFn>,
                          ToItem> {
  using FromItem = InnerSizedIter::Item;
  static_assert(
      ::sus::fn::FnMut<FilterMapFn, ::sus::Option<ToItem>(FromItem&&)>);

 public:
  using Item = ToItem;

  // Type is Move and (can be) Clone.
  FilterMap(FilterMap&&) = default;
  FilterMap& operator=(FilterMap&&) = default;

  // sus::mem::Clone trait.
  constexpr FilterMap clone() const noexcept
    requires(::sus::mem::Clone<FilterMapFn> &&  //
             ::sus::mem::Clone<InnerSizedIter>)
  {
    return FilterMap(sus::clone(fn_), sus::clone(next_iter_));
  }

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept {
    while (true) {
      Option<FromItem> in = next_iter_.next();
      Option<ToItem> out;
      if (in.is_none()) return out;
      out = ::sus::fn::call_mut(fn_, sus::move(in).unwrap());
      if (out.is_some()) return out;
    }
  }

  /// sus::iter::Iterator trait.
  constexpr SizeHint size_hint() const noexcept {
    // Can't know a lower bound, due to the filter function.
    return SizeHint(0u, next_iter_.size_hint().upper);
  }

  // sus::iter::DoubleEndedIterator trait.
  constexpr Option<Item> next_back() noexcept
    requires(DoubleEndedIterator<InnerSizedIter, FromItem>)
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

  explicit constexpr FilterMap(FilterMapFn&& fn,
                               InnerSizedIter&& next_iter) noexcept
      : fn_(::sus::move(fn)), next_iter_(::sus::move(next_iter)) {}

  FilterMapFn fn_;
  InnerSizedIter next_iter_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(fn_), decltype(next_iter_));
};

}  // namespace sus::iter
