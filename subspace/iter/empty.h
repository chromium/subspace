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

#pragma once

#include "subspace/iter/iterator_defn.h"
#include "subspace/mem/relocate.h"
#include "subspace/option/option.h"

namespace sus::iter {

template <class ItemT>
class Empty;

/// Constructs an `Empty` iterator, which is an empty iterator that returns
/// nothing.
///
/// # Example
/// ```
/// auto empty = sus::iter::empty<u16>();
/// sus::check(empty.next().is_none());
/// ```
template <class Item>
constexpr inline Empty<Item> empty() noexcept {
  return Empty<Item>();
}

/// An Iterator that never returns an `Item`.
///
/// This type is created by `sus::iter::empty()`.
template <class ItemT>
class [[nodiscard]] [[sus_trivial_abi]] Empty final
    : public IteratorBase<Empty<ItemT>, ItemT> {
 public:
  using Item = ItemT;

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept { return sus::Option<Item>(); }
  /// sus::iter::Iterator trait.
  constexpr ::sus::iter::SizeHint size_hint() const noexcept {
    return SizeHint(0u, ::sus::Option<::sus::num::usize>::with(0u));
  }
  // sus::iter::DoubleEndedIterator trait.
  constexpr Option<Item> next_back() noexcept { return sus::Option<Item>(); }
  // sus::iter::ExactSizeIterator trait.
  constexpr usize exact_size_hint() const noexcept { return 0u; }

 private:
  friend Empty<Item> sus::iter::empty<Item>() noexcept;

  constexpr Empty() = default;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn);
};

}  // namespace sus::iter
