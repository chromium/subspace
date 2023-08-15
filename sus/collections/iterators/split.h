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

#include "sus/iter/iterator_defn.h"
#include "sus/iter/iterator_ref.h"
#include "sus/lib/__private/forward_decl.h"
#include "sus/macros/no_unique_address.h"
#include "sus/mem/replace.h"
#include "sus/num/unsigned_integer.h"
#include "sus/ops/range.h"

namespace sus::collections {

namespace __private {

/// An private iterator over subslices separated by elements that
/// match a predicate function, splitting at most a fixed number of
/// times.
template <class ItemT, ::sus::iter::Iterator<ItemT> I>
class [[sus_trivial_abi]] GenericSplitN final
    : public ::sus::iter::IteratorBase<GenericSplitN<ItemT, I>, ItemT> {
 public:
  using Item = ItemT;

  constexpr explicit GenericSplitN(I&& iter, usize count)
      : iter_(::sus::move(iter)), count_(count) {}

  constexpr Option<Item> next() noexcept {
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

  constexpr ::sus::iter::SizeHint size_hint() const noexcept {
    auto [lower, upper_opt] = iter_.size_hint();
    const auto count = count_;
    return {::sus::ops::min(count, lower),
            Option<usize>(
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
                                       ::sus::collections::Slice<ItemT>> {
 public:
  // `Item` is a `Slice<T>`.
  using Item = ::sus::collections::Slice<ItemT>;

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept {
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
      if (::sus::fn::call_mut(pred_, *o)) {
        ret = Option<Item>(v_[::sus::ops::RangeTo(idx)]);
        v_ = v_[::sus::ops::RangeFrom(idx + 1u)];
        return ret;
      }
      idx += 1u;
    }
  }

  // sus::iter::DoubleEndedIterator trait.
  constexpr Option<Item> next_back() noexcept {
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
      if (::sus::fn::call_mut(pred_, *o)) {
        ret = Option<Item>(v_[::sus::ops::RangeFrom(idx + 1u)]);
        v_ = v_[::sus::ops::RangeTo(idx)];
        return ret;
      }
    }
  }

  // Replace the default impl in sus::iter::IteratorBase.
  constexpr ::sus::iter::SizeHint size_hint() const noexcept {
    if (v_.is_empty()) {
      return {0u, ::sus::Option<::sus::num::usize>(0u)};
    } else {
      // If the predicate doesn't match anything, we yield one slice. If it
      // matches every element, we yield `len()` one-element slices, or a single
      // empty slice.
      return {1u, ::sus::Option<::sus::num::usize>(
                      ::sus::ops::max(1_usize, v_.len()))};
    }
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by Slice, Vec, Array.
  friend class Slice<ItemT>;
  friend class Vec<ItemT>;
  template <class ArrayItemT, size_t N>
  friend class Array;

  // Access to finish().
  template <class A, ::sus::iter::Iterator<A> B>
  friend class __private::GenericSplitN;
  // Access to finish().
  template <class A>
  friend class RSplit;

  constexpr Option<Item> finish() noexcept {
    if (finished_) {
      return Option<Item>();
    } else {
      finished_ = true;
      return Option<Item>(v_);
    }
  }

  static constexpr auto with(
      ::sus::iter::IterRef ref, const Slice<ItemT>& values,
      ::sus::fn::FnMutRef<bool(const ItemT&)>&& pred) noexcept {
    return Split(::sus::move(ref), values, ::sus::move(pred));
  }

  constexpr Split(::sus::iter::IterRef ref, const Slice<ItemT>& values,
                  ::sus::fn::FnMutRef<bool(const ItemT&)>&& pred) noexcept
      : ref_(::sus::move(ref)), v_(values), pred_(::sus::move(pred)) {}

  [[sus_no_unique_address]] ::sus::iter::IterRef ref_;
  Slice<ItemT> v_;
  ::sus::fn::FnMutRef<bool(const ItemT&)> pred_;
  bool finished_ = false;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(ref_),
                                  decltype(v_), decltype(pred_),
                                  decltype(finished_));
};

/// An iterator over subslices separated by elements that match a predicate
/// function.
///
/// This struct is created by the `split_mut()` method on slices.
template <class ItemT>
class [[nodiscard]] [[sus_trivial_abi]] SplitMut final
    : public ::sus::iter::IteratorBase<SplitMut<ItemT>,
                                       ::sus::collections::SliceMut<ItemT>> {
 public:
  // `Item` is a `SliceMut<T>`.
  using Item = ::sus::collections::SliceMut<ItemT>;

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept {
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
      if (::sus::fn::call_mut(pred_, *o)) {
        ret = Option<Item>(v_[::sus::ops::RangeTo(idx)]);
        v_ = v_[::sus::ops::RangeFrom(idx + 1u)];
        return ret;
      }
      idx += 1u;
    }
  }

  // sus::iter::DoubleEndedIterator trait.
  constexpr Option<Item> next_back() noexcept {
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
      if (::sus::fn::call_mut(pred_, *o)) {
        ret = Option<Item>(v_[::sus::ops::RangeFrom(idx + 1u)]);
        v_ = v_[::sus::ops::RangeTo(idx)];
        return ret;
      }
    }
  }

  // Replace the default impl in sus::iter::IteratorBase.
  constexpr ::sus::iter::SizeHint size_hint() const noexcept {
    if (v_.is_empty()) {
      return {0u, ::sus::Option<::sus::num::usize>(0u)};
    } else {
      // If the predicate doesn't match anything, we yield one slice. If it
      // matches every element, we yield `len()` one-element slices, or a single
      // empty slice.
      return {1u, ::sus::Option<::sus::num::usize>(
                      ::sus::ops::max(1_usize, v_.len()))};
    }
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by SliceMut, Vec, Array.
  friend class SliceMut<ItemT>;
  friend class Vec<ItemT>;
  template <class ArrayItemT, size_t N>
  friend class Array;

  // Access to finish().
  template <class A, ::sus::iter::Iterator<A> B>
  friend class __private::GenericSplitN;
  // Access to finish().
  template <class A>
  friend class RSplitMut;

  constexpr Option<Item> finish() noexcept {
    if (finished_) {
      return Option<Item>();
    } else {
      finished_ = true;
      return Option<Item>(v_);
    }
  }

  static constexpr auto with(
      ::sus::iter::IterRef ref, const SliceMut<ItemT>& values,
      ::sus::fn::FnMutRef<bool(const ItemT&)>&& pred) noexcept {
    return SplitMut(::sus::move(ref), values, ::sus::move(pred));
  }

  constexpr SplitMut(::sus::iter::IterRef ref, const SliceMut<ItemT>& values,
                     ::sus::fn::FnMutRef<bool(const ItemT&)>&& pred) noexcept
      : ref_(::sus::move(ref)), v_(values), pred_(::sus::move(pred)) {}

  [[sus_no_unique_address]] ::sus::iter::IterRef ref_;
  SliceMut<ItemT> v_;
  ::sus::fn::FnMutRef<bool(const ItemT&)> pred_;
  bool finished_ = false;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(ref_),
                                  decltype(v_), decltype(pred_),
                                  decltype(finished_));
};

/// An iterator over subslices separated by elements that match a predicate
/// function. Unlike `Split`, it contains the matched part as a terminator
/// of the subslice.
///
/// This struct is created by the `split_inclusive()` method on slices.
template <class ItemT>
class [[nodiscard]] [[sus_trivial_abi]] SplitInclusive final
    : public ::sus::iter::IteratorBase<SplitInclusive<ItemT>,
                                       ::sus::collections::Slice<ItemT>> {
 public:
  // `Item` is a `Slice<T>`.
  using Item = ::sus::collections::Slice<ItemT>;

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept {
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
      if (o.is_none() || ::sus::fn::call_mut(pred_, *o)) {
        if (idx >= last) {
          finished_ = true;
          idx = last;
        }
        ret = Option<Item>(v_[::sus::ops::RangeTo(idx + 1u)]);
        v_ = v_[::sus::ops::RangeFrom(idx + 1u)];
        return ret;
      }
      idx += 1u;
    }
  }

  // sus::iter::DoubleEndedIterator trait.
  constexpr Option<Item> next_back() noexcept {
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
      if (o.is_none() || ::sus::fn::call_mut(pred_, *o)) {
        if (idx == 0u) {
          finished_ = true;
        }
        ret = Option<Item>(v_[::sus::ops::RangeFrom(idx)]);
        v_ = v_[::sus::ops::RangeTo(idx)];
        return ret;
      }
      idx -= 1u;
    }
  }

