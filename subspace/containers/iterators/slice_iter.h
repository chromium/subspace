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
#include "subspace/iter/iterator_ref.h"
#include "subspace/lib/__private/forward_decl.h"
#include "subspace/macros/no_unique_address.h"
#include "subspace/mem/move.h"
#include "subspace/mem/mref.h"
#include "subspace/mem/nonnull.h"
#include "subspace/mem/relocate.h"
#include "subspace/mem/replace.h"
#include "subspace/num/signed_integer.h"
#include "subspace/num/unsigned_integer.h"

namespace sus::containers {

template <class ItemT>
struct [[nodiscard]] [[sus_trivial_abi]] SliceIter final
    : public ::sus::iter::IteratorBase<SliceIter<ItemT>, ItemT> {
 public:
  using Item = ItemT;

 private:
  // `Item` is a `const T&`.
  static_assert(std::is_reference_v<Item>);
  static_assert(std::is_const_v<std::remove_reference_t<Item>>);
  // `RawItem` is a `T`.
  using RawItem = std::remove_const_t<std::remove_reference_t<Item>>;

 public:
  static constexpr auto with(::sus::iter::IterRef ref, const RawItem* start,
                             usize len) noexcept {
    const RawItem* end = start + len;
    // Wrap-around would be an invalid allocation and would break our distance
    // functions.
    ::sus::check(end >= start);
    return SliceIter(::sus::move(ref), start, end);
  }

  /// Returns a slice of the items left to be iterated.
  Slice<Item> as_slice() const& {
    return Slice<Item>::from_raw_parts(::sus::marker::unsafe_fn, ptr_,
                                       end_ - ptr_);
  }

  Option<Item> next() noexcept {
    if (ptr_ == end_) [[unlikely]]
      return Option<Item>();
    // SAFETY: end_ is always > ptr_ when we get here (this was checked by the
    // constructor) so ptr_ will be inside the allocation, not pointing just
    // after it (like end_ may be).
    return Option<Item>::with(*::sus::mem::replace(mref(ptr_), ptr_ + 1u));
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept {
    if (ptr_ == end_) [[unlikely]]
      return Option<Item>();
    // SAFETY: end_ is always > ptr_ when we get here (this was checked by the
    // constructor) so subtracting one and dereffing will be inside the
    // allocation.
    end_ -= 1u;
    return Option<Item>::with(*end_);
  }

  ::sus::iter::SizeHint size_hint() const noexcept {
    // SAFETY: The constructor checks that end_ - ptr_ is positive and Slice can
    // not exceed isize::MAX.
    const auto remaining = ::sus::num::usize::from_unchecked(
        ::sus::marker::unsafe_fn, end_ - ptr_);
    return ::sus::iter::SizeHint(
        remaining, ::sus::Option<::sus::num::usize>::with(remaining));
  }

  /// sus::iter::ExactSizeIterator trait.
  ::sus::num::usize exact_size_hint() const noexcept {
    // SAFETY: The constructor checks that end_ - ptr_ is positive and Slice can
    // not exceed isize::MAX.
    return ::sus::num::usize::from_unchecked(::sus::marker::unsafe_fn,
                                             end_ - ptr_);
  }

 private:
  constexpr SliceIter(::sus::iter::IterRef ref, const RawItem* start,
                      const RawItem* end) noexcept
      : ref_(::sus::move(ref)), ptr_(start), end_(end) {}

  [[sus_no_unique_address]] ::sus::iter::IterRef ref_;
  const RawItem* ptr_;
  const RawItem* end_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(ptr_),
                                  decltype(end_), decltype(ref_));
};

static_assert(::sus::mem::Copy<SliceIter<const i32&>>);
static_assert(::sus::mem::Move<SliceIter<const i32&>>);

template <class ItemT>
struct [[sus_trivial_abi]] SliceIterMut final
    : public ::sus::iter::IteratorBase<SliceIterMut<ItemT>, ItemT> {
 public:
  using Item = ItemT;

 private:
  // `Item` is a `T&`.
  static_assert(std::is_reference_v<Item>);
  static_assert(!std::is_const_v<std::remove_reference_t<Item>>);
  // `RawItem` is a `T`.
  using RawItem = std::remove_reference_t<Item>;

 public:
  static constexpr auto with(::sus::iter::IterRef ref, RawItem* start,
                             usize len) noexcept {
    RawItem* end = start + len;
    // Wrap-around would be an invalid allocation and would break our distance
    // functions.
    ::sus::check(end >= start);
    return SliceIterMut(sus::move(ref), start, end);
  }

  /// Returns a mutable slice of the items left to be iterated, consuming the
  /// iterator.
  SliceMut<RawItem> as_mut_slice() && {
    return SliceMut<RawItem>::from_raw_parts_mut(::sus::marker::unsafe_fn,
                                                 ref_.to_view(), ptr_,
                                                 usize::from(end_ - ptr_));
  }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    if (ptr_ == end_) [[unlikely]]
      return Option<Item>();
    // SAFETY: end_ is always > ptr_ when we get here (this was checked by the
    // constructor) so ptr_ will be inside the allocation, not pointing just
    // after it (like end_ may be).
    return Option<Item>::with(
        mref(*::sus::mem::replace(mref(ptr_), ptr_ + 1u)));
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept {
    if (ptr_ == end_) [[unlikely]]
      return Option<Item>();
    // SAFETY: end_ is always > ptr_ when we get here (this was checked by the
    // constructor) so subtracting one and dereffing will be inside the
    // allocation.
    end_ -= 1u;
    return Option<Item>::with(mref(*end_));
  }

  ::sus::iter::SizeHint size_hint() const noexcept {
    const auto remaining = exact_size_hint();
    return {remaining, ::sus::Option<::sus::num::usize>::with(remaining)};
  }

  /// sus::iter::ExactSizeIterator trait.
  ::sus::num::usize exact_size_hint() const noexcept {
    // SAFETY: The constructor checks that end_ - ptr_ is positive and Slice can
    // not exceed isize::MAX.
    return ::sus::num::usize::from_unchecked(::sus::marker::unsafe_fn,
                                             end_ - ptr_);
  }

 private:
  constexpr SliceIterMut(::sus::iter::IterRef ref, RawItem* start,
                         RawItem* end) noexcept
      : ref_(::sus::move(ref)), ptr_(start), end_(end) {
    int i = 1;
    (void)i;
  }

  [[sus_no_unique_address]] ::sus::iter::IterRef ref_;
  RawItem* ptr_;
  RawItem* end_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(ptr_),
                                  decltype(end_), decltype(ref_));
};

static_assert(::sus::mem::Copy<SliceIterMut<i32&>>);
static_assert(::sus::mem::Move<SliceIterMut<i32&>>);

}  // namespace sus::containers
