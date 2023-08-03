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

namespace __private {
template <class T, class U>
constexpr inline Option<U> and_then_or_clear(
    Option<T>& opt, ::sus::fn::FnOnce<Option<U>(T&)> auto&& f) {
  if (opt.is_none()) {
    return Option<U>();
  } else {
    return ::sus::fn::call_once(::sus::move(f), *opt).or_else([&opt]() {
      opt = Option<T>();
      return Option<U>();
    });
  }
}

}  // namespace __private

/// An iterator that yields the current count and the element during iteration.
///
/// This type is returned from `Iterator::enumerate()`.
template <class InnerSizedIter, class OtherSizedIter>
class [[nodiscard]] Chain final
    : public IteratorBase<Chain<InnerSizedIter, OtherSizedIter>,
                          typename InnerSizedIter::Item> {
 public:
  using Item = typename InnerSizedIter::Item;

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept {
    return __private::and_then_or_clear<InnerSizedIter, Item>(
               first_iter_, [](InnerSizedIter& iter) { return iter.next(); })
        .or_else([this] {
          if (second_iter_.is_some())
            return second_iter_->next();
          else
            return Option<Item>();
        });
  }

  // sus::iter::DoubleEndedIterator trait.
  constexpr Option<Item> next_back() noexcept
    requires(DoubleEndedIterator<InnerSizedIter, Item> &&
             DoubleEndedIterator<OtherSizedIter, Item>)
  {
    return __private::and_then_or_clear<OtherSizedIter, Item>(
               second_iter_,
               [](OtherSizedIter& iter) { return iter.next_back(); })
        .or_else([this] {
          if (first_iter_.is_some())
            return first_iter_->next();
          else
            return Option<Item>();
        });
  }

  constexpr SizeHint size_hint() const noexcept {
    if (first_iter_.is_none()) {
      if (second_iter_.is_none()) {
        return SizeHint(0u, sus::Option<usize>::with(0u));
      } else {
        return second_iter_->size_hint();
      }
    } else {
      if (second_iter_.is_none()) {
        return first_iter_->size_hint();
      } else {
        auto [fst_lower, fst_upper] = first_iter_->size_hint();
        auto [snd_lower, snd_upper] = second_iter_->size_hint();
        auto lower = fst_lower.saturating_add(snd_lower);
        if (fst_upper.is_some() && snd_upper.is_some())
          return SizeHint(lower, (*fst_upper).checked_add(*snd_upper));
        else
          return SizeHint(lower, ::sus::Option<::sus::num::usize>());
      }
    }
  }

  // No exact_size_hint() as the size of two iterators may overflow.

  // TODO: Implement nth(), nth_back(), etc...

 private:
  template <class U, class V>
  friend class IteratorBase;

  static constexpr Chain with(InnerSizedIter&& first_iter,
                    OtherSizedIter&& second_iter) noexcept {
    return Chain(::sus::move(first_iter), ::sus::move(second_iter));
  }

  constexpr Chain(InnerSizedIter&& first_iter, OtherSizedIter&& second_iter)
      : first_iter_(
            ::sus::Option<InnerSizedIter>::with(::sus::move(first_iter))),
        second_iter_(
            ::sus::Option<OtherSizedIter>::with(::sus::move(second_iter))) {}

  // These are "fused" with `Option` so we don't need separate state to track
  // which part is already exhausted, and we get niche layout for `None` with
  // the `SizedIterator` type.
  //
  // Only the "first" iterator is actually set `None` when exhausted,
  // depending on whether you iterate forward or backward. If you mix
  // directions, then both sides may be `None`.
  ::sus::Option<InnerSizedIter> first_iter_;
  ::sus::Option<OtherSizedIter> second_iter_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(first_iter_),
                                           decltype(second_iter_));
};

}  // namespace sus::iter
