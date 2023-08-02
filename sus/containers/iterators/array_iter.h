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

// IWYU pragma: private, include "sus/containers/array.h"
// IWYU pragma: friend "sus/.*"
#pragma once

#include <stdint.h>

#include <type_traits>

#include "sus/iter/iterator_defn.h"
#include "sus/iter/size_hint.h"
#include "sus/lib/__private/forward_decl.h"
#include "sus/marker/unsafe.h"
#include "sus/mem/clone.h"
#include "sus/mem/move.h"
#include "sus/num/unsigned_integer.h"

namespace sus::containers {

template <::sus::mem::Move ItemT, size_t N>
struct [[nodiscard]] ArrayIntoIter final
    : public ::sus::iter::IteratorBase<ArrayIntoIter<ItemT, N>, ItemT> {
 public:
  using Item = ItemT;

  static constexpr auto with(Array<Item, N>&& array) noexcept {
    return ArrayIntoIter(::sus::move(array));
  }

  // sus::mem::Clone trait.
  constexpr ArrayIntoIter clone() const noexcept
    requires(::sus::mem::Clone<Array<ItemT, N>>)
  {
    return ArrayIntoIter(::sus::clone(array_), front_index_, back_index_);
  }

  /// sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept {
    if constexpr (N == 0) {
      return Option<Item>();
    } else {
      if (front_index_ == back_index_) [[unlikely]]
        return Option<Item>();
      // SAFETY: The front and back indicies are kept within the length of the
      // Array so can not go out of bounds.
      Item& item = array_.get_unchecked_mut(
          ::sus::marker::unsafe_fn,
          ::sus::mem::replace(front_index_, front_index_ + 1_usize));
      return Option<Item>::with(move(item));
    }
  }

  /// sus::iter::Iterator trait.
  constexpr ::sus::iter::SizeHint size_hint() const noexcept {
    ::sus::num::usize remaining = back_index_ - front_index_;
    return ::sus::iter::SizeHint(
        remaining, ::sus::Option<::sus::num::usize>::with(remaining));
  }

  /// sus::iter::DoubleEndedIterator trait.
  constexpr Option<Item> next_back() noexcept {
    if constexpr (N == 0) {
      return Option<Item>();
    } else {
      if (front_index_ == back_index_) [[unlikely]]
        return Option<Item>();
      // SAFETY: The front and back indicies are kept within the length of the
      // Array so can not go out of bounds.
      back_index_ -= 1u;
      Item& item =
          array_.get_unchecked_mut(::sus::marker::unsafe_fn, back_index_);
      return Option<Item>::with(move(item));
    }
  }

  /// sus::iter::ExactSizeIterator trait.
  constexpr ::sus::num::usize exact_size_hint() const noexcept {
    return back_index_ - front_index_;
  }

 private:
  constexpr ArrayIntoIter(Array<Item, N>&& array) noexcept
      : array_(::sus::move(array)) {}

  constexpr ArrayIntoIter(Array<Item, N>&& array, usize front,
                          usize back) noexcept
      : array_(::sus::move(array)), front_index_(front), back_index_(back) {}

  Array<Item, N> array_;
  usize front_index_ = 0_usize;
  usize back_index_ = N;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(front_index_),
                                           decltype(back_index_),
                                           decltype(array_));
};

}  // namespace sus::containers
