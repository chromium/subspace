// Copyright 2023 Google LLC
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

/// An iterator over overlapping subslices of length size.
///
/// This struct is created by the `windows()` method on slices.

#include "sus/iter/iterator_defn.h"
#include "sus/iter/iterator_ref.h"
#include "sus/lib/__private/forward_decl.h"
#include "sus/macros/no_unique_address.h"
#include "sus/mem/relocate.h"
#include "sus/num/unsigned_integer.h"
#include "sus/ops/range.h"

namespace sus::collections {

/// An iterator over overlapping subslices of length `size`.
///
/// This struct is created by the `windows()` method on slices.
template <class ItemT>
class [[nodiscard]] [[_sus_trivial_abi]] Windows final
    : public ::sus::iter::IteratorBase<Windows<ItemT>,
                                       ::sus::collections::Slice<ItemT>> {
 public:
  // `Item` is a `Slice<T>`.
  using Item = ::sus::collections::Slice<ItemT>;

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept {
    Option<Item> ret;

    if (size_ > v_.len()) {
      return ret;
    } else {
      ret = Option<Item>(v_[::sus::ops::RangeTo<usize>(size_)]);
      v_ = v_[::sus::ops::RangeFrom<usize>(1u)];
      return ret;
    }
  }

  // sus::iter::DoubleEndedIterator trait.
  constexpr Option<Item> next_back() noexcept {
    Option<Item> ret;

    if (size_ > v_.len()) {
      return ret;
    } else {
      ret = Option<Item>(
          v_[::sus::ops::RangeFrom<usize>(v_.len() - size_)]);
      v_ = v_[::sus::ops::RangeTo<usize>(v_.len() - 1u)];
      return ret;
    }
  }

  // Replace the default impl in sus::iter::IteratorBase.
  constexpr ::sus::iter::SizeHint size_hint() const noexcept {
    const auto remaining = exact_size_hint();
    return {remaining, ::sus::Option<::sus::num::usize>(remaining)};
  }

  /// sus::iter::ExactSizeIterator trait.
  constexpr ::sus::num::usize exact_size_hint() const noexcept {
    if (size_ > v_.len()) {
      return 0u;
    } else {
      return v_.len() - size_ + 1u;
    }
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by Slice, Vec, Array.
  friend class Slice<ItemT>;
  friend class Vec<ItemT>;
  template <class ArrayItemT, size_t N>
    friend class Array;

  constexpr Windows(::sus::iter::IterRef ref, const Slice<ItemT>& values,
                    /* TODO: NonZeroUsize*/ usize size) noexcept
      : ref_(::sus::move(ref)), v_(values), size_(size) {}

  [[sus_no_unique_address]] ::sus::iter::IterRef ref_;
  Slice<ItemT> v_;
  /* TODO: NonZeroUsize*/ usize size_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(ref_),
                                  decltype(v_), decltype(size_));
};

/// An iterator over overlapping subslices of length `size`.
///
/// This struct is created by the `windows()` method on slices.
template <class ItemT>
class [[nodiscard]] [[_sus_trivial_abi]] WindowsMut final
    : public ::sus::iter::IteratorBase<WindowsMut<ItemT>,
                                       ::sus::collections::SliceMut<ItemT>> {
 public:
  // `Item` is a `SliceMut<T>`.
  using Item = ::sus::collections::SliceMut<ItemT>;

  WindowsMut(WindowsMut&&) = default;
  WindowsMut& operator=(WindowsMut&&) = default;

  /// sus::mem::Clone trait.
  WindowsMut clone() const noexcept {
    return WindowsMut(::sus::clone(v_), ::sus::clone(size_));
  }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    Option<Item> ret;

    if (size_ > v_.len()) {
      return ret;
    } else {
      ret = Option<Item>(v_[::sus::ops::RangeTo<usize>(size_)]);
      v_ = v_[::sus::ops::RangeFrom<usize>(1u)];
      return ret;
    }
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept {
    Option<Item> ret;

    if (size_ > v_.len()) {
      return ret;
    } else {
      ret = Option<Item>(
          v_[::sus::ops::RangeFrom<usize>(v_.len() - size_)]);
      v_ = v_[::sus::ops::RangeTo<usize>(v_.len() - 1u)];
      return ret;
    }
  }

  // Replace the default impl in sus::iter::IteratorBase.
  ::sus::iter::SizeHint size_hint() const noexcept {
    const auto remaining = exact_size_hint();
    return {remaining, ::sus::Option<::sus::num::usize>(remaining)};
  }

  /// sus::iter::ExactSizeIterator trait.
  ::sus::num::usize exact_size_hint() const noexcept {
    if (size_ > v_.len()) {
      return 0u;
    } else {
      return v_.len() - size_ + 1u;
    }
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by SliceMut, Vec, Array.
  friend class SliceMut<ItemT>;
  friend class Vec<ItemT>;
  template <class ArrayItemT, size_t N>
    friend class Array;

  constexpr WindowsMut(::sus::iter::IterRef ref, const SliceMut<ItemT>& values,
                       /* TODO: NonZeroUsize*/ usize size) noexcept
      : ref_(::sus::move(ref)), v_(values), size_(size) {}

  [[sus_no_unique_address]] ::sus::iter::IterRef ref_;
  SliceMut<ItemT> v_;
  /* TODO: NonZeroUsize*/ usize size_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(ref_),
                                  decltype(v_), decltype(size_));
};

}  // namespace sus::collections
