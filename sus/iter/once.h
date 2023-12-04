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

#pragma once

#include "sus/iter/iterator_defn.h"
#include "sus/iter/size_hint.h"
#include "sus/macros/lifetimebound.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"

namespace sus::iter {

using ::sus::option::Option;

template <class ItemT>
class Once;

/// Constructs a `Once` iterator that will return `o` and then None.
///
/// # Examples
/// An iterator that returns a number once:
/// ```
/// auto o = sus::iter::once<u16>(3_u16);
/// sus_check(o.next().unwrap() == 3_u16);
/// sus_check(o.next().is_none());
/// ```
///
/// An iterator that returns a reference once:
/// ```
/// auto u = 3_u16;
/// auto o = sus::iter::once<u16&>(u);
/// u16& r = o.next().unwrap();
/// sus_check(r == 3u);
/// sus_check(&r == &u);
/// sus_check(o.next().is_none());
/// ```
template <class Item>
constexpr inline Once<Item> once(Item o sus_lifetimebound) noexcept {
  return Once<Item>(::sus::forward<Item>(o));
}

/// An Iterator that walks over at most a single Item.
template <class ItemT>
class [[nodiscard]] Once final : public IteratorBase<Once<ItemT>, ItemT> {
 public:
  using Item = ItemT;

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept { return single_.take(); }
  /// sus::iter::Iterator trait.
  constexpr SizeHint size_hint() const noexcept {
    ::sus::num::usize rem = single_.is_some() ? 1u : 0u;
    return SizeHint(rem, ::sus::Option<::sus::num::usize>(rem));
  }
  // sus::iter::DoubleEndedIterator trait.
  constexpr Option<Item> next_back() noexcept { return single_.take(); }
  // sus::iter::ExactSizeIterator trait.
  constexpr usize exact_size_hint() const noexcept {
    return single_.is_some() ? 1u : 0u;
  }
  constexpr ::sus::iter::__private::TrustedLenMarker trusted_len()
      const noexcept {
    return {};
  }

 private:
  friend constexpr Once<Item> sus::iter::once<Item>(Item o) noexcept;

  constexpr Once(Item single sus_lifetimebound)
      : single_(::sus::forward<Item>(single)) {}

  Option<Item> single_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(single_));
};

}  // namespace sus::iter