  // Replace the default impl in sus::iter::IteratorBase.
  constexpr ::sus::iter::SizeHint size_hint() const noexcept {
    if (v_.is_empty()) {
      return {0u, ::sus::Option<::sus::num::usize>(0u)};
    } else {
      // If the predicate doesn't match anything, we yield one slice. If it
      // matches every element, we yield `len()` one-element slices, or a single
      // empty slice.
      return {1u, ::sus::Option<::sus::num::usize>(
                      ::sus::ops::max(1_usize, v_.len()))};
    }
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by Slice, Vec, Array.
  friend class Slice<ItemT>;
  friend class Vec<ItemT>;
  template <class ArrayItemT, size_t N>
  friend class Array;

  static constexpr auto with(
      ::sus::iter::IterRef ref, const Slice<ItemT>& values,
      ::sus::fn::FnMutRef<bool(const ItemT&)>&& pred) noexcept {
    return SplitInclusive(::sus::move(ref), values, ::sus::move(pred));
  }

  constexpr SplitInclusive(
      ::sus::iter::IterRef ref, const Slice<ItemT>& values,
      ::sus::fn::FnMutRef<bool(const ItemT&)>&& pred) noexcept
      : ref_(::sus::move(ref)),
        v_(values),
        pred_(::sus::move(pred)),
        finished_(v_.is_empty()) {}

  [[sus_no_unique_address]] ::sus::iter::IterRef ref_;
  Slice<ItemT> v_;
  ::sus::fn::FnMutRef<bool(const ItemT&)> pred_;
  bool finished_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(ref_),
                                  decltype(v_), decltype(pred_),
                                  decltype(finished_));
};

/// An iterator over subslices separated by elements that match a predicate
/// function. Unlike `Split`, it contains the matched part as a terminator
/// of the subslice.
///
/// This struct is created by the `split_inclusive_mut()` method on slices.
template <class ItemT>
class [[nodiscard]] [[sus_trivial_abi]] SplitInclusiveMut final
    : public ::sus::iter::IteratorBase<SplitInclusiveMut<ItemT>,
                                       ::sus::collections::SliceMut<ItemT>> {
 public:
  // `Item` is a `SliceMut<T>`.
  using Item = ::sus::collections::SliceMut<ItemT>;

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept {
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
      if (o.is_none() || ::sus::fn::call_mut(pred_, *o)) {
        if (idx >= last) {
          finished_ = true;
          idx = last;
        }
        ret = Option<Item>(v_[::sus::ops::RangeTo(idx + 1u)]);
        v_ = v_[::sus::ops::RangeFrom(idx + 1u)];
        return ret;
      }
      idx += 1u;
    }
  }

