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
class GenericSplitN final
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
    return {::sus::cmp::min(count, lower),
            Option<usize>(
                ::sus::move(upper_opt).map_or(count, [count](usize upper) {
                  return ::sus::cmp::min(count, upper);
                }))};
  }

 private:
  I iter_;
  usize count_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(iter_), decltype(count_));
};

}  // namespace __private

/// An iterator over subslices separated by elements that match a predicate
/// function.
///
/// This struct is created by the `split()` method on slices.
template <class ItemT, ::sus::fn::FnMut<bool(const ItemT&)> Pred>
class [[nodiscard]] Split final
    : public ::sus::iter::IteratorBase<Split<ItemT, Pred>,
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
                      ::sus::cmp::max(1_usize, v_.len()))};
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
  template <class A, ::sus::fn::FnMut<bool(const A&)> APred>
  friend class RSplit;

  constexpr Option<Item> finish() noexcept {
    if (finished_) {
      return Option<Item>();
    } else {
      finished_ = true;
      return Option<Item>(v_);
    }
  }

  constexpr Split(::sus::iter::IterRef ref, const Slice<ItemT>& values,
                  Pred&& pred) noexcept
      : ref_(::sus::move(ref)), v_(values), pred_(::sus::move(pred)) {}

  [[sus_no_unique_address]] ::sus::iter::IterRef ref_;
  Slice<ItemT> v_;
  Pred pred_;
  bool finished_ = false;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(ref_), decltype(v_),
                                           decltype(pred_),
                                           decltype(finished_));
};

/// An iterator over subslices separated by elements that match a predicate
/// function.
///
/// This struct is created by the `split_mut()` method on slices.
template <class ItemT, ::sus::fn::FnMut<bool(const ItemT&)> Pred>
class [[nodiscard]] SplitMut final
    : public ::sus::iter::IteratorBase<SplitMut<ItemT, Pred>,
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
                      ::sus::cmp::max(1_usize, v_.len()))};
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
  template <class A, ::sus::fn::FnMut<bool(const A&)>>
  friend class RSplitMut;

  constexpr Option<Item> finish() noexcept {
    if (finished_) {
      return Option<Item>();
    } else {
      finished_ = true;
      return Option<Item>(v_);
    }
  }

  constexpr SplitMut(::sus::iter::IterRef ref, const SliceMut<ItemT>& values,
                     Pred&& pred) noexcept
      : ref_(::sus::move(ref)), v_(values), pred_(::sus::move(pred)) {}

  [[sus_no_unique_address]] ::sus::iter::IterRef ref_;
  SliceMut<ItemT> v_;
  Pred pred_;
  bool finished_ = false;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(ref_), decltype(v_),
                                           decltype(pred_),
                                           decltype(finished_));
};

/// An iterator over subslices separated by elements that match a predicate
/// function. Unlike `Split`, it contains the matched part as a terminator
/// of the subslice.
///
/// This struct is created by the `split_inclusive()` method on slices.
template <class ItemT, ::sus::fn::FnMut<bool(const ItemT&)> Pred>
class [[nodiscard]] SplitInclusive final
    : public ::sus::iter::IteratorBase<SplitInclusive<ItemT, Pred>,
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
                      ::sus::cmp::max(1_usize, v_.len()))};
    }
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by Slice, Vec, Array.
  friend class Slice<ItemT>;
  friend class Vec<ItemT>;
  template <class ArrayItemT, size_t N>
  friend class Array;

  constexpr SplitInclusive(::sus::iter::IterRef ref, const Slice<ItemT>& values,
                           Pred&& pred) noexcept
      : ref_(::sus::move(ref)),
        v_(values),
        pred_(::sus::move(pred)),
        finished_(v_.is_empty()) {}

  [[sus_no_unique_address]] ::sus::iter::IterRef ref_;
  Slice<ItemT> v_;
  Pred pred_;
  bool finished_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(ref_), decltype(v_),
                                           decltype(pred_),
                                           decltype(finished_));
};

/// An iterator over subslices separated by elements that match a predicate
/// function. Unlike `Split`, it contains the matched part as a terminator
/// of the subslice.
///
/// This struct is created by the `split_inclusive_mut()` method on slices.
template <class ItemT, ::sus::fn::FnMut<bool(const ItemT&)> Pred>
class [[nodiscard]] SplitInclusiveMut final
    : public ::sus::iter::IteratorBase<SplitInclusiveMut<ItemT, Pred>,
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
                      ::sus::cmp::max(1_usize, v_.len()))};
    }
  }

  // TODO: Impl count(), nth(), last(), nth_back().

 private:
  // Constructed by SliceMut, Vec, Array.
  friend class SliceMut<ItemT>;
  friend class Vec<ItemT>;
  template <class ArrayItemT, size_t N>
  friend class Array;

  constexpr SplitInclusiveMut(::sus::iter::IterRef ref,
                              const SliceMut<ItemT>& values,
                              Pred&& pred) noexcept
      : ref_(::sus::move(ref)),
        v_(values),
        pred_(::sus::move(pred)),
        finished_(v_.is_empty()) {}

  [[sus_no_unique_address]] ::sus::iter::IterRef ref_;
  SliceMut<ItemT> v_;
  Pred pred_;
  bool finished_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(ref_), decltype(v_),
                                           decltype(pred_),
                                           decltype(finished_));
};

