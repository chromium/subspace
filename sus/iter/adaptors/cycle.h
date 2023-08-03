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
#include "sus/iter/sized_iterator.h"
#include "sus/mem/clone.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"

namespace sus::iter {

/// An iterator that repeats endlessly.
///
/// This type is returned from `Iterator::cycle()`.
template <class InnerSizedIter>
class [[nodiscard]] [[sus_trivial_abi]] Cycle final
    : public IteratorBase<Cycle<InnerSizedIter>,
                          typename InnerSizedIter::Item> {
  static_assert(::sus::mem::Clone<InnerSizedIter>);

 public:
  using Item = typename InnerSizedIter::Item;

  // sus::mem::Clone trait.
  Cycle clone() const noexcept {
    return Cycle(::sus::clone(original_), ::sus::clone(active_));
  }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    auto o = active_.next();
    if (o.is_none()) {
      active_ = ::sus::clone(original_);
      o = active_.next();
    }
    return o;
  }

  /// sus::iter::Iterator trait.
  SizeHint size_hint() const noexcept {
    SizeHint sz = original_.size_hint();
    if (sz.lower != 0u) {
      // More than 0 items, so it's infinite.
      return SizeHint(usize::MAX, ::sus::Option<usize>());
    }
    // Posssibly 0 items, lower limit is 0.
    return SizeHint(0u, ::sus::move(sz).upper.map_or_else(
        // No upper limit, so we have no limit either.
        [] { return ::sus::Option<usize>(); },
        [](usize u) {
          if (u == 0u) {
            // Upper limit is 0, empty iterator.
            return ::sus::Option<usize>::with(0u);
          } else
            // Non-zero upper limit, will cycle forever.
            return ::sus::Option<usize>();
        }));
  }

 private:
  template <class U, class V>
  friend class IteratorBase;

  static Cycle with(InnerSizedIter && iter) noexcept {
    return Cycle(::sus::clone(iter), ::sus::move(iter));
  }

  Cycle(InnerSizedIter&& __restrict original,
        InnerSizedIter&& __restrict active)
      : original_(::sus::move(original)), active_(::sus::move(active)) {}

  InnerSizedIter original_;
  InnerSizedIter active_;

  // The InnerSizedIter is trivially relocatable.
  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(original_),
                                  decltype(active_));
};

}  // namespace sus::iter
