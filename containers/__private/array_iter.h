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
#include "marker/unsafe.h"
#include "mem/move.h"
#include "mem/mref.h"
#include "num/unsigned_integer.h"

namespace sus::containers {

template <class T, size_t N>
  requires(N <= PTRDIFF_MAX)
class Array;

template <::sus::mem::Move Item, size_t N>
struct ArrayIntoIter : public ::sus::iter::IteratorBase<Item> {
 public:
  static constexpr auto with(Array<Item, N>&& array) noexcept {
    return ::sus::iter::Iterator<ArrayIntoIter>(::sus::move(array));
  }

  Option<Item> next() noexcept final {
    if (next_index_.primitive_value == N) [[unlikely]]
      return Option<Item>::none();
    // SAFETY: The array has a fixed size. The next_index_ is encapsulated and
    // only changed in this class/method. The next_index_ stops incrementing
    // when it reaches N and starts at 0, and N >= 0, so when we get here we
    // know next_index_ is in range of the array. We use get_unchecked_mut()
    // here because it's difficult for the compiler to make the same
    // observations we have here, as next_index_ is a field and changes across
    // multiple method calls.
    Item& item = array_.get_unchecked_mut(
        ::sus::marker::unsafe_fn,
        ::sus::mem::replace(mref(next_index_), next_index_ + 1_usize));
    return Option<Item>::some(move(item));
  }

 protected:
  ArrayIntoIter(Array<Item, N>&& array) noexcept : array_(::sus::move(array)) {}

 private:
  usize next_index_ = 0_usize;
  Array<Item, N> array_;

  sus_class_maybe_trivial_relocatable_types(::sus::marker::unsafe_fn,
                                            decltype(next_index_),
                                            decltype(array_));
};

}  // namespace sus::containers
