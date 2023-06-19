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
#include "subspace/ops/range.h"

namespace sus::containers {
template <class T>
class Slice;
template <class T>
class SliceMut;
}  // namespace sus::containers

namespace sus::containers {

namespace __private {

/// An private iterator over subslices separated by elements that
/// match a predicate function, splitting at most a fixed number of
/// times.
template <class ItemT, ::sus::iter::Iterator<ItemT> I>
class [[sus_trivial_abi]] GenericSplitN final
    : public ::sus::iter::IteratorBase<GenericSplitN<ItemT, I>, ItemT> {
 public:
  using Item = ItemT;

  explicit GenericSplitN(I&& iter, usize count)
      : iter_(::sus::move(iter)), count_(count) {}

  GenericSplitN(GenericSplitN&&) = default;
  GenericSplitN& operator=(GenericSplitN&&) = default;

  Option<Item> next() noexcept {
    if (count_ == 0u)
      return Option<Item>();
    else if (count_ == 1u) {
      count_ -= 1u;
      return iter_.finish();
    } else {
      count_ -= 1u;
      return iter_.next();
    }
  }

  ::sus::iter::SizeHint size_hint() const noexcept {
    auto [lower, upper_opt] = iter_.size_hint();
    const auto count = count_;
    return {::sus::ops::min(count, lower),
            Option<usize>::with(
                ::sus::move(upper_opt).map_or(count, [count](usize upper) {
                  return ::sus::ops::min(count, upper);
                }))};
  }

 private:
  I iter_;
  usize count_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(iter_),
                                  decltype(count_));
};

}  // namespace __private

/// An iterator over subslices separated by elements that match a predicate
/// function.
///
/// This struct is created by the `split()` method on slices.
template <class ItemT>
class [[nodiscard]] [[sus_trivial_abi]] Split final
    : public ::sus::iter::IteratorBase<Split<ItemT>,
                                       ::sus::containers::Slice<ItemT>> {
 public:
  // `Item` is a `Slice<T>`.
  using Item = ::sus::containers::Slice<ItemT>;

  Split(Split&&) = default;
  Split& operator=(Split&&) = default;

  /// sus::mem::Clone trait.
  Split clone() const noexcept {
    return Split(::sus::clone(v_), ::sus::clone(pred_));
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
        ret = Option<Item>::with(v_[::sus::ops::RangeTo(idx)]);
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
        ret = Option<Item>::with(v_[::sus::ops::RangeFrom(idx + 1u)]);
        v_ = v_[::sus::ops::RangeTo(idx)];
        return ret;
      }
    }
  }

  // Replace the default impl in sus::iter::IteratorBase.
  ::sus::iter::SizeHint size_hint() const noexcept {
    if (v_.is_empty()) {
      return {0u, ::sus::Option<::sus::num::usize>::with(0u)};
    } else {
      // If the predicate doesn't match anything, we yield one slice. If it
      // matches every element, we yield `len()` one-element slices, or a single
      // empty slice.
      return {1u, ::sus::Option<::sus::num::usize>::with(
                      ::sus::ops::max(1_usize, v_.len()))};
    }
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by Slice.
  friend class Slice<ItemT>;
  // Access to finish().
  template <class A, ::sus::iter::Iterator<A> B>
  friend class __private::GenericSplitN;
  // Access to finish().
  template <class A>
  friend class RSplit;

  Option<Item> finish() noexcept {
    if (finished_) {
      return Option<Item>();
    } else {
      finished_ = true;
      return Option<Item>::with(v_);
    }
  }

  static constexpr auto with(
      const Slice<ItemT>& values,
      ::sus::fn::FnMutRef<bool(const ItemT&)>&& pred) noexcept {
    return Split(values, ::sus::move(pred));
  }

  constexpr Split(const Slice<ItemT>& values,
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
class [[nodiscard]] [[sus_trivial_abi]] SplitMut final
    : public ::sus::iter::IteratorBase<SplitMut<ItemT>,
                                       ::sus::containers::SliceMut<ItemT>> {
 public:
  // `Item` is a `SliceMut<T>`.
  using Item = ::sus::containers::SliceMut<ItemT>;

  SplitMut(SplitMut&&) = default;
  SplitMut& operator=(SplitMut&&) = default;

  /// sus::mem::Clone trait.
  SplitMut clone() const noexcept {
    return SplitMut(::sus::clone(v_), ::sus::clone(pred_));
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
        ret = Option<Item>::with(v_[::sus::ops::RangeTo(idx)]);
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
        ret = Option<Item>::with(v_[::sus::ops::RangeFrom(idx + 1u)]);
        v_ = v_[::sus::ops::RangeTo(idx)];
        return ret;
      }
    }
  }

  // Replace the default impl in sus::iter::IteratorBase.
  ::sus::iter::SizeHint size_hint() const noexcept {
    if (v_.is_empty()) {
      return {0u, ::sus::Option<::sus::num::usize>::with(0u)};
    } else {
      // If the predicate doesn't match anything, we yield one slice. If it
      // matches every element, we yield `len()` one-element slices, or a single
      // empty slice.
      return {1u, ::sus::Option<::sus::num::usize>::with(
                      ::sus::ops::max(1_usize, v_.len()))};
    }
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by SliceMut.
  friend class SliceMut<ItemT>;
  // Access to finish().
  template <class A, ::sus::iter::Iterator<A> B>
  friend class __private::GenericSplitN;
  // Access to finish().
  template <class A>
  friend class RSplitMut;

  Option<Item> finish() noexcept {
    if (finished_) {
      return Option<Item>();
    } else {
      finished_ = true;
      return Option<Item>::with(v_);
    }
  }

  static constexpr auto with(
      const SliceMut<ItemT>& values,
      ::sus::fn::FnMutRef<bool(const ItemT&)>&& pred) noexcept {
    return SplitMut(values, ::sus::move(pred));
  }

  constexpr SplitMut(const SliceMut<ItemT>& values,
                     ::sus::fn::FnMutRef<bool(const ItemT&)>&& pred) noexcept
      : v_(values), pred_(::sus::move(pred)) {}

  SliceMut<ItemT> v_;
  ::sus::fn::FnMutRef<bool(const ItemT&)> pred_;
  bool finished_ = false;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(v_),
                                  decltype(pred_), decltype(finished_));
};

