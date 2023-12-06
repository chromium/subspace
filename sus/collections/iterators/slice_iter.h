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

// IWYU pragma: private, include "sus/collections/slice.h"
// IWYU pragma: friend "sus/.*"
#pragma once

#include <stdint.h>

#include <type_traits>

#include "sus/iter/iterator_defn.h"
#include "sus/iter/iterator_ref.h"
#include "sus/lib/__private/forward_decl.h"
#include "sus/macros/no_unique_address.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"
#include "sus/mem/replace.h"
#include "sus/num/signed_integer.h"
#include "sus/num/unsigned_integer.h"
#include "sus/ptr/nonnull.h"

namespace sus::collections {

/// An iterator over a contiguous array of objects with const access to them.
///
/// This type is returned from `Vec::iter()` and `Slice::iter()` among others.
template <class ItemT>
struct [[nodiscard]] [[_sus_trivial_abi]] SliceIter final
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
  explicit constexpr SliceIter(::sus::iter::IterRef ref, const RawItem* start,
                               usize len) noexcept
      : ref_(::sus::move(ref)), ptr_(start), end_(start + len) {
    // Wrap-around would be an invalid allocation and would break our distance
    // functions.
    sus_check(end_ >= ptr_);
  }

  /// Returns a slice of the items left to be iterated.
  constexpr Slice<Item> as_slice() const& {
    return Slice<Item>::from_raw_collection(::sus::marker::unsafe_fn, ptr_,
                                            end_ - ptr_);
  }

  constexpr Option<Item> next() noexcept {
    if (ptr_ == end_) [[unlikely]]
      return Option<Item>();
    // SAFETY: end_ is always > ptr_ when we get here (this was checked by the
    // constructor) so ptr_ will be inside the allocation, not pointing just
    // after it (like end_ may be).
    return Option<Item>(*::sus::mem::replace(ptr_, ptr_ + 1u));
  }

  // sus::iter::DoubleEndedIterator trait.
  constexpr Option<Item> next_back() noexcept {
    if (ptr_ == end_) [[unlikely]]
      return Option<Item>();
    // SAFETY: end_ is always > ptr_ when we get here (this was checked by the
    // constructor) so subtracting one and dereffing will be inside the
    // allocation.
    end_ -= 1u;
    return Option<Item>(*end_);
  }

  constexpr ::sus::iter::SizeHint size_hint() const noexcept {
    const auto remaining = exact_size_hint();
    return ::sus::iter::SizeHint(remaining,
                                 ::sus::Option<::sus::num::usize>(remaining));
  }

  /// sus::iter::ExactSizeIterator trait.
  constexpr ::sus::num::usize exact_size_hint() const noexcept {
    // SAFETY: The constructor checks that `end_ - ptr_` is positive and Slice
    // can not exceed isize::MAX.
    return ::sus::num::usize::try_from(end_ - ptr_)
        .unwrap_unchecked(::sus::marker::unsafe_fn);
  }

  /// sus::iter::TrustedLen trait.
  /// #[doc.hidden]
  constexpr ::sus::iter::__private::TrustedLenMarker trusted_len()
      const noexcept {
    return {};
  }

 private:
  [[_sus_no_unique_address]] ::sus::iter::IterRef ref_;
  const RawItem* ptr_;
  const RawItem* end_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(ptr_),
                                  decltype(end_), decltype(ref_));
};

static_assert(::sus::mem::Copy<SliceIter<const i32&>>);
static_assert(::sus::mem::Move<SliceIter<const i32&>>);

/// An iterator over a contiguous array of objects with mutable access to them.
///
/// This type is returned from `Vec::iter_mut()` and `Slice::iter_mut()` among
/// others.
template <class ItemT>
struct [[_sus_trivial_abi]] SliceIterMut final
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
  explicit constexpr SliceIterMut(::sus::iter::IterRef ref, RawItem* start,
                                  usize len) noexcept
      : ref_(::sus::move(ref)), ptr_(start), end_(start + len) {
    // Wrap-around would be an invalid allocation and would break our distance
    // functions.
    sus_check(end_ >= ptr_);
  }

  /// Returns a mutable slice of the items left to be iterated, consuming the
  /// iterator.
  constexpr SliceMut<RawItem> as_mut_slice() && {
    return SliceMut<RawItem>::from_raw_collection_mut(
        ::sus::marker::unsafe_fn, ref_.to_view(), ptr_,
        // SAFETY: `end_ > ptr_` at all times, and the distance between two
        // pointers in a single allocation is at most isize::MAX which fits in
        // usize.
        usize::try_from(end_ - ptr_)
            .unwrap_unchecked(::sus::marker::unsafe_fn));
  }

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept {
    if (ptr_ == end_) [[unlikely]]
      return Option<Item>();
    // SAFETY: end_ is always > ptr_ when we get here (this was checked by the
    // constructor) so ptr_ will be inside the allocation, not pointing just
    // after it (like end_ may be).
    return Option<Item>(*::sus::mem::replace(ptr_, ptr_ + 1u));
  }

  // sus::iter::DoubleEndedIterator trait.
  constexpr Option<Item> next_back() noexcept {
    if (ptr_ == end_) [[unlikely]]
      return Option<Item>();
    // SAFETY: end_ is always > ptr_ when we get here (this was checked by the
    // constructor) so subtracting one and dereffing will be inside the
    // allocation.
    end_ -= 1u;
    return Option<Item>(*end_);
  }

  constexpr ::sus::iter::SizeHint size_hint() const noexcept {
    const auto remaining = exact_size_hint();
    return {remaining, ::sus::Option<::sus::num::usize>(remaining)};
  }

  /// sus::iter::ExactSizeIterator trait.
  constexpr usize exact_size_hint() const noexcept {
    // SAFETY: The constructor checks that end_ - ptr_ is positive and Slice can
    // not exceed isize::MAX.
    return usize::try_from(end_ - ptr_)
        .unwrap_unchecked(::sus::marker::unsafe_fn);
  }

  /// sus::iter::TrustedLen trait.
  /// #[doc.hidden]
  constexpr ::sus::iter::__private::TrustedLenMarker trusted_len()
      const noexcept {
    return {};
  }

 private:
  [[_sus_no_unique_address]] ::sus::iter::IterRef ref_;
  RawItem* ptr_;
  RawItem* end_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(ptr_),
                                  decltype(end_), decltype(ref_));
};

static_assert(::sus::mem::Copy<SliceIterMut<i32&>>);
static_assert(::sus::mem::Move<SliceIterMut<i32&>>);

}  // namespace sus::collections
