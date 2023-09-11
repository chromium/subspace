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

// IWYU pragma: private, include "sus/iter/iterator.h"
// IWYU pragma: friend "sus/.*"
#pragma once

#include "sus/fn/fn_concepts.h"
#include "sus/iter/iterator_defn.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"

namespace sus::iter {

/// An iterator that calls a function with a reference to each element before
/// yielding it.
///
/// This type is returned from `Iterator::inspect()`.
template <class InnerSizedIter, class InspectFn>
class [[nodiscard]] Inspect final
    : public IteratorBase<Inspect<InnerSizedIter, InspectFn>,
                          typename InnerSizedIter::Item> {
 public:
  using Item = typename InnerSizedIter::Item;

  static_assert(
      ::sus::fn::FnMut<InspectFn, void(const std::remove_reference_t<Item>&)>);

  // Type is Move and (can be) Clone.
  Inspect(Inspect&&) = default;
  Inspect& operator=(Inspect&&) = default;

  // sus::mem::Clone trait.
  constexpr Inspect clone() noexcept
    requires(::sus::mem::Clone<InspectFn> &&  //
             ::sus::mem::Clone<InnerSizedIter>)
  {
    return Inspect(::sus::clone(inspect_), ::sus::clone(next_iter_));
  }

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept {
    Option<Item> item = next_iter_.next();
    if (item.is_some()) ::sus::fn::call_mut(inspect_, item.as_value());
    return item;
  }

  /// sus::iter::Iterator trait.
  constexpr SizeHint size_hint() const noexcept {
    return next_iter_.size_hint();
  }

  // sus::iter::DoubleEndedIterator trait.
  constexpr Option<Item> next_back() noexcept
    requires(DoubleEndedIterator<InnerSizedIter, Item>)
  {
    Option<Item> item = next_iter_.next_back();
    if (item.is_some()) ::sus::fn::call_mut(inspect_, item.as_value());
    return item;
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

  explicit constexpr Inspect(InspectFn&& fn, InnerSizedIter&& next_iter)
      : inspect_(::sus::move(fn)), next_iter_(::sus::move(next_iter)) {}

  InspectFn inspect_;
  InnerSizedIter next_iter_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(inspect_),
                                           decltype(next_iter_));
};

}  // namespace sus::iter
