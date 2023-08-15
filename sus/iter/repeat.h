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
#include "sus/mem/clone.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"

namespace sus::iter {

using ::sus::option::Option;

template <class ItemT>
class Repeat;

/// Creates a new iterator that endlessly repeats a single element.
///
/// The `repeat()` function repeats a single value over and over again.
///
/// Infinite iterators like `repeat()` are often used with adapters like
/// [`Iterator::take()`](sus::iter::IteratorBase::take), in order to make them
/// finite.
///
/// If the element type of the iterator you need does not implement `Clone`, or
/// if you do not want to keep the repeated element in memory, you can instead
/// use the `repeat_with()` function.
///
/// # Example
/// ```
/// auto r = sus::iter::repeat<u16>(3u);
/// sus::check(r.next().unwrap() == 3_u16);
/// sus::check(r.next().unwrap() == 3_u16);
/// sus::check(r.next().unwrap() == 3_u16);
/// ```
template <class Item>
  requires ::sus::mem::Clone<Item>
constexpr inline Repeat<Item> repeat(Item item) noexcept {
  return Repeat<Item>(::sus::move(item));
}

/// An Iterator that walks over at most a single Item.
template <class ItemT>
class [[nodiscard]] Repeat final : public IteratorBase<Repeat<ItemT>, ItemT> {
 public:
  using Item = ItemT;

  // sus::mem::Clone trait.
  constexpr Repeat clone() const noexcept
    requires(::sus::mem::Clone<Item>)
  {
    return Repeat(::sus::clone(item_));
  }
  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept {
    return ::sus::Option<Item>(sus::clone(item_));
  }
  /// sus::iter::Iterator trait.
  constexpr SizeHint size_hint() const noexcept {
    return SizeHint(usize::MAX, ::sus::Option<::sus::num::usize>());
  }
  // sus::iter::DoubleEndedIterator trait.
  constexpr Option<Item> next_back() noexcept {
    return ::sus::Option<Item>(sus::clone(item_));
  }

 private:
  friend Repeat<Item> sus::iter::repeat<Item>(Item item) noexcept;

  constexpr Repeat(Item item) : item_(::sus::move(item)) {}

  Item item_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(item_));
};

}  // namespace sus::iter