  // sus::iter::DoubleEndedIterator trait.
  constexpr Option<Item> next_back() noexcept {
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
      if (o.is_none() || ::sus::fn::call_mut(pred_, *o)) {
        if (idx == 0u) {
          finished_ = true;
        }
        ret = Option<Item>(v_[::sus::ops::RangeFrom(idx)]);
        v_ = v_[::sus::ops::RangeTo(idx)];
        return ret;
      }
      idx -= 1u;
    }
  }

  // Replace the default impl in sus::iter::IteratorBase.
  constexpr ::sus::iter::SizeHint size_hint() const noexcept {
    if (v_.is_empty()) {
      return {0u, ::sus::Option<::sus::num::usize>(0u)};
    } else {
      // If the predicate doesn't match anything, we yield one slice. If it
      // matches every element, we yield `len()` one-element slices, or a single
      // empty slice.
      return {1u, ::sus::Option<::sus::num::usize>(
                      ::sus::ops::max(1_usize, v_.len()))};
    }
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by SliceMut, Vec, Array.
  friend class SliceMut<ItemT>;
  friend class Vec<ItemT>;
  template <class ArrayItemT, size_t N>
  friend class Array;

  static constexpr auto with(
      ::sus::iter::IterRef ref, const SliceMut<ItemT>& values,
      ::sus::fn::FnMutRef<bool(const ItemT&)>&& pred) noexcept {
    return SplitInclusiveMut(::sus::move(ref), values, ::sus::move(pred));
  }

  constexpr SplitInclusiveMut(
      ::sus::iter::IterRef ref, const SliceMut<ItemT>& values,
      ::sus::fn::FnMutRef<bool(const ItemT&)>&& pred) noexcept
      : ref_(::sus::move(ref)),
        v_(values),
        pred_(::sus::move(pred)),
        finished_(v_.is_empty()) {}

  [[sus_no_unique_address]] ::sus::iter::IterRef ref_;
  SliceMut<ItemT> v_;
  ::sus::fn::FnMutRef<bool(const ItemT&)> pred_;
  bool finished_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(ref_),
                                  decltype(v_), decltype(pred_),
                                  decltype(finished_));
};

