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

/// An iterator that moves from the elements of an underlying iterator.
///
/// This type is returned from [`IteratorOverRange::moved()`](
/// $sus::iter::IteratorOverRange::moved).
template <class InnerSizedIter>
class [[nodiscard]] Moved final
    : public IteratorBase<Moved<InnerSizedIter>,
                          std::remove_cvref_t<typename InnerSizedIter::Item>> {
  static_assert(
      !std::is_const_v<std::remove_reference_t<typename InnerSizedIter::Item>>,
      "Moved requires a mutable reference to move from");

 public:
  using Item = std::remove_cvref_t<typename InnerSizedIter::Item>;

  // Type is Move and (can be) Clone.
  Moved(Moved&&) = default;
  Moved& operator=(Moved&&) = default;

  // sus::mem::Clone trait.
  constexpr Moved clone() const noexcept
    requires(::sus::mem::Clone<InnerSizedIter>)
  {
    return Moved(::sus::clone(next_iter_));
  }

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept {
    return next_iter_.next().map(
        // The `next_iter_` item may be a value or a reference, but we move from
        // it unconditionally.
        [](auto&& item) -> std::remove_reference_t<Item> {
          return ::sus::move(item);
        });
  }

  /// sus::iter::Iterator trait.
  constexpr SizeHint size_hint() const noexcept {
    return next_iter_.size_hint();
  }

  // sus::iter::DoubleEndedIterator trait.
  constexpr Option<Item> next_back() noexcept
    requires(DoubleEndedIterator<InnerSizedIter, typename InnerSizedIter::Item>)
  {
    return next_iter_.next_back().map(
        [](const Item& item) -> Item { return item; });
  }

  // sus::iter::ExactSizeIterator trait.
  constexpr usize exact_size_hint() const noexcept
    requires(ExactSizeIterator<InnerSizedIter, typename InnerSizedIter::Item>)
  {
    return next_iter_.exact_size_hint();
  }

  /// sus::iter::TrustedLen trait.
  constexpr ::sus::iter::__private::TrustedLenMarker trusted_len()
      const noexcept
    requires(TrustedLen<InnerSizedIter>)
  {
    return {};
  }

 private:
  template <class R, class B, class E, class I>
  friend class IteratorOverRange;

  explicit constexpr Moved(InnerSizedIter&& next_iter)
      : next_iter_(::sus::move(next_iter)) {}

  InnerSizedIter next_iter_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(next_iter_));
};

}  // namespace sus::iter
