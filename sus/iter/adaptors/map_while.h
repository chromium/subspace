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

/// An iterator that maps each item to a new type based on a map function.
///
/// This type is returned from `Iterator::map()`.
template <class ToItem, class InnerSizedIter, class MapFn>
class [[nodiscard]] MapWhile final
    : public IteratorBase<MapWhile<ToItem, InnerSizedIter, MapFn>, ToItem> {
  using FromItem = InnerSizedIter::Item;
  static_assert(::sus::fn::FnMut<MapFn, ::sus::Option<ToItem>(FromItem&&)>);

 public:
  using Item = ToItem;

  // Type is Move and (can be) Clone.
  MapWhile(MapWhile&&) = default;
  MapWhile& operator=(MapWhile&&) = default;

  // sus::mem::Clone trait.
  constexpr MapWhile clone() const noexcept
    requires(::sus::mem::Clone<MapFn> &&  //
             ::sus::mem::Clone<InnerSizedIter>)
  {
    return MapWhile(::sus::clone(fn_), ::sus::clone(next_iter_));
  }

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept {
    Option<FromItem> item = next_iter_.next();
    if (item.is_none()) {
      return sus::none();
    } else {
      return ::sus::fn::call_mut(
          fn_, sus::move(item).unwrap_unchecked(::sus::marker::unsafe_fn));
    }
  }

  /// sus::iter::Iterator trait.
  constexpr SizeHint size_hint() const noexcept {
    // Can't know a lower bound, due to the predicate.
    return SizeHint(0u, next_iter_.size_hint().upper);
  }

  // sus::iter::DoubleEndedIterator trait.
  constexpr Option<Item> next_back() noexcept
    requires(DoubleEndedIterator<InnerSizedIter, FromItem>)
  {
    Option<FromItem> item = next_iter_.next_back();
    if (item.is_none()) {
      return sus::none();
    } else {
      return ::sus::fn::call_mut(
          fn_, sus::move(item).unwrap_unchecked(::sus::marker::unsafe_fn));
    }
  }

 private:
  template <class U, class V>
  friend class IteratorBase;

  explicit constexpr MapWhile(MapFn fn, InnerSizedIter&& next_iter)
      : fn_(::sus::move(fn)), next_iter_(::sus::move(next_iter)) {}

  MapFn fn_;
  InnerSizedIter next_iter_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(fn_), decltype(next_iter_));
};

}  // namespace sus::iter
