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

// IWYU pragma: private, include "sus/option/option.h"
// IWYU pragma: friend "sus/.*"
#pragma once

#include "sus/iter/iterator_defn.h"
#include "sus/iter/size_hint.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"

namespace sus::option {

/// An iterator over the element from an [`Option`]($sus::option::Option).
///
/// This type is returned from [`Option::iter`]($sus::option::Option::iter),
/// [`Option::iter_mut`]($sus::option::Option::iter_mut) and
/// [`Option::into_iter`]($sus::option::Option::into_iter)
template <class ItemT>
class [[nodiscard]] OptionIter final
    : public ::sus::iter::IteratorBase<OptionIter<ItemT>, ItemT> {
 public:
  using Item = ItemT;

  // Type is Move and (can be) Clone.
  OptionIter(OptionIter&&) = default;
  OptionIter& operator=(OptionIter&&) = default;

  // sus::mem::Clone trait.
  constexpr OptionIter clone() const noexcept
    requires(::sus::mem::Clone<Option<Item>>)
  {
    return OptionIter(::sus::clone(item_));
  }

  // [`Iterator`]($sus::iter::Iterator) trait.
  constexpr Option<Item> next() noexcept { return item_.take(); }
  /// [`Iterator`]($sus::iter::Iterator) trait.
  constexpr ::sus::iter::SizeHint size_hint() const noexcept {
    const usize rem = item_.is_some() ? 1u : 0u;
    return ::sus::iter::SizeHint(rem, Option<usize>(rem));
  }
  /// [`DoubleEndedIterator`]($sus::iter::DoubleEndedIterator) trait.
  constexpr Option<Item> next_back() noexcept { return item_.take(); }
  /// [`ExactSizeIterator`]($sus::iter::ExactSizeIterator) trait.
  constexpr usize exact_size_hint() const noexcept {
    return item_.is_some() ? 1u : 0u;
  }
  /// [`TrustedLen`]($sus::iter::TrustedLen) trait.
  /// #[doc.hidden]
  constexpr ::sus::iter::__private::TrustedLenMarker trusted_len()
      const noexcept {
    return {};
  }

 private:
  template <class U>
  friend class Option;

  constexpr OptionIter(Option<Item> item) : item_(::sus::move(item)) {}

  Option<Item> item_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(item_));
};

}  // namespace sus::option
