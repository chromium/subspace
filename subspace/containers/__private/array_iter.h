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

#include <stdint.h>

#include <type_traits>

#include "subspace/iter/iterator_defn.h"
#include "subspace/marker/unsafe.h"
#include "subspace/mem/move.h"
#include "subspace/mem/mref.h"
#include "subspace/num/unsigned_integer.h"

namespace sus::containers {

template <class T, size_t N>
  requires(N <= size_t{PTRDIFF_MAX})
class Array;

template <::sus::mem::Move ItemT, size_t N>
struct ArrayIntoIter final
    : public ::sus::iter::IteratorImpl<ArrayIntoIter<ItemT, N>, ItemT> {
 public:
  using Item = ItemT;

  static constexpr auto with(Array<Item, N>&& array) noexcept {
    return ArrayIntoIter(::sus::move(array));
  }

  Option<Item> next() noexcept {
    if (front_index_ == back_index_) [[unlikely]]
      return Option<Item>::none();
    // SAFETY: The front and back indicies are kept within the length of the
    // Array so can not go out of bounds.
    Item& item = array_.get_unchecked_mut(
        ::sus::marker::unsafe_fn,
        ::sus::mem::replace(mref(front_index_), front_index_ + 1_usize));
    return Option<Item>::some(move(item));
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept {
    if (front_index_ == back_index_) [[unlikely]]
      return Option<Item>::none();
    // SAFETY: The front and back indicies are kept within the length of the
    // Array so can not go out of bounds.
    back_index_ -= 1u;
    Item& item =
        array_.get_unchecked_mut(::sus::marker::unsafe_fn, back_index_);
    return Option<Item>::some(move(item));
  }

 private:
  ArrayIntoIter(Array<Item, N>&& array) noexcept : array_(::sus::move(array)) {}

  Array<Item, N> array_;
  usize front_index_ = 0_usize;
  usize back_index_ = N;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(front_index_),
                                           decltype(back_index_),
                                           decltype(array_));
};

}  // namespace sus::containers
