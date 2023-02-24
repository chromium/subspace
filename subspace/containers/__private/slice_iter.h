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
#include "subspace/num/signed_integer.h"
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
    // SAFETY: end_ is always > ptr_ when we get here (this was checked by the
    // constructor) so ptr_ will be inside the allocation, not pointing just
    // after it (like end_ may be).
    return Option<Item>::some(*::sus::mem::replace_ptr(mref(ptr_), ptr_ + 1u));
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept {
    if (ptr_ == end_) [[unlikely]]
      return Option<Item>::none();
    // SAFETY: end_ is always > ptr_ when we get here (this was checked by the
    // constructor) so subtracting one and dereffing will be inside the
    // allocation.
    end_ -= 1u;
    return Option<Item>::some(*end_);
  }

  ::sus::iter::SizeHint size_hint() const noexcept final {
    // SAFETY: The constructor checks that end_ - ptr_ is positive and Slice can
    // not exceed isize::MAX.
    const auto remaining = ::sus::num::usize::from_unchecked(
        ::sus::marker::unsafe_fn, end_ - ptr_);
    return ::sus::iter::SizeHint(
        remaining, ::sus::Option<::sus::num::usize>::some(remaining));
  }

  /// sus::iter::ExactSizeIterator trait.
  ::sus::num::usize exact_size_hint() const noexcept {
    // SAFETY: The constructor checks that end_ - ptr_ is positive and Slice can
    // not exceed isize::MAX.
    return ::sus::num::usize::from_unchecked(::sus::marker::unsafe_fn,
                                             end_ - ptr_);
  }

 private:
  constexpr SliceIter(const RawItem* start, usize len) noexcept
      : ptr_(start), end_(start + size_t{len}) {
    // Wrap-around would be an invalid allocation and would break our distance
    // functions.
    check(end_ >= ptr_);
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

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept final {
    if (ptr_ == end_) [[unlikely]]
      return Option<Item>::none();
    // SAFETY: end_ is always > ptr_ when we get here (this was checked by the
    // constructor) so ptr_ will be inside the allocation, not pointing just
    // after it (like end_ may be).
    return Option<Item>::some(
        mref(*::sus::mem::replace_ptr(mref(ptr_), ptr_ + 1u)));
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept {
    if (ptr_ == end_) [[unlikely]]
      return Option<Item>::none();
    // SAFETY: end_ is always > ptr_ when we get here (this was checked by the
    // constructor) so subtracting one and dereffing will be inside the
    // allocation.
    end_ -= 1u;
    return Option<Item>::some(mref(*end_));
  }

  ::sus::iter::SizeHint size_hint() const noexcept final {
    // SAFETY: The constructor checks that end_ - ptr_ is positive and Slice can
    // not exceed isize::MAX.
    const auto remaining = ::sus::num::usize::from_unchecked(
        ::sus::marker::unsafe_fn, end_ - ptr_);
    return ::sus::iter::SizeHint(
        remaining, ::sus::Option<::sus::num::usize>::some(remaining));
  }

  /// sus::iter::ExactSizeIterator trait.
  ::sus::num::usize exact_size_hint() const noexcept {
    // SAFETY: The constructor checks that end_ - ptr_ is positive and Slice can
    // not exceed isize::MAX.
    return ::sus::num::usize::from_unchecked(::sus::marker::unsafe_fn,
                                             end_ - ptr_);
  }

 private:
  constexpr SliceIterMut(RawItem* start, usize len) noexcept
      : ptr_(start), end_(start + size_t{len}) {
    // Wrap-around would be an invalid allocation and would break our distance
    // functions.
    check(end_ >= ptr_);
  }

  RawItem* ptr_;
  RawItem* end_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(ptr_),
                                  decltype(end_));
};

}  // namespace sus::containers
