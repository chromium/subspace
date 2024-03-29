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
#include "sus/mem/clone.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"

namespace sus::iter {

/// An iterator that repeats endlessly.
///
/// This type is returned from `Iterator::cycle()`.
template <class InnerSizedIter>
class [[nodiscard]] Cycle final
    : public IteratorBase<Cycle<InnerSizedIter>,
                          typename InnerSizedIter::Item> {
  static_assert(::sus::mem::Clone<InnerSizedIter>);

 public:
  using Item = typename InnerSizedIter::Item;

  // The type is Move and CLone.
  Cycle(Cycle&&) = default;
  Cycle& operator=(Cycle&&) = default;

  // sus::mem::Clone trait.
  constexpr Cycle clone() const noexcept {
    return Cycle(::sus::clone(original_), ::sus::clone(active_));
  }

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept {
    auto o = active_.next();
    if (o.is_none()) {
      active_ = ::sus::clone(original_);
      o = active_.next();
    }
    return o;
  }

  /// sus::iter::Iterator trait.
  constexpr SizeHint size_hint() const noexcept {
    auto [lower, upper] = original_.size_hint();
    if (lower != 0u) {
      // More than 0 items, so it's infinite.
      return SizeHint(usize::MAX, ::sus::Option<usize>());
    }
    // Posssibly 0 items, lower limit is 0.
    return SizeHint(0u, ::sus::move(upper).map_or_else(
                            // No upper limit, so we have no limit either.
                            [] { return ::sus::Option<usize>(); },
                            [](usize u) {
                              if (u == 0u) {
                                // Upper limit is 0, empty iterator.
                                return ::sus::Option<usize>(0u);
                              } else
                                // Non-zero upper limit, will cycle forever.
                                return ::sus::Option<usize>();
                            }));
  }

  /// sus::iter::TrustedLen trait.
  constexpr ::sus::iter::__private::TrustedLenMarker trusted_len()
      const noexcept
    requires(TrustedLen<InnerSizedIter>)
  {
    return {};
  }

 private:
  template <class U, class V>
  friend class IteratorBase;

  explicit constexpr Cycle(InnerSizedIter&& iter)
      : original_(::sus::clone(iter)), active_(::sus::move(iter)) {}

  InnerSizedIter original_;
  InnerSizedIter active_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(original_),
                                           decltype(active_));
};

}  // namespace sus::iter