/// An iterator over subslices separated by elements that match a predicate
/// function, starting from the end of the slice.
///
/// This struct is created by the `rsplit()` method on slices.
template <class ItemT, ::sus::fn::FnMut<bool(const ItemT&)> Pred>
class [[nodiscard]] RSplit final
    : public ::sus::iter::IteratorBase<RSplit<ItemT, Pred>,
                                       ::sus::collections::Slice<ItemT>> {
 public:
  using Item = ::sus::collections::Slice<ItemT>;

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

  constexpr RSplit(Split<ItemT, Pred>&& split) noexcept
      : inner_(::sus::move(split)) {}

  Split<ItemT, Pred> inner_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(inner_));
};

/// An iterator over the subslices of the vector which are separated by elements
/// that match pred, starting from the end of the slice.
///
/// This struct is created by the `rsplit_mut()` method on slices.
template <class ItemT, ::sus::fn::FnMut<bool(const ItemT&)> Pred>
class [[nodiscard]] RSplitMut final
    : public ::sus::iter::IteratorBase<RSplitMut<ItemT, Pred>,
                                       ::sus::collections::SliceMut<ItemT>> {
 public:
  using Item = ::sus::collections::SliceMut<ItemT>;

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

  constexpr RSplitMut(SplitMut<ItemT, Pred>&& split) noexcept
      : inner_(::sus::move(split)) {}

  SplitMut<ItemT, Pred> inner_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(inner_));
};

/// An iterator over subslices separated by elements that match a predicate
/// function, limited to a given number of splits.
///
/// This struct is created by the `splitn()` method on slices.
template <class ItemT, ::sus::fn::FnMut<bool(const ItemT&)> Pred>
class [[nodiscard]] SplitN final
    : public ::sus::iter::IteratorBase<SplitN<ItemT, Pred>,
                                       ::sus::collections::Slice<ItemT>> {
 public:
  using Item = ::sus::collections::Slice<ItemT>;

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

  constexpr SplitN(Split<ItemT, Pred> split, usize n) noexcept
      : inner_(::sus::move(split), n) {}

  __private::GenericSplitN<Slice<ItemT>, Split<ItemT, Pred>> inner_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(inner_));
};

/// An iterator over mutable subslices separated by elements that match a
/// predicate function, limited to a given number of splits.
///
/// This struct is created by the `splitn_mut()` method on slices.
template <class ItemT, ::sus::fn::FnMut<bool(const ItemT&)> Pred>
class [[nodiscard]] SplitNMut final
    : public ::sus::iter::IteratorBase<SplitNMut<ItemT, Pred>,
                                       ::sus::collections::SliceMut<ItemT>> {
 public:
  using Item = ::sus::collections::SliceMut<ItemT>;

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

  constexpr SplitNMut(SplitMut<ItemT, Pred> split, usize n) noexcept
      : inner_(::sus::move(split), n) {}

  __private::GenericSplitN<SliceMut<ItemT>, SplitMut<ItemT, Pred>> inner_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(inner_));
};

/// An iterator over subslices separated by elements that match a predicate
/// function, limited to a given number of splits, starting from the end of the
/// slice.
///
/// This struct is created by the `rsplitn()` method on slices.
template <class ItemT, ::sus::fn::FnMut<bool(const ItemT&)> Pred>
class [[nodiscard]] RSplitN final
    : public ::sus::iter::IteratorBase<RSplitN<ItemT, Pred>,
                                       ::sus::collections::Slice<ItemT>> {
 public:
  using Item = ::sus::collections::Slice<ItemT>;

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

  constexpr RSplitN(RSplit<ItemT, Pred> split, usize n) noexcept
      : inner_(::sus::move(split), n) {}

  __private::GenericSplitN<Slice<ItemT>, RSplit<ItemT, Pred>> inner_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(inner_));
};

/// An iterator over subslices separated by elements that match a predicate
/// function, limited to a given number of splits, starting from the end of the
/// slice.
///
/// This struct is created by the rsplitn_mut method on slices.
template <class ItemT, ::sus::fn::FnMut<bool(const ItemT&)> Pred>
class [[nodiscard]] RSplitNMut final
    : public ::sus::iter::IteratorBase<RSplitNMut<ItemT, Pred>,
                                       ::sus::collections::SliceMut<ItemT>> {
 public:
  // `Item` is a `SliceMut<T>`.
  using Item = ::sus::collections::SliceMut<ItemT>;

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

  constexpr RSplitNMut(RSplitMut<ItemT, Pred> split, usize n) noexcept
      : inner_(::sus::move(split), n) {}

  __private::GenericSplitN<SliceMut<ItemT>, RSplitMut<ItemT, Pred>> inner_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(inner_));
};

}  // namespace sus::collections
