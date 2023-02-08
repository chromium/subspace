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
#include "subspace/mem/move.h"
#include "subspace/mem/mref.h"
#include "subspace/mem/nonnull.h"
#include "subspace/mem/relocate.h"
#include "subspace/mem/replace.h"
#include "subspace/num/unsigned_integer.h"

namespace sus::containers {

template <class T>
class Slice;

template <class ItemT>
struct [[sus_trivial_abi]] SliceIter final
    : public ::sus::iter::IteratorImpl<SliceIter<ItemT>, ItemT> {
 public:
  using Item = ItemT;

 private:
  // `Item` is a `const T&`.
  static_assert(std::is_reference_v<Item>);
  static_assert(std::is_const_v<std::remove_reference_t<Item>>);
  // `RawItem` is a `T`.
  using RawItem = std::remove_const_t<std::remove_reference_t<Item>>;

 public:
  static constexpr auto with(const RawItem* start, usize len) noexcept {
    return SliceIter(start, len);
  }

  Option<Item> next() noexcept final {
    if (ptr_ == end_) [[unlikely]]
      return Option<Item>::none();
    // SAFETY: Since end_ > ptr_, which is checked in the constructor, ptr_ + 1
    // will never be null.
    return Option<Item>::some(*::sus::mem::replace_ptr(mref(ptr_), ptr_ + 1u));
  }

  ::sus::iter::SizeHint size_hint() noexcept final {
    // SAFETY: end_ is always larger than ptr_ which is only incremented until
    // end_, so this static cast does not drop a negative sign bit. That ptr_
    // starts at or before end_ is checked in the constructor.
    const usize remaining = static_cast<size_t>(end_ - ptr_);
    return ::sus::iter::SizeHint(
        remaining, ::sus::Option<::sus::num::usize>::some(remaining));
  }

 private:
  constexpr SliceIter(const RawItem* start, usize len) noexcept
      : ptr_(start), end_(start + len.primitive_value) {
    check(end_ >= ptr_ || !end_);  // end_ may wrap around to 0, but not past 0.
  }

  const RawItem* ptr_;
  const RawItem* end_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(ptr_),
                                  decltype(end_));
};

template <class ItemT>
struct [[sus_trivial_abi]] SliceIterMut final
    : public ::sus::iter::IteratorImpl<SliceIterMut<ItemT>, ItemT> {
 public:
  using Item = ItemT;

 private:
  // `Item` is a `const T&`.
  static_assert(std::is_reference_v<Item>);
  static_assert(!std::is_const_v<std::remove_reference_t<Item>>);
  // `RawItem` is a `T`.
  using RawItem = std::remove_reference_t<Item>;

 public:
  static constexpr auto with(RawItem* start, usize len) noexcept {
    return SliceIterMut(start, len);
  }

  Option<Item> next() noexcept final {
    if (ptr_ == end_) [[unlikely]]
      return Option<Item>::none();
    // SAFETY: Since end_ > ptr_, which is checked in the constructor, ptr_ + 1
    // will never be null.
    return Option<Item>::some(
        mref(*::sus::mem::replace_ptr(mref(ptr_), ptr_ + 1u)));
  }

  ::sus::iter::SizeHint size_hint() noexcept final {
    // SAFETY: end_ is always larger than ptr_ which is only incremented until
    // end_, so this static cast does not drop a negative sign bit. That ptr_
    // starts at or before end_ is checked in the constructor.
    const usize remaining = static_cast<size_t>(end_ - ptr_);
    return ::sus::iter::SizeHint(
        remaining, ::sus::Option<::sus::num::usize>::some(remaining));
  }

 private:
  constexpr SliceIterMut(RawItem* start, usize len) noexcept
      : ptr_(start), end_(start + len.primitive_value) {
    check(end_ >= ptr_ || !end_);  // end_ may wrap around to 0, but not past 0.
  }

  RawItem* ptr_;
  RawItem* end_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(ptr_),
                                  decltype(end_));
};

}  // namespace sus::containers
