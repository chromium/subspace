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

template <class T>
class Vec;

template <class ItemT>
struct [[nodiscard]] VecIntoIter final
    : public ::sus::iter::IteratorBase<VecIntoIter<ItemT>, ItemT> {
 public:
  using Item = ItemT;

  /// Constructs `VecIntoIter` from a `Vec`.
  static constexpr auto with(Vec<Item>&& vec) noexcept {
    return VecIntoIter(::sus::move(vec));
  }

  /// sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    if (front_index_ == back_index_) [[unlikely]]
      return Option<Item>::none();
    // SAFETY: This class owns the Vec and does not expose it, so its length is
    // known and can not change. Thus the indices which are kept within the
    // length of the Vec can not go out of bounds.
    Item& item = vec_.get_unchecked_mut(
        ::sus::marker::unsafe_fn,
        ::sus::mem::replace(mref(front_index_), front_index_ + 1_usize));
    return Option<Item>::some(move(item));
  }

  /// sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept {
    if (front_index_ == back_index_) [[unlikely]]
      return Option<Item>::none();
    // SAFETY: This class owns the Vec and does not expose it, so its length is
    // known and can not change. Thus the indices which are kept within the
    // length of the Vec can not go out of bounds.
    back_index_ -= 1u;
    Item& item = vec_.get_unchecked_mut(::sus::marker::unsafe_fn, back_index_);
    return Option<Item>::some(move(item));
  }

  /// sus::iter::Iterator method.
  ::sus::iter::SizeHint size_hint() const noexcept final {
    const usize remaining = back_index_ - front_index_;
    return ::sus::iter::SizeHint(
        remaining, ::sus::Option<::sus::num::usize>::some(remaining));
  }

  /// sus::iter::ExactSizeIterator trait.
  ::sus::num::usize exact_size_hint() const noexcept {
    return back_index_ - front_index_;
  }

 private:
  VecIntoIter(Vec<Item>&& vec) noexcept : vec_(::sus::move(vec)) {}

  Vec<Item> vec_;
  usize front_index_ = 0_usize;
  usize back_index_ = vec_.len();

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(front_index_),
                                           decltype(back_index_),
                                           decltype(vec_));
};

}  // namespace sus::containers
