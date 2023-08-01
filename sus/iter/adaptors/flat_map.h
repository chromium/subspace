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

#include "sus/fn/fn_box_defn.h"
#include "sus/iter/iterator_defn.h"
#include "sus/iter/sized_iterator.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"

namespace sus::iter {

/// An iterator that maps an iterator of types into an iterator of iterable
/// types through a user-defined function, and then flattens them into an
/// interator of those those iterable types' items.
///
/// In other words, this type maps `Iterator[X]` into
/// `Iterator[IntoIterable[T]]` into `Iterator[T]`.
///
/// This type is returned from `Iterator::flat_map()`.
template <class IntoIterable, class InnerSizedIter>
class [[nodiscard]] FlatMap final
    : public IteratorBase<FlatMap<IntoIterable, InnerSizedIter>,
                          typename IntoIteratorOutputType<IntoIterable>::Item> {
  using EachIter = IntoIteratorOutputType<IntoIterable>;
  using MapFn =
      ::sus::fn::FnMutBox<IntoIterable(typename InnerSizedIter::Item&&)>;

 public:
  using Item = typename EachIter::Item;

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    Option<Item> out;
    while (true) {
      // Take an item off front_iter_ if possible.
      if (front_iter_.is_some()) {
        out = front_iter_.as_value_mut().next();
        if (out.is_some()) return out;
        front_iter_ = Option<EachIter>();
      }
      // Otherwise grab the next iterator into front_iter_.
      front_iter_ = iters_.next().map([this](auto&& i) {
        return ::sus::fn::call_mut(map_fn_, ::sus::move(i)).into_iter();
      });
      if (front_iter_.is_none()) break;
    }
    // There's no more iterator to place in front_iter_. Take an item off
    // back_iter_ if possible.
    if (back_iter_.is_some()) {
      out = back_iter_.as_value_mut().next();
      if (out.is_some()) return out;
      back_iter_ = Option<EachIter>();
    }
    // There's nothing left.
    return out;
  }

  /// sus::iter::Iterator trait.
  ::sus::iter::SizeHint size_hint() const noexcept {
    auto [flo, fhi] = front_iter_.as_ref().map_or(
        SizeHint(0u, ::sus::some(0u)),
        [](const EachIter& i) { return i.size_hint(); });
    auto [blo, bhi] = back_iter_.as_ref().map_or(
        SizeHint(0u, ::sus::some(0u)),
        [](const EachIter& i) { return i.size_hint(); });
    // Lower bound is the items in the iterators we can see.
    usize lo = flo.saturating_add(blo);
    // We have no upper bound if there's any other iterators left, and we can't
    // tell if there's any iterators left without additional tracking state.
    return SizeHint(lo, ::sus::Option<usize>());
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept
    requires(InnerSizedIter::DoubleEnded &&  //
             DoubleEndedIterator<EachIter, typename EachIter::Item>)
  {
    Option<Item> out;
    while (true) {
      // Take an item off back_iter_ if possible.
      if (back_iter_.is_some()) {
        out = back_iter_.as_value_mut().next_back();
        if (out.is_some()) return out;
        back_iter_ = Option<EachIter>();
      }
      // Otherwise grab the next iterator into back_iter_.
      back_iter_ = iters_.next_back().map([this](auto&& i) {
        return ::sus::fn::call_mut(map_fn_, ::sus::move(i)).into_iter();
      });
      if (back_iter_.is_none()) break;
    }
    // There's no more iterator to place in back_iter_. Take an item off
    // front_iter_ if possible.
    if (front_iter_.is_some()) {
      out = front_iter_.as_value_mut().next();
      if (out.is_some()) return out;
      front_iter_ = Option<EachIter>();
    }
    // There's nothing left.
    return out;
  }

 private:
  template <class U, class V>
  friend class IteratorBase;

  static FlatMap with(MapFn&& fn, InnerSizedIter&& iters) noexcept {
    return FlatMap(::sus::move(fn), ::sus::move(iters));
  }

  FlatMap(MapFn&& fn, InnerSizedIter&& iters)
      : map_fn_(::sus::move(fn)), iters_(::sus::move(iters)) {}

  MapFn map_fn_;
  InnerSizedIter iters_;
  ::sus::Option<EachIter> front_iter_;
  ::sus::Option<EachIter> back_iter_;

  // The MapFn and InnerSizedIter are trivially relocatable but the EachIter may
  // not be.
  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(map_fn_), decltype(iters_),
                                           decltype(front_iter_),
                                           decltype(front_iter_));
};

}  // namespace sus::iter
