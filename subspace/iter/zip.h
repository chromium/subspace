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

#include "subspace/fn/fn_concepts.h"
#include "subspace/iter/iterator_defn.h"
#include "subspace/iter/sized_iterator.h"
#include "subspace/mem/move.h"
#include "subspace/mem/relocate.h"

namespace sus::iter {

using ::sus::mem::relocate_by_memcpy;

namespace __private {
template <class T>
using GetItem = typename T::Item;

template <class Item, size_t N, class... T>
auto nexts(Option<Item>& out, auto& iters, T&&... args) -> void {
  constexpr size_t I = sizeof...(T);
  if constexpr (I == N) {
    if ((... && args.is_some()))
      out.insert(Item::with(::sus::move(args).unwrap()...));
  } else {
    return nexts<Item, N>(out, iters, ::sus::move(args)...,
                          iters.template at_mut<I>().next());
  }
}

template <size_t I, size_t N>
auto size_hints(auto& iters) noexcept -> SizeHint {
  if constexpr (I == N - 1) {
    return iters.template at<I>().size_hint();
  } else {
    SizeHint left = iters.template at<I>().size_hint();
    SizeHint right = size_hints<I + 1, N>(iters);
    usize lower = ::sus::ops::min(left.lower, right.lower);
    if (left.upper.is_some() && right.upper.is_some()) {
      usize lu = left.upper.as_value();
      usize ru = right.upper.as_value();
      return SizeHint(lower, ::sus::some(::sus::ops::min(lu, ru)));
    } else if (left.upper.is_some()) {
      return SizeHint(lower, left.upper);
    } else if (right.upper.is_some()) {
      return SizeHint(lower, right.upper);
    } else {
      return SizeHint(lower, sus::none());
    }
  }
}

template <size_t I, size_t N>
auto exact_size_hints(auto& iters) noexcept -> usize {
  if constexpr (I == N - 1) {
    return iters.template at<I>().exact_size_hint();
  } else {
    usize left = iters.template at<I>().exact_size_hint();
    usize right = exact_size_hints<I + 1, N>(iters);
    return ::sus::ops::min(left, right);
  }
}

}  // namespace __private

/// An iterator that iterates a group of other iterators simultaneously.
///
/// This type is returned from `Iterator::zip()`.
template <class... InnerSizedIters>
class [[nodiscard]] [[sus_trivial_abi]] Zip final
    : public IteratorBase<Zip<InnerSizedIters...>,
                          sus::Tuple<__private::GetItem<InnerSizedIters>...>> {
 public:
  using Item = sus::Tuple<__private::GetItem<InnerSizedIters>...>;

  // sus::mem::Clone trait.
  Zip clone() const noexcept
    requires((... && InnerSizedIters::Clone))
  {
    return Zip(::sus::clone(iters_));
  }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    Option<Item> out;
    __private::nexts<Item, sizeof...(InnerSizedIters)>(out, iters_);
    return out;
  }
  /// sus::iter::Iterator trait.
  SizeHint size_hint() const noexcept {
    return __private::size_hints<0, sizeof...(InnerSizedIters)>(iters_);
  }

  // sus::iter::ExactSizeIterator trait.
  usize exact_size_hint() const noexcept
    requires((... && InnerSizedIters::ExactSize))
  {
    return __private::exact_size_hints<0, sizeof...(InnerSizedIters)>(iters_);
  }

 private:
  template <class U, class V>
  friend class IteratorBase;

  static Zip with(sus::Tuple<InnerSizedIters...> && iters) noexcept {
    return Zip(::sus::move(iters));
  }

  Zip(::sus::Tuple<InnerSizedIters...> && iters) noexcept
      : iters_(::sus::move(iters)) {}

  ::sus::Tuple<InnerSizedIters...> iters_;

  // The InnerSizedIter and usize are trivially relocatable.
  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(iters_));
};

}  // namespace sus::iter
