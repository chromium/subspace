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

#pragma once

/// An iterator over overlapping subslices of length size.
///
/// This struct is created by the `windows()` method on slices.

#include "subspace/iter/iterator_defn.h"
#include "subspace/mem/relocate.h"
#include "subspace/num/unsigned_integer.h"
#include "subspace/ops/range.h"

namespace sus::containers {

/// An iterator over overlapping subslices of length `size`.
///
/// This struct is created by the `windows()` method on slices.
template <class ItemT>
class [[nodiscard]] [[sus_trivial_abi]] Windows final
    : public ::sus::iter::IteratorBase<Windows<ItemT>,
                                       ::sus::containers::Slice<ItemT>> {
 public:
  // `Item` is a `Slice<T>`.
  using Item = ::sus::containers::Slice<ItemT>;

  Windows(Windows&&) = default;
  Windows& operator=(Windows&&) = default;

  /// sus::mem::Clone trait.
  Windows clone() const noexcept {
    return Windows(::sus::clone(v_), ::sus::clone(size_));
  }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    Option<Item> ret;

    if (size_ > v_.len()) {
      return ret;
    } else {
      ret = Option<Item>::some(v_[::sus::ops::RangeTo<usize>(size_)]);
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
      ret = Option<Item>::some(
          v_[::sus::ops::RangeFrom<usize>(v_.len() - size_)]);
      v_ = v_[::sus::ops::RangeTo<usize>(v_.len() - 1u)];
      return ret;
    }
  }

  // Replace the default impl in sus::iter::IteratorBase.
  ::sus::iter::SizeHint size_hint() const noexcept final {
    const auto remaining = exact_size_hint();
    return {remaining, ::sus::Option<::sus::num::usize>::some(remaining)};
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
  // Constructed by Slice.
  friend class Slice<ItemT>;

  static constexpr auto with(Slice<ItemT> values,
                             /* TODO: NonZeroUsize*/ usize size) noexcept {
    return Windows(values, size);
  }

  constexpr Windows(Slice<ItemT> values,
                    /* TODO: NonZeroUsize*/ usize size) noexcept
      : v_(values), size_(size) {}

  Slice<ItemT> v_;
  /* TODO: NonZeroUsize*/ usize size_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(v_),
                                  decltype(size_));
};

/// An iterator over overlapping subslices of length `size`.
///
/// This struct is created by the `windows()` method on slices.
template <class ItemT>
class [[nodiscard]] [[sus_trivial_abi]] WindowsMut final
    : public ::sus::iter::IteratorBase<WindowsMut<ItemT>,
                                       ::sus::containers::SliceMut<ItemT>> {
 public:
  // `Item` is a `SliceMut<T>`.
  using Item = ::sus::containers::SliceMut<ItemT>;

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
      ret = Option<Item>::some(v_[::sus::ops::RangeTo<usize>(size_)]);
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
      ret = Option<Item>::some(
          v_[::sus::ops::RangeFrom<usize>(v_.len() - size_)]);
      v_ = v_[::sus::ops::RangeTo<usize>(v_.len() - 1u)];
      return ret;
    }
  }

  // Replace the default impl in sus::iter::IteratorBase.
  ::sus::iter::SizeHint size_hint() const noexcept final {
    const auto remaining = exact_size_hint();
    return {remaining, ::sus::Option<::sus::num::usize>::some(remaining)};
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
  // Constructed by SliceMut.
  friend class SliceMut<ItemT>;

  static constexpr auto with(SliceMut<ItemT> values,
                             /* TODO: NonZeroUsize*/ usize size) noexcept {
    return WindowsMut(values, size);
  }

  constexpr WindowsMut(SliceMut<ItemT> values,
                    /* TODO: NonZeroUsize*/ usize size) noexcept
      : v_(values), size_(size) {}

  SliceMut<ItemT> v_;
  /* TODO: NonZeroUsize*/ usize size_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(v_),
                                  decltype(size_));
};

}  // namespace sus::containers
