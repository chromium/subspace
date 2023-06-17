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
#include "subspace/mem/move.h"
#include "subspace/mem/relocate.h"

namespace sus::iter {

using ::sus::option::Option;

/// An Iterator that generates each item from a function that takes the previous
/// item.
template <class ItemT>
class [[nodiscard]] Successors final
    : public IteratorBase<Successors<ItemT>, ItemT> {
 public:
  using Item = ItemT;

  /// Creates a new iterator where each successive item is computed based on the
  /// preceding one.
  ///
  /// The iterator starts with the given `first` item (if any) and calls the
  /// given `FnMutBox<Option<Item>>(const Item&)` functor to compute each item's
  /// successor.
  ///
  /// # Example
  /// ```
  /// auto powers_of_10 = sus::iter::Successors<u16>::with(
  ///     sus::some(1_u16), [](const u16& n) { return n.checked_mul(10_u16); });
  /// sus::check(
  ///     sus::move(powers_of_10).collect<Vec<u16>>() ==
  ///     sus::Slice<u16>::from({1_u16, 10_u16, 100_u16, 1000_u16, 10000_u16}));
  /// ```
  static Successors<Item> with(
      Option<Item> first,
      ::sus::fn::FnMutBox<Option<Item>(const Item&)> func) noexcept {
    return Successors<Item>(::sus::move(first), ::sus::move(func));
  }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    Option<Item> item = next_.take();
    if (item.is_some()) next_ = func_(item.as_value());
    return item;
  }

 private:
  Successors(Option<Item> first,
             ::sus::fn::FnMutBox<Option<Item>(const Item&)>&& func)
      : next_(::sus::move(first)), func_(::sus::move(func)) {}

  Option<Item> next_;
  ::sus::fn::FnMutBox<Option<Item>(const Item&)> func_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(next_), decltype(func_));
};

}  // namespace sus::iter
