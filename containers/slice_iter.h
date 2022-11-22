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
#include "mem/nonnull.h"
#include "mem/relocate.h"
#include "mem/replace.h"
#include "num/unsigned_integer.h"

namespace sus::containers {

template <class T>
class Slice;

template <class Item>
struct [[sus_trivial_abi]] SliceIter
    : public ::sus::iter::IteratorBase<const Item&> {
 public:
  static constexpr auto with(const Item* start, usize len) noexcept {
    return ::sus::iter::Iterator<SliceIter>(start, len);
  }

  Option<const Item&> next() noexcept final {
    if (ptr_ == end_) [[unlikely]]
      return Option<const Item&>::none();
    // SAFETY: Since end_ > ptr_, which is checked in the constructor, ptr_ + 1
    // will never be null.
    return Option<const Item&>::some(
        *::sus::mem::replace_ptr(mref(ptr_), ptr_ + 1_usize));
  }

 protected:
  constexpr SliceIter(const Item* start, usize len) noexcept
      : ptr_(start), end_(start + len) {
    check(end_ > ptr_ || !end_);  // end_ may wrap around to 0, but not past 0.
  }

 private:
  const Item* ptr_;
  const Item* end_;

  sus_class_assert_trivial_relocatable_types(unsafe_fn, decltype(ptr_), decltype(end_));
};

template <class Item>
struct [[sus_trivial_abi]] SliceIterMut
    : public ::sus::iter::IteratorBase<Item&> {
 public:
  static constexpr auto with(Item* start, usize len) noexcept {
    return ::sus::iter::Iterator<SliceIterMut>(start, len);
  }

  Option<Item&> next() noexcept final {
    if (ptr_ == end_) [[unlikely]]
      return Option<Item&>::none();
    // SAFETY: Since end_ > ptr_, which is checked in the constructor, ptr_ + 1
    // will never be null.
    return Option<Item&>::some(
        mref(*::sus::mem::replace_ptr(mref(ptr_), ptr_ + 1_usize)));
  }

 protected:
  constexpr SliceIterMut(Item* start, usize len) noexcept
      : ptr_(start), end_(start + len) {
    check(end_ > ptr_ || !end_);  // end_ may wrap around to 0, but not past 0.
  }

 private:
  Item* ptr_;
  Item* end_;

  sus_class_assert_trivial_relocatable_types(unsafe_fn, decltype(ptr_), decltype(end_));
};

}  // namespace sus::containers
