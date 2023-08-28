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

/// An iterator that copied the elements of an underlying iterator.
///
/// This type is returned from `Iterator::copied()`.
template <class InnerSizedIter>
class [[nodiscard]] Copied final
    : public IteratorBase<Copied<InnerSizedIter>,
                          std::remove_cvref_t<typename InnerSizedIter::Item>> {
 public:
  using Item = std::remove_cvref_t<typename InnerSizedIter::Item>;

  // Type is Move and (can be) Clone.
  Copied(Copied&&) = default;
  Copied& operator=(Copied&&) = default;

  // sus::mem::Clone trait.
  constexpr Copied clone() const noexcept
    requires(::sus::mem::Clone<InnerSizedIter>)
  {
    return Copied(::sus::clone(next_iter_));
  }

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept {
    return next_iter_.next().map([](const Item& item) -> Item { return item; });
  }

  /// sus::iter::Iterator trait.
  constexpr SizeHint size_hint() const noexcept {
    return next_iter_.size_hint();
  }

  // sus::iter::DoubleEndedIterator trait.
  constexpr Option<Item> next_back() noexcept
    requires(DoubleEndedIterator<InnerSizedIter, Item>)
  {
    return next_iter_.next_back().map(
        [](const Item& item) -> Item { return item; });
  }

  // sus::iter::ExactSizeIterator trait.
  constexpr usize exact_size_hint() const noexcept
    requires(ExactSizeIterator<InnerSizedIter, Item>)
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
  template <class U, class V>
  friend class IteratorBase;

  explicit constexpr Copied(InnerSizedIter&& next_iter)
      : next_iter_(::sus::move(next_iter)) {}

  InnerSizedIter next_iter_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(next_iter_));
};

}  // namespace sus::iter
