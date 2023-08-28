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

#include "sus/iter/iterator_defn.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"

namespace sus::iter {

using ::sus::mem::relocate_by_memcpy;

/// An iterator that skips over `n` elements of another iterator.
///
/// This type is returned from `Iterator::skip()`.
template <class InnerSizedIter>
class [[nodiscard]] StepBy final
    : public IteratorBase<StepBy<InnerSizedIter>,
                          typename InnerSizedIter::Item> {
 public:
  using Item = InnerSizedIter::Item;

  // The type is Move and (can be) Clone.
  StepBy(StepBy&&) = default;
  StepBy& operator=(StepBy&&) = default;

  // sus::mem::Clone trait.
  constexpr StepBy clone() const noexcept
    requires(::sus::mem::Clone<InnerSizedIter>)
  {
    return StepBy(CLONE, step_, ::sus::clone(next_iter_), first_take_);
  }

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept {
    Option<Item> out = next_iter_.next();
    if (first_take_) {
      first_take_ = false;
    } else {
      // TODO: If SizedIterator satisifies Iterator we can use nth(n).
      for (usize i; i < step_; i += 1u) {
        if (out.is_none()) return out;
        out = next_iter_.next();
      }
    }
    return out;
  }
  /// sus::iter::Iterator trait.
  constexpr SizeHint size_hint() const noexcept {
    auto first_size = [this](usize n) {
      if (n == 0u) {
        return 0_usize;
      } else {
        return 1u + (n - 1u) / (step_ + 1u);
      }
    };
    auto other_size = [this](usize n) { return n / (step_ + 1u); };

    auto [lower, upper] = next_iter_.size_hint();

    if (first_take_) {
      return {first_size(lower), upper.map(first_size)};
    } else {
      return {other_size(lower), upper.map(other_size)};
    }
  }

  // sus::iter::DoubleEndedIterator trait.
  constexpr Option<Item> next_back() noexcept
    requires(DoubleEndedIterator<InnerSizedIter, Item> &&  //
             ExactSizeIterator<InnerSizedIter, Item>)
  {
    // TODO: If SizedIterator satisifies Iterator we can use
    // nth_back(n).
    const usize n = next_back_index();
    Option<Item> out = next_iter_.next_back();
    for (usize i; i < n; i += 1u) {
      if (out.is_none()) return out;
      out = next_iter_.next_back();
    }
    return out;
  }

  // sus::iter::ExactSizeIterator trait.
  constexpr usize exact_size_hint() const noexcept
    requires(ExactSizeIterator<InnerSizedIter, Item>)
  {
    return size_hint().lower;
  }

  /// sus::iter::TrustedLen trait.
  constexpr ::sus::iter::__private::TrustedLenMarker trusted_len()
      const noexcept
    requires(TrustedLen<InnerSizedIter>)
  {
    return {};
  }

  // private:
  template <class U, class V>
  friend class IteratorBase;

  // The zero-based index starting from the end of the iterator of the
  // last element. Used in the `DoubleEndedIterator` implementation.
  constexpr usize next_back_index() const
    requires(ExactSizeIterator<InnerSizedIter, Item>)
  {
    auto rem = next_iter_.exact_size_hint() % (step_ + 1u);
    if (first_take_)
      return rem == 0u ? step_ : rem - 1u;
    else
      return rem;
  }

  // Regular ctor.
  explicit constexpr StepBy(usize step, InnerSizedIter&& next_iter) noexcept
      :  // We subtract one from `step`, as stepping 1 means we skip 0 elements
         // between each.
        step_(step - 1u),
        next_iter_(::sus::move(next_iter)),
        first_take_(true) {}
  // Clone ctor.
  enum Clone { CLONE };
  explicit constexpr StepBy(Clone, usize step, InnerSizedIter&& next_iter,
                            bool first_take) noexcept
      : step_(step),
        next_iter_(::sus::move(next_iter)),
        first_take_(first_take) {}

  usize step_;
  InnerSizedIter next_iter_;
  bool first_take_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(step_),
                                           decltype(next_iter_),
                                           decltype(first_take_));
};

}  // namespace sus::iter