/// An iterator over subslices separated by elements that match a predicate
/// function. Unlike `Split`, it contains the matched part as a terminator
/// of the subslice.
///
/// This struct is created by the `split_inclusive()` method on slices.
template <class ItemT>
class [[nodiscard]] [[sus_trivial_abi]] SplitInclusive final
    : public ::sus::iter::IteratorBase<SplitInclusive<ItemT>,
                                       ::sus::containers::Slice<ItemT>> {
 public:
  // `Item` is a `Slice<T>`.
  using Item = ::sus::containers::Slice<ItemT>;

  SplitInclusive(SplitInclusive&&) = default;
  SplitInclusive& operator=(SplitInclusive&&) = default;

  /// sus::mem::Clone trait.
  SplitInclusive clone() const noexcept {
    return SplitInclusive(::sus::clone(v_), ::sus::clone(pred_));
  }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    Option<Item> ret;

    if (finished_) [[unlikely]] {
      return ret;
    }

    // TODO: Use v_.iter().position().
    ::sus::num::usize idx;
    const usize last = v_.len() - 1u;
    auto it = v_.iter();
    while (true) {
      Option<const ItemT&> o = it.next();
      if (o.is_none() || pred_(*o)) {
        if (idx >= last) {
          finished_ = true;
          idx = last;
        }
        ret = Option<Item>::with(v_[::sus::ops::RangeTo(idx + 1u)]);
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

    // The last index of `v_` is already checked and found to match
    // by the last iteration, so we start searching a new match
    // one index to the left.
    const auto remainder =
        v_.is_empty() ? Slice<ItemT>() : v_[::sus::ops::RangeTo(v_.len() - 1u)];
    // TODO: Use v_.iter().rposition().
    ::sus::num::usize idx = remainder.len();
    auto it = remainder.iter().rev();
    while (true) {
      Option<const ItemT&> o = it.next();
      if (o.is_none() || pred_(*o)) {
        if (idx == 0u) {
          finished_ = true;
        }
        ret = Option<Item>::with(v_[::sus::ops::RangeFrom(idx)]);
        v_ = v_[::sus::ops::RangeTo(idx)];
        return ret;
      }
      idx -= 1u;
    }
  }

  // Replace the default impl in sus::iter::IteratorBase.
  ::sus::iter::SizeHint size_hint() const noexcept {
    if (v_.is_empty()) {
      return {0u, ::sus::Option<::sus::num::usize>::with(0u)};
    } else {
      // If the predicate doesn't match anything, we yield one slice. If it
      // matches every element, we yield `len()` one-element slices, or a single
      // empty slice.
      return {1u, ::sus::Option<::sus::num::usize>::with(
                      ::sus::ops::max(1_usize, v_.len()))};
    }
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by Slice.
  friend class Slice<ItemT>;

  static constexpr auto with(
      const Slice<ItemT>& values,
      ::sus::fn::FnMutRef<bool(const ItemT&)>&& pred) noexcept {
    return SplitInclusive(values, ::sus::move(pred));
  }

  constexpr SplitInclusive(
      const Slice<ItemT>& values,
      ::sus::fn::FnMutRef<bool(const ItemT&)>&& pred) noexcept
      : v_(values), pred_(::sus::move(pred)), finished_(v_.is_empty()) {}

  Slice<ItemT> v_;
  ::sus::fn::FnMutRef<bool(const ItemT&)> pred_;
  bool finished_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(v_),
                                  decltype(pred_), decltype(finished_));
};

/// An iterator over subslices separated by elements that match a predicate
/// function. Unlike `Split`, it contains the matched part as a terminator
/// of the subslice.
///
/// This struct is created by the `split_inclusive_mut()` method on slices.
template <class ItemT>
class [[nodiscard]] [[sus_trivial_abi]] SplitInclusiveMut final
    : public ::sus::iter::IteratorBase<SplitInclusiveMut<ItemT>,
                                       ::sus::containers::SliceMut<ItemT>> {
 public:
  // `Item` is a `SliceMut<T>`.
  using Item = ::sus::containers::SliceMut<ItemT>;

  SplitInclusiveMut(SplitInclusiveMut&&) = default;
  SplitInclusiveMut& operator=(SplitInclusiveMut&&) = default;

  /// sus::mem::Clone trait.
  SplitInclusiveMut clone() const noexcept {
    return SplitInclusiveMut(::sus::clone(v_), ::sus::clone(pred_));
  }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept {
    Option<Item> ret;

    if (finished_) [[unlikely]] {
      return ret;
    }

    // TODO: Use v_.iter().position().
    ::sus::num::usize idx;
    const usize last = v_.len() - 1u;
    auto it = v_.iter();
    while (true) {
      Option<const ItemT&> o = it.next();
      if (o.is_none() || pred_(*o)) {
        if (idx >= last) {
          finished_ = true;
          idx = last;
        }
        ret = Option<Item>::with(v_[::sus::ops::RangeTo(idx + 1u)]);
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

    // The last index of `v_` is already checked and found to match
    // by the last iteration, so we start searching a new match
    // one index to the left.
    const auto remainder = v_.is_empty()
                               ? SliceMut<ItemT>()
                               : v_[::sus::ops::RangeTo(v_.len() - 1u)];
    // TODO: Use v_.iter().rposition().
    ::sus::num::usize idx = remainder.len();
    auto it = remainder.iter().rev();
    while (true) {
      Option<const ItemT&> o = it.next();
      if (o.is_none() || pred_(*o)) {
        if (idx == 0u) {
          finished_ = true;
        }
        ret = Option<Item>::with(v_[::sus::ops::RangeFrom(idx)]);
        v_ = v_[::sus::ops::RangeTo(idx)];
        return ret;
      }
      idx -= 1u;
    }
  }

  // Replace the default impl in sus::iter::IteratorBase.
  ::sus::iter::SizeHint size_hint() const noexcept {
    if (v_.is_empty()) {
      return {0u, ::sus::Option<::sus::num::usize>::with(0u)};
    } else {
      // If the predicate doesn't match anything, we yield one slice. If it
      // matches every element, we yield `len()` one-element slices, or a single
      // empty slice.
      return {1u, ::sus::Option<::sus::num::usize>::with(
                      ::sus::ops::max(1_usize, v_.len()))};
    }
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by SliceMut.
  friend class SliceMut<ItemT>;

  static constexpr auto with(
      const SliceMut<ItemT>& values,
      ::sus::fn::FnMutRef<bool(const ItemT&)>&& pred) noexcept {
    return SplitInclusiveMut(values, ::sus::move(pred));
  }

  constexpr SplitInclusiveMut(
      const SliceMut<ItemT>& values,
      ::sus::fn::FnMutRef<bool(const ItemT&)>&& pred) noexcept
      : v_(values), pred_(::sus::move(pred)), finished_(v_.is_empty()) {}

  SliceMut<ItemT> v_;
  ::sus::fn::FnMutRef<bool(const ItemT&)> pred_;
  bool finished_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(v_),
                                  decltype(pred_), decltype(finished_));
};

/// An iterator over subslices separated by elements that match a predicate
/// function, starting from the end of the slice.
///
/// This struct is created by the `rsplit()` method on slices.
template <class ItemT>
class [[nodiscard]] [[sus_trivial_abi]] RSplit final
    : public ::sus::iter::IteratorBase<RSplit<ItemT>,
                                       ::sus::containers::Slice<ItemT>> {
 public:
  // `Item` is a `Slice<T>`.
  using Item = typename Split<ItemT>::Item;

  RSplit(RSplit&&) = default;
  RSplit& operator=(RSplit&&) = default;

  /// sus::mem::Clone trait.
  RSplit clone() const noexcept { return RSplit(::sus::clone(inner_)); }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept { return inner_.next_back(); }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept { return inner_.next(); }

  // Replace the default impl in sus::iter::IteratorBase.
  ::sus::iter::SizeHint size_hint() const noexcept {
    return inner_.size_hint();
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by Slice.
  friend class Slice<ItemT>;
  // Access to finish().
  template <class A, ::sus::iter::Iterator<A> B>
  friend class __private::GenericSplitN;

  Option<Item> finish() noexcept { return inner_.finish(); }

  static constexpr auto with(Split<ItemT> && split) noexcept {
    return RSplit(::sus::move(split));
  }

  constexpr RSplit(Split<ItemT> && split) noexcept
      : inner_(::sus::move(split)) {}

  Split<ItemT> inner_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(inner_));
};

/// An iterator over the subslices of the vector which are separated by elements
/// that match pred, starting from the end of the slice.
///
/// This struct is created by the `rsplit_mut()` method on slices.
template <class ItemT>
class [[nodiscard]] [[sus_trivial_abi]] RSplitMut final
    : public ::sus::iter::IteratorBase<RSplitMut<ItemT>,
                                       ::sus::containers::SliceMut<ItemT>> {
 public:
  // `Item` is a `SliceMut<T>`.
  using Item = typename SplitMut<ItemT>::Item;

  RSplitMut(RSplitMut&&) = default;
  RSplitMut& operator=(RSplitMut&&) = default;

  /// sus::mem::Clone trait.
  RSplitMut clone() const noexcept { return RSplitMut(::sus::clone(inner_)); }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept { return inner_.next_back(); }

  // sus::iter::DoubleEndedIterator trait.
  Option<Item> next_back() noexcept { return inner_.next(); }

  // Replace the default impl in sus::iter::IteratorBase.
  ::sus::iter::SizeHint size_hint() const noexcept {
    return inner_.size_hint();
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by SliceMut.
  friend class SliceMut<ItemT>;
  // Access to finish().
  template <class A, ::sus::iter::Iterator<A> B>
  friend class __private::GenericSplitN;

  Option<Item> finish() noexcept { return inner_.finish(); }

  static constexpr auto with(SplitMut<ItemT> && split) noexcept {
    return RSplitMut(::sus::move(split));
  }

  constexpr RSplitMut(SplitMut<ItemT> && split) noexcept
      : inner_(::sus::move(split)) {}

  SplitMut<ItemT> inner_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(inner_));
};

/// An iterator over subslices separated by elements that match a predicate
/// function, limited to a given number of splits.
///
/// This struct is created by the `splitn()` method on slices.
template <class ItemT>
class [[nodiscard]] [[sus_trivial_abi]] SplitN final
    : public ::sus::iter::IteratorBase<SplitN<ItemT>,
                                       ::sus::containers::Slice<ItemT>> {
 public:
  // `Item` is a `Slice<T>`.
  using Item = typename Split<ItemT>::Item;

  SplitN(SplitN&&) = default;
  SplitN& operator=(SplitN&&) = default;

  /// sus::mem::Clone trait.
  SplitN clone() const noexcept { return SplitN(::sus::clone(inner_)); }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept { return inner_.next(); }

  // Replace the default impl in sus::iter::IteratorBase.
  ::sus::iter::SizeHint size_hint() const noexcept {
    return inner_.size_hint();
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by Slice.
  friend class Slice<ItemT>;

  static constexpr auto with(Split<ItemT> && split, usize n) noexcept {
    return SplitN(::sus::move(split), n);
  }

  constexpr SplitN(Split<ItemT> split, usize n) noexcept
      : inner_(::sus::move(split), n) {}

  __private::GenericSplitN<Slice<ItemT>, Split<ItemT>> inner_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(inner_));
};

/// An iterator over mutable subslices separated by elements that match a
/// predicate function, limited to a given number of splits.
///
/// This struct is created by the `splitn_mut()` method on slices.
template <class ItemT>
class [[nodiscard]] [[sus_trivial_abi]] SplitNMut final
    : public ::sus::iter::IteratorBase<SplitNMut<ItemT>,
                                       ::sus::containers::SliceMut<ItemT>> {
 public:
  // `Item` is a `SliceMut<T>`.
  using Item = typename SplitMut<ItemT>::Item;

  SplitNMut(SplitNMut&&) = default;
  SplitNMut& operator=(SplitNMut&&) = default;

  /// sus::mem::Clone trait.
  SplitNMut clone() const noexcept { return SplitNMut(::sus::clone(inner_)); }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept { return inner_.next(); }

  // Replace the default impl in sus::iter::IteratorBase.
  ::sus::iter::SizeHint size_hint() const noexcept {
    return inner_.size_hint();
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by SliceMut.
  friend class SliceMut<ItemT>;

  static constexpr auto with(SplitMut<ItemT> && split, usize n) noexcept {
    return SplitNMut(::sus::move(split), n);
  }

  constexpr SplitNMut(SplitMut<ItemT> split, usize n) noexcept
      : inner_(::sus::move(split), n) {}

  __private::GenericSplitN<SliceMut<ItemT>, SplitMut<ItemT>> inner_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(inner_));
};

/// An iterator over subslices separated by elements that match a predicate
/// function, limited to a given number of splits, starting from the end of the
/// slice.
///
/// This struct is created by the `rsplitn()` method on slices.
template <class ItemT>
class [[nodiscard]] [[sus_trivial_abi]] RSplitN final
    : public ::sus::iter::IteratorBase<RSplitN<ItemT>,
                                       ::sus::containers::Slice<ItemT>> {
 public:
  // `Item` is a `Slice<T>`.
  using Item = typename RSplit<ItemT>::Item;

  RSplitN(RSplitN&&) = default;
  RSplitN& operator=(RSplitN&&) = default;

  /// sus::mem::Clone trait.
  RSplitN clone() const noexcept { return RSplitN(::sus::clone(inner_)); }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept { return inner_.next(); }

  // Replace the default impl in sus::iter::IteratorBase.
  ::sus::iter::SizeHint size_hint() const noexcept {
    return inner_.size_hint();
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by Slice.
  friend class Slice<ItemT>;

  static constexpr auto with(RSplit<ItemT> && split, usize n) noexcept {
    return RSplitN(::sus::move(split), n);
  }

  constexpr RSplitN(RSplit<ItemT> split, usize n) noexcept
      : inner_(::sus::move(split), n) {}

  __private::GenericSplitN<Slice<ItemT>, RSplit<ItemT>> inner_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(inner_));
};

/// An iterator over subslices separated by elements that match a predicate
/// function, limited to a given number of splits, starting from the end of the
/// slice.
///
/// This struct is created by the rsplitn_mut method on slices.
template <class ItemT>
class [[nodiscard]] [[sus_trivial_abi]] RSplitNMut final
    : public ::sus::iter::IteratorBase<RSplitNMut<ItemT>,
                                       ::sus::containers::SliceMut<ItemT>> {
 public:
  // `Item` is a `SliceMut<T>`.
  using Item = typename RSplitMut<ItemT>::Item;

  RSplitNMut(RSplitNMut&&) = default;
  RSplitNMut& operator=(RSplitNMut&&) = default;

  /// sus::mem::Clone trait.
  RSplitNMut clone() const noexcept { return RSplitNMut(::sus::clone(inner_)); }

  // sus::iter::Iterator trait.
  Option<Item> next() noexcept { return inner_.next(); }

  // Replace the default impl in sus::iter::IteratorBase.
  ::sus::iter::SizeHint size_hint() const noexcept {
    return inner_.size_hint();
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by SliceMut.
  friend class SliceMut<ItemT>;

  static constexpr auto with(RSplitMut<ItemT> && split, usize n) noexcept {
    return RSplitNMut(::sus::move(split), n);
  }

  constexpr RSplitNMut(RSplitMut<ItemT> split, usize n) noexcept
      : inner_(::sus::move(split), n) {}

  __private::GenericSplitN<SliceMut<ItemT>, RSplitMut<ItemT>> inner_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(inner_));
};

}  // namespace sus::containers
