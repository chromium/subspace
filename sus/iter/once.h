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
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"

namespace sus::iter {

using ::sus::option::Option;

template <class ItemT>
class Once;

/// Constructs a `Once` iterator that will return `o` and then None. If `o` is
/// `None`, then the iterator will be empty on construction.
///
/// # Example
/// ```
/// auto o = sus::iter::once<u16>(sus::some(3_u16));
/// sus::check(o.next().unwrap() == 3_u16);
/// ```
template <class Item>
inline Once<Item> once(Option<Item> o) noexcept {
  return Once<Item>(::sus::move(o));
}

/// An Iterator that walks over at most a single Item.
template <class ItemT>
class [[nodiscard]] Once final : public IteratorBase<Once<ItemT>, ItemT> {
 public:
  using Item = ItemT;

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept { return single_.take(); }
  /// sus::iter::Iterator trait.
  SizeHint size_hint() const noexcept {
    ::sus::num::usize rem = single_.is_some() ? 1u : 0u;
    return SizeHint(rem, ::sus::Option<::sus::num::usize>::with(rem));
  }
  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept { return single_.take(); }
  // sus::iter::ExactSizeIterator trait.
  usize exact_size_hint() const noexcept { return single_.is_some() ? 1u : 0u; }

 private:
  friend Once<Item> sus::iter::once<Item>(Option<Item> o) noexcept;

  Once(Option<Item> single) : single_(::sus::move(single)) {}

  Option<Item> single_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(single_));
};

}  // namespace sus::iter
