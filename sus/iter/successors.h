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

#include "sus/fn/fn_concepts.h"
#include "sus/iter/iterator_defn.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"

namespace sus::iter {

using ::sus::option::Option;

template <class ItemT, class GenFn>
class Successors;

/// Creates a new iterator where each successive item is computed based on the
/// preceding one.
///
/// The iterator starts with the given `first` item (if any) and calls the
/// given `FnMutBox<Option<Item>>(const Item&)` functor to compute each item's
/// successor.
///
/// # Example
/// ```
/// auto powers_of_10 = sus::iter::successors<u16>(
///     sus::some(1_u16), [](const u16& n) { return n.checked_mul(10_u16); });
/// sus::check(
///     sus::move(powers_of_10).collect<Vec<u16>>() ==
///     sus::Slice<u16>::from({1_u16, 10_u16, 100_u16, 1000_u16, 10000_u16}));
/// ```
template <class Item, ::sus::fn::FnMut<Option<Item>(const Item&)> GenFn>
constexpr inline Successors<Item, GenFn> successors(Option<Item> first,
                                                    GenFn func) noexcept {
  return Successors<Item, GenFn>(::sus::move(first), ::sus::move(func));
}

/// An Iterator that generates each item from a function that takes the previous
/// item.
///
/// This type is created by `sus::iter::successors()`.
template <class ItemT, class GenFn>
class [[nodiscard]] Successors final
    : public IteratorBase<Successors<ItemT, GenFn>, ItemT> {
 public:
  using Item = ItemT;

  // Type is Move and (can be) Clone.
  Successors(Successors&&) = default;
  Successors& operator=(Successors&&) = default;

  // sus::mem::Clone trait.
  constexpr Successors clone() const noexcept
    requires(::sus::mem::Clone<GenFn>)
  {
    return Successors(sus::clone(next_), sus::clone(func_));
  }

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept {
    Option<Item> item = next_.take();
    if (item.is_some()) next_ = ::sus::fn::call_mut(func_, item.as_value());
    return item;
  }

  /// sus::iter::Iterator trait.
  constexpr SizeHint size_hint() const noexcept {
    if (next_.is_some())
      return SizeHint(1u, ::sus::Option<::sus::num::usize>());
    else
      return SizeHint(0u, ::sus::Option<::sus::num::usize>(0u));
  }

 private:
  friend constexpr Successors<Item, GenFn> sus::iter::successors<Item>(
      Option<Item> first, GenFn func) noexcept;

  constexpr Successors(Option<Item> first, GenFn func)
      : next_(::sus::move(first)), func_(::sus::move(func)) {}

  Option<Item> next_;
  GenFn func_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(next_), decltype(func_));
};

}  // namespace sus::iter