/// An iterator over subslices separated by elements that match a predicate
/// function, starting from the end of the slice.
///
/// This struct is created by the `rsplit()` method on slices.
template <class ItemT>
class [[nodiscard]] [[sus_trivial_abi]] RSplit final
    : public ::sus::iter::IteratorBase<RSplit<ItemT>,
                                       ::sus::collections::Slice<ItemT>> {
 public:
  // `Item` is a `Slice<T>`.
  using Item = typename Split<ItemT>::Item;

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept { return inner_.next_back(); }

  // sus::iter::DoubleEndedIterator trait.
  constexpr Option<Item> next_back() noexcept { return inner_.next(); }

  // Replace the default impl in sus::iter::IteratorBase.
  constexpr ::sus::iter::SizeHint size_hint() const noexcept {
    return inner_.size_hint();
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by Slice, Vec, Array.
  friend class Slice<ItemT>;
  friend class Vec<ItemT>;
  template <class ArrayItemT, size_t N>
  friend class Array;

  // Access to finish().
  template <class A, ::sus::iter::Iterator<A> B>
  friend class __private::GenericSplitN;

  constexpr Option<Item> finish() noexcept { return inner_.finish(); }

  static constexpr auto with(Split<ItemT>&& split) noexcept {
    return RSplit(::sus::move(split));
  }

  constexpr RSplit(Split<ItemT>&& split) noexcept
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
                                       ::sus::collections::SliceMut<ItemT>> {
 public:
  // `Item` is a `SliceMut<T>`.
  using Item = typename SplitMut<ItemT>::Item;

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept { return inner_.next_back(); }

  // sus::iter::DoubleEndedIterator trait.
  constexpr Option<Item> next_back() noexcept { return inner_.next(); }

  // Replace the default impl in sus::iter::IteratorBase.
  constexpr ::sus::iter::SizeHint size_hint() const noexcept {
    return inner_.size_hint();
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by SliceMut, Vec, Array.
  friend class SliceMut<ItemT>;
  friend class Vec<ItemT>;
  template <class ArrayItemT, size_t N>
  friend class Array;

  // Access to finish().
  template <class A, ::sus::iter::Iterator<A> B>
  friend class __private::GenericSplitN;

  Option<Item> finish() noexcept { return inner_.finish(); }

  static constexpr auto with(SplitMut<ItemT>&& split) noexcept {
    return RSplitMut(::sus::move(split));
  }

  constexpr RSplitMut(SplitMut<ItemT>&& split) noexcept
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
                                       ::sus::collections::Slice<ItemT>> {
 public:
  // `Item` is a `Slice<T>`.
  using Item = typename Split<ItemT>::Item;

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept { return inner_.next(); }

  // Replace the default impl in sus::iter::IteratorBase.
  constexpr ::sus::iter::SizeHint size_hint() const noexcept {
    return inner_.size_hint();
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by Slice, Vec, Array.
  friend class Slice<ItemT>;
  friend class Vec<ItemT>;
  template <class ArrayItemT, size_t N>
  friend class Array;

  static constexpr auto with(Split<ItemT>&& split, usize n) noexcept {
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
                                       ::sus::collections::SliceMut<ItemT>> {
 public:
  // `Item` is a `SliceMut<T>`.
  using Item = typename SplitMut<ItemT>::Item;

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept { return inner_.next(); }

  // Replace the default impl in sus::iter::IteratorBase.
  constexpr ::sus::iter::SizeHint size_hint() const noexcept {
    return inner_.size_hint();
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by SliceMut, Vec, Array.
  friend class SliceMut<ItemT>;
  friend class Vec<ItemT>;
  template <class ArrayItemT, size_t N>
  friend class Array;

  static constexpr auto with(SplitMut<ItemT>&& split, usize n) noexcept {
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
                                       ::sus::collections::Slice<ItemT>> {
 public:
  // `Item` is a `Slice<T>`.
  using Item = typename RSplit<ItemT>::Item;

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept { return inner_.next(); }

  // Replace the default impl in sus::iter::IteratorBase.
  constexpr ::sus::iter::SizeHint size_hint() const noexcept {
    return inner_.size_hint();
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by Slice, Vec, Array.
  friend class Slice<ItemT>;
  friend class Vec<ItemT>;
  template <class ArrayItemT, size_t N>
  friend class Array;

  static constexpr auto with(RSplit<ItemT>&& split, usize n) noexcept {
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
                                       ::sus::collections::SliceMut<ItemT>> {
 public:
  // `Item` is a `SliceMut<T>`.
  using Item = typename RSplitMut<ItemT>::Item;

  // sus::iter::Iterator trait.
  constexpr Option<Item> next() noexcept { return inner_.next(); }

  // Replace the default impl in sus::iter::IteratorBase.
  constexpr ::sus::iter::SizeHint size_hint() const noexcept {
    return inner_.size_hint();
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by SliceMut, Vec, Array.
  friend class SliceMut<ItemT>;
  friend class Vec<ItemT>;
  template <class ArrayItemT, size_t N>
  friend class Array;

  static constexpr auto with(RSplitMut<ItemT>&& split, usize n) noexcept {
    return RSplitNMut(::sus::move(split), n);
  }

  constexpr RSplitNMut(RSplitMut<ItemT> split, usize n) noexcept
      : inner_(::sus::move(split), n) {}

  __private::GenericSplitN<SliceMut<ItemT>, RSplitMut<ItemT>> inner_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(inner_));
};

}  // namespace sus::collections
