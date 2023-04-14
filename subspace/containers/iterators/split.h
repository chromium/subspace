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

#include "subspace/iter/iterator_defn.h"
#include "subspace/mem/replace.h"
#include "subspace/num/unsigned_integer.h"

namespace sus::containers {
template <class T>
class Slice;
template <class T>
class SliceMut;
}  // namespace sus::containers

namespace sus::ops {
template <class T>
  requires(::sus::ops::Ord<T>)
class RangeFrom;
template <class T>
  requires(::sus::ops::Ord<T>)
class RangeTo;
}  // namespace sus::ops

namespace sus::containers {

/// An iterator over subslices separated by elements that match a predicate
/// function.
///
/// This struct is created by the `split()` method on slices.
template <class ItemT>
struct [[nodiscard]] [[sus_trivial_abi]] Split final
    : public ::sus::iter::IteratorBase<Split<ItemT>,
                                       ::sus::containers::Slice<ItemT>> {
 public:
  // `Item` is a `Slice<T>`.
  using Item = ::sus::containers::Slice<ItemT>;

 private:
  // `SliceItem` is a `T`.
  using SliceItem = ItemT;

 public:
  Split(Split&&) = default;
  Split& operator=(Split&&) = default;

  /// sus::mem::Clone trait.
  Split clone() const noexcept {
    return Split(::sus::clone(v_), sus::clone(pred_));
  }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    Option<Item> ret;

    if (finished_) [[unlikely]] {
      return ret;
    }

    // TODO: Use v_.iter().position().
    ::sus::num::usize idx;
    auto it = v_.iter();
    while (true) {
      Option<const ItemT&> o = it.next();
      if (o.is_none()) {
        return finish();
      }
      if (pred_(*o)) {
        ret = Option<Item>::some(v_[::sus::ops::RangeTo(idx)]);
        v_ = v_[::sus::ops::RangeFrom(idx + 1u)];
        return ret;
      }
      idx += 1u;
    }
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept {
    Option<Item> ret;

    if (finished_) [[unlikely]] {
      return ret;
    }

    // TODO: Use v_.iter().rposition().
    ::sus::num::usize idx = v_.len();
    auto it = v_.iter().rev();
    while (true) {
      Option<const ItemT&> o = it.next();
      if (o.is_none()) {
        return finish();
      }
      idx -= 1u;
      if (pred_(*o)) {
        ret = Option<Item>::some(v_[::sus::ops::RangeFrom(idx + 1u)]);
        v_ = v_[::sus::ops::RangeTo(idx)];
        return ret;
      }
    }
  }

  // Replace the default impl in sus::iter::IteratorBase.
  ::sus::iter::SizeHint size_hint() const noexcept final {
    if (v_.is_empty()) {
      return {0u, ::sus::Option<::sus::num::usize>::some(0u)};
    } else {
      // If the predicate doesn't match anything, we yield one slice. If it
      // matches every element, we yield `len()` one-element slices, or a single
      // empty slice.
      return {1u, ::sus::Option<::sus::num::usize>::some(
                      ::sus::ops::max(1_usize, v_.len()))};
    }
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by Slice.
  friend class Slice<ItemT>;

  Option<Item> finish() noexcept {
    if (finished_) {
      return Option<Item>::none();
    } else {
      finished_ = true;
      return Option<Item>::some(v_);
    }
  }

  static constexpr auto with(
      Slice<ItemT> values,
      ::sus::fn::FnMutRef<bool(const ItemT&)>&& pred) noexcept {
    return Split(values, ::sus::move(pred));
  }

  constexpr Split(Slice<ItemT> values,
                  ::sus::fn::FnMutRef<bool(const ItemT&)>&& pred) noexcept
      : v_(values), pred_(::sus::move(pred)) {}

  Slice<ItemT> v_;
  ::sus::fn::FnMutRef<bool(const ItemT&)> pred_;
  bool finished_ = false;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(v_),
                                  decltype(pred_), decltype(finished_));
};

/// An iterator over subslices separated by elements that match a predicate
/// function.
///
/// This struct is created by the `split_mut()` method on slices.
template <class ItemT>
struct [[nodiscard]] [[sus_trivial_abi]] SplitMut final
    : public ::sus::iter::IteratorBase<SplitMut<ItemT>,
                                       ::sus::containers::SliceMut<ItemT>> {
 public:
  // `Item` is a `SliceMut<T>`.
  using Item = ::sus::containers::SliceMut<ItemT>;

 private:
  // `SliceItem` is a `T`.
  using SliceItem = ItemT;

 public:
  SplitMut(SplitMut&&) = default;
  SplitMut& operator=(SplitMut&&) = default;

  /// sus::mem::Clone trait.
  SplitMut clone() const noexcept {
    return SplitMut(::sus::clone(v_), sus::clone(pred_));
  }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    Option<Item> ret;

    if (finished_) [[unlikely]] {
      return ret;
    }

    // TODO: Use v_.iter().position().
    ::sus::num::usize idx;
    auto it = v_.iter();
    while (true) {
      Option<const ItemT&> o = it.next();
      if (o.is_none()) {
        return finish();
      }
      if (pred_(*o)) {
        ret = Option<Item>::some(v_[::sus::ops::RangeTo(idx)]);
        v_ = v_[::sus::ops::RangeFrom(idx + 1u)];
        return ret;
      }
      idx += 1u;
    }
  }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept {
    Option<Item> ret;

    if (finished_) [[unlikely]] {
      return ret;
    }

    // TODO: Use v_.iter().rposition().
    ::sus::num::usize idx = v_.len();
    auto it = v_.iter().rev();
    while (true) {
      Option<const ItemT&> o = it.next();
      if (o.is_none()) {
        return finish();
      }
      idx -= 1u;
      if (pred_(*o)) {
        ret = Option<Item>::some(v_[::sus::ops::RangeFrom(idx + 1u)]);
        v_ = v_[::sus::ops::RangeTo(idx)];
        return ret;
      }
    }
  }

  // Replace the default impl in sus::iter::IteratorBase.
  ::sus::iter::SizeHint size_hint() const noexcept final {
    if (v_.is_empty()) {
      return {0u, ::sus::Option<::sus::num::usize>::some(0u)};
    } else {
      // If the predicate doesn't match anything, we yield one slice. If it
      // matches every element, we yield `len()` one-element slices, or a single
      // empty slice.
      return {1u, ::sus::Option<::sus::num::usize>::some(
                      ::sus::ops::max(1_usize, v_.len()))};
    }
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by SliceMut.
  friend class SliceMut<ItemT>;

  Option<Item> finish() noexcept {
    if (finished_) {
      return Option<Item>::none();
    } else {
      finished_ = true;
      return Option<Item>::some(v_);
    }
  }

  static constexpr auto with(
      SliceMut<ItemT> values,
      ::sus::fn::FnMutRef<bool(const ItemT&)>&& pred) noexcept {
    return SplitMut(values, ::sus::move(pred));
  }

  constexpr SplitMut(SliceMut<ItemT> values,
                     ::sus::fn::FnMutRef<bool(const ItemT&)>&& pred) noexcept
      : v_(values), pred_(::sus::move(pred)) {}

  SliceMut<ItemT> v_;
  ::sus::fn::FnMutRef<bool(const ItemT&)> pred_;
  bool finished_ = false;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(v_),
                                  decltype(pred_), decltype(finished_));
};

}  // namespace sus::containers
