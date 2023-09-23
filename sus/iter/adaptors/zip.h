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

#include "sus/fn/fn_concepts.h"
#include "sus/iter/iterator_defn.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"

namespace sus::iter {

using ::sus::mem::relocate_by_memcpy;

namespace __private {
template <class T>
using GetItem = typename T::Item;

template <class TupleItem, size_t N, class... T>
inline constexpr Option<TupleItem> nexts(auto& iters, T&&... args) {
  constexpr size_t I = sizeof...(T);
  if constexpr (I == N) {
    if ((... && args.is_some())) {
      // SAFETY: args.is_some() is checked above, so unwrap has a value.
      return Option<TupleItem>(TupleItem(
          ::sus::move(args).unwrap_unchecked(::sus::marker::unsafe_fn)...));
    } else {
      return Option<TupleItem>();
    }
  } else {
    return nexts<TupleItem, N>(iters, ::sus::move(args)...,
                               iters.template at_mut<I>().next());
  }
}

template <size_t I, size_t N>
inline constexpr SizeHint size_hints(auto& iters) noexcept {
  if constexpr (I == N - 1) {
    return iters.template at<I>().size_hint();
  } else {
    SizeHint left = iters.template at<I>().size_hint();
    SizeHint right = size_hints<I + 1, N>(iters);
    usize lower = ::sus::cmp::min(left.lower, right.lower);
    if (left.upper.is_some() && right.upper.is_some()) {
      usize lu = left.upper.as_value();
      usize ru = right.upper.as_value();
      return SizeHint(lower, ::sus::some(::sus::cmp::min(lu, ru)));
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
inline constexpr usize exact_size_hints(auto& iters) noexcept {
  if constexpr (I == N - 1) {
    return iters.template at<I>().exact_size_hint();
  } else {
    usize left = iters.template at<I>().exact_size_hint();
    usize right = exact_size_hints<I + 1, N>(iters);
    return ::sus::cmp::min(left, right);
  }
}

}  // namespace __private

/// An iterator that iterates a group of other iterators simultaneously.
///
/// This type is returned from `Iterator::zip()`.
template <class... InnerSizedIters>
class [[nodiscard]] Zip final
    : public IteratorBase<Zip<InnerSizedIters...>,
                          sus::Tuple<__private::GetItem<InnerSizedIters>...>> {
 public:
  using Item = sus::Tuple<__private::GetItem<InnerSizedIters>...>;

  // Type is Move and (can be) Clone.
  constexpr Zip(Zip&&) = default;
  Zip& operator=(Zip&&) = default;

  // sus::mem::Clone trait.
  constexpr Zip clone() const noexcept
    requires((... && ::sus::mem::Clone<InnerSizedIters>))
  {
    return Zip(::sus::clone(iters_));
  }

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept {
    return __private::nexts<Item, sizeof...(InnerSizedIters)>(iters_);
  }
  /// sus::iter::Iterator trait.
  constexpr SizeHint size_hint() const noexcept {
    return __private::size_hints<0, sizeof...(InnerSizedIters)>(iters_);
  }

  // sus::iter::ExactSizeIterator trait.
  constexpr usize exact_size_hint() const noexcept
    requires(
        (... &&
         ExactSizeIterator<InnerSizedIters, typename InnerSizedIters::Item>))
  {
    return __private::exact_size_hints<0, sizeof...(InnerSizedIters)>(iters_);
  }

  /// sus::iter::TrustedLen trait.
  constexpr ::sus::iter::__private::TrustedLenMarker trusted_len()
      const noexcept
    requires((... && TrustedLen<InnerSizedIters>))
  {
    return {};
  }

 private:
  template <class U, class V>
  friend class IteratorBase;

  explicit constexpr Zip(::sus::Tuple<InnerSizedIters...>&& iters) noexcept
      : iters_(::sus::move(iters)) {}

  ::sus::Tuple<InnerSizedIters...> iters_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(iters_));
};

}  // namespace sus::iter
