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

#include "iter/iterator_defn.h"
#include "mem/move.h"
#include "mem/mref.h"
#include "num/unsigned_integer.h"

namespace sus::containers {

template <class T, size_t N>
  requires(N <= PTRDIFF_MAX)
class Array;

template <class Item, size_t N>
struct ArrayIter : public ::sus::iter::IteratorBase<const Item&> {
 public:
  static constexpr auto with(const Array<Item, N>& array) noexcept {
    return ::sus::iter::Iterator<ArrayIter>(array);
  }

  Option<const Item&> next() noexcept final {
    if (next_index_.primitive_value == N) [[unlikely]]
      return Option<const Item&>::none();
    return Option<const Item&>::some(array_.get(
        ::sus::mem::replace(mref(next_index_), next_index_ + 1_usize)));
  }

 protected:
  ArrayIter(const Array<Item, N>& array) noexcept : array_(array) {}

 private:
  usize next_index_ = 0_usize;
  const Array<Item, N>& array_;
};

template <class Item, size_t N>
struct ArrayIterMut : public ::sus::iter::IteratorBase<Item&> {
 public:
  static constexpr auto with(Array<Item, N>& array) noexcept {
    return ::sus::iter::Iterator<ArrayIterMut>(array);
  }

  Option<Item&> next() noexcept final {
    if (next_index_.primitive_value == N) [[unlikely]]
      return Option<Item&>::none();
    return Option<Item&>::some(mref(array_.get_mut(
        ::sus::mem::replace(mref(next_index_), next_index_ + 1_usize))));
  }

 protected:
  ArrayIterMut(Array<Item, N>& array) noexcept : array_(array) {}

 private:
  usize next_index_ = 0_usize;
  Array<Item, N>& array_;
};

template <class Item, size_t N>
  requires(std::is_move_constructible_v<Item>)
struct ArrayIntoIter : public ::sus::iter::IteratorBase<Item> {
 public:
  static constexpr auto with(Array<Item, N>&& array) noexcept {
    return ::sus::iter::Iterator<ArrayIntoIter>(::sus::move(array));
  }

  Option<Item> next() noexcept final {
    if (next_index_.primitive_value == N) [[unlikely]]
      return Option<Item>::none();
    return Option<Item>::some(move(array_.get_mut(
        ::sus::mem::replace(mref(next_index_), next_index_ + 1_usize))));
  }

 protected:
  ArrayIntoIter(Array<Item, N>&& array) noexcept : array_(::sus::move(array)) {}

 private:
  usize next_index_ = 0_usize;
  Array<Item, N> array_;
};

}  // namespace sus::containers
