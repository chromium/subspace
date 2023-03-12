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

#include <algorithm>  // Replace std::sort.
#include <concepts>
#include <type_traits>

#include "subspace/assertions/check.h"
#include "subspace/construct/into.h"
#include "subspace/containers/__private/slice_iter.h"
#include "subspace/fn/fn_concepts.h"
#include "subspace/fn/run_fn.h"
#include "subspace/iter/iterator_defn.h"
#include "subspace/macros/lifetimebound.h"
#include "subspace/marker/unsafe.h"
#include "subspace/mem/clone.h"
#include "subspace/num/unsigned_integer.h"
#include "subspace/ops/ord.h"
#include "subspace/ops/range.h"
#include "subspace/option/option.h"

// TODO: sort_by_key()
// TODO: sort_by_cached_key()
// TODO: sort_unstable_by_key()

namespace sus::containers {

template <class T>
class Vec;

/// A dynamically-sized view into a contiguous sequence, `[T]`.
///
/// Contiguous here means that elements are laid out so that every element is
/// the same distance from its neighbors.
///
/// Slices are a view into a block of memory represented as a pointer and a
/// length.
template <class T>
class [[sus_trivial_abi]] Slice {
 public:
  static_assert(!std::is_reference_v<T>,
                "Slice holds references, so the type parameter can not also be "
                "a reference");

  constexpr Slice() : Slice(nullptr, 0_usize) {}

  static constexpr inline Slice from_raw_parts(::sus::marker::UnsafeFnMarker,
                                               T* data sus_lifetimebound,
                                               ::sus::usize len) noexcept {
    ::sus::check(size_t{len} <= size_t{isize::MAX_PRIMITIVE});
    return Slice(data, len);
  }

  // sus::construct::From<Slice<T>, T[]> trait.
  template <size_t N>
    requires(N <= size_t{PTRDIFF_MAX})
  static constexpr inline Slice from(T (&data)[N] sus_lifetimebound) {
    return Slice(data, N);
  }

  /// Returns the number of elements in the slice.
  constexpr inline usize len() const& noexcept { return len_; }

  /// Returns true if the slice has a length of 0.
  constexpr inline bool is_empty() const& noexcept { return len_ == 0u; }

  /// Returns a reference to the element at position `i` in the Slice.
  ///
  /// # Panics
  /// If the index `i` is beyond the end of the slice, the function will panic.
  /// #[doc.overloads=slice.index.usize]
  constexpr inline const T& operator[](usize i) const& noexcept {
    check(i < len_);
    return data_[i.primitive_value];
  }

  constexpr T& operator[](usize i) & noexcept
    requires(!std::is_const_v<T>)
  {
    check(i < len_);
    return data_[i.primitive_value];
  }

  /// Returns a const reference to the element at index `i`, or `None` if
  /// `i` is beyond the end of the Slice.
  constexpr Option<const T&> get(usize i) const& noexcept {
    if (i < len_) [[likely]]
      return Option<const T&>::some(data_[i.primitive_value]);
    else
      return Option<const T&>::none();
  }

  /// Returns a mutable reference to the element at index `i`, or `None` if
  /// `i` is beyond the end of the Slice.
  constexpr Option<T&> get_mut(usize i) & noexcept
    requires(!std::is_const_v<T>)
  {
    if (i < len_) [[likely]]
      return Option<T&>::some(mref(data_[i.primitive_value]));
    else
      return Option<T&>::none();
  }

  /// Returns a const reference to the element at index `i`.
  ///
  /// # Safety
  /// The index `i` must be inside the bounds of the slice or Undefined
  /// Behaviour results. The size of the slice must therefore also have a length
  /// of at least 1.
  constexpr inline const T& get_unchecked(::sus::marker::UnsafeFnMarker,
                                          usize i) const& noexcept {
    return data_[i.primitive_value];
  }

  /// Returns a mutable reference to the element at index `i`.
  ///
  /// # Safety
  /// The index `i` must be inside the bounds of the slice or Undefined
  /// Behaviour results. The size of the slice must therefore also have a length
  /// of at least 1.
  constexpr inline T& get_unchecked_mut(::sus::marker::UnsafeFnMarker,
                                        usize i) & noexcept
    requires(!std::is_const_v<T>)
  {
    return data_[i.primitive_value];
  }

  /// Returns a subslice which contains elements in `range`, which specifies a
  /// start and a length.
  ///
  /// The start is the index of the first element to be returned in the
  /// subslice, and the length is the number of elements in the output Slice.
  /// As such, `r.get_range(Range(0u, r.len()))` simply returns a copy of
  /// `r`.
  ///
  /// # Panics
  /// If the Range would otherwise contain an element that is out of bounds, the
  /// function will panic.
  /// #[doc.overloads=slice.index.range]
  constexpr inline Slice<T> operator[](
      const ::sus::ops::RangeBounds<usize> auto range) const noexcept {
    const usize start = range.start_bound().unwrap_or(0u);
    const usize end = range.end_bound().unwrap_or(len_);
    const usize len = end >= start ? end - start : 0u;
    ::sus::check(len <= len_);  // Avoid underflow below.
    // We allow start == len_ && end == len_, which returns an empty slice.
    ::sus::check(start <= len_ && start <= len_ - len);
    return Slice(data_ + size_t{start}, len);
  }

  /// Returns a subslice which contains elements in `range`, which specifies a
  /// start and a length.
  ///
  /// The start is the index of the first element to be returned in the
  /// subslice, and the length is the number of elements in the output Slice.
  /// As such, `r.get_range(Range(0u, r.len()))` simply returns a copy of
  /// `r`.
  ///
  /// Returns None if the Range would otherwise contain an element that is out
  /// of bounds.
  constexpr Option<Slice<T>> get_range(
      const ::sus::ops::RangeBounds<usize> auto range) const noexcept {
    const usize start = range.start_bound().unwrap_or(0u);
    const usize end = range.end_bound().unwrap_or(len_);
    const usize len = end >= start ? end - start : 0u;
    if (len > len_) return sus::none();  // Avoid underflow below.
    // We allow start == len_ && end == len_, which returns an empty slice.
    if (start > len_ || start > len_ - len) return sus::none();
    return Option<Slice<T>>::some(Slice(data_ + size_t{start}, len));
  }

  /// Returns a subslice which contains elements in `range`, which specifies a
  /// start and a length.
  ///
  /// The start is the index of the first element to be returned in the
  /// subslice, and the length is the number of elements in the output Slice.
  /// As such, `r.get_range(Range(0u, r.len()))` simply returns a copy of
  /// `r`.
  ///
  /// # Safety
  /// It is possible to specify a Range contains an element that is out
  /// of bounds of the Slice, which can result in Undefined Behaviour.
  constexpr Slice<T> get_range_unchecked(
      ::sus::marker::UnsafeFnMarker,
      const ::sus::ops::RangeBounds<usize> auto range) const noexcept {
    const usize start = range.start_bound().unwrap_or(0u);
    const usize end = range.end_bound().unwrap_or(len_);
    const usize len = end >= start ? end - start : 0u;
    return Slice(data_ + size_t{start}, len);
  }

  /// Sorts the slice.
  ///
  /// This sort is stable (i.e., does not reorder equal elements) and O(n *
  /// log(n)^2) worst-case.
  ///
  /// When applicable, unstable sorting is preferred because it is generally
  /// faster than stable sorting and it doesn’t allocate auxiliary memory. See
  /// `sort_unstable()`.
  //
  // TODO: Rust's stable sort is O(n * log(n)), so this can be improved.
  void sort() noexcept
    requires(!std::is_const_v<T> && ::sus::ops::Ord<T>)
  {
    if (len_ > 0_usize) {
      std::stable_sort(data_, data_ + size_t{len_});
    }
  }

  /// Sorts the slice with a comparator function.
  ///
  /// This sort is stable (i.e., does not reorder equal elements) and O(n *
  /// log(n)^2) worst-case.
  ///
  /// The comparator function must define a total ordering for the elements in
  /// the slice. If the ordering is not total, the order of the elements is
  /// unspecified.
  //
  // TODO: Rust's stable sort is O(n * log(n)), so this can be improved.
  template <::sus::fn::FnMut<::sus::fn::Anything(const T&, const T&)> F,
            int&..., class R = std::invoke_result_t<F, const T&, const T&>>
    requires(::sus::ops::Ordering<R>)
  void sort_by(F compare) noexcept
    requires(!std::is_const_v<T>)
  {
    if (len_ > 0_usize) {
      std::stable_sort(data_, data_ + size_t{len_},
                       [&compare](const T& l, const T& r) {
                         return ::sus::run_mut(compare, l, r) < 0;
                       });
    }
  }

  /// Sorts the slice, but might not preserve the order of equal elements.
  ///
  /// This sort is unstable (i.e., may reorder equal elements), in-place (i.e.,
  /// does not allocate), and O(n * log(n)) worst-case.
  void sort_unstable() noexcept
    requires(!std::is_const_v<T> && ::sus::ops::Ord<T>)
  {
    if (len_ > 0_usize) {
      std::sort(data_, data_ + size_t{len_});
    }
  }

  /// Sorts the slice with a comparator function, but might not preserve the
  /// order of equal elements.
  ///
  /// This sort is unstable (i.e., may reorder equal elements), in-place (i.e.,
  /// does not allocate), and O(n * log(n)) worst-case.
  ///
  /// The comparator function must define a total ordering for the elements in
  /// the slice. If the ordering is not total, the order of the elements is
  /// unspecified.
  template <::sus::fn::FnMut<::sus::fn::Anything(const T&, const T&)> F,
            int&..., class R = std::invoke_result_t<F, const T&, const T&>>
    requires(::sus::ops::Ordering<R>)
  void sort_unstable_by(F compare) noexcept
    requires(!std::is_const_v<T>)
  {
    if (len_ > 0_usize) {
      std::sort(data_, data_ + size_t{len_},
                [&compare](const T& l, const T& r) {
                  return ::sus::run_mut(compare, l, r) < 0;
                });
    }
  }

  /// Returns a const pointer to the first element in the slice.
  inline const T* as_ptr() const& noexcept {
    check(len_ > 0_usize);
    return data_;
  }

  /// Returns a mutable pointer to the first element in the slice.
  inline T* as_mut_ptr() & noexcept
    requires(!std::is_const_v<T>)
  {
    check(len_ > 0_usize);
    return data_;
  }

  /// Returns an iterator over all the elements in the slice, visited in the
  /// same order they appear in the slice. The iterator gives const access to
  /// each element.
  constexpr SliceIter<const T&> iter() const& noexcept {
    return SliceIter<const T&>::with(data_, len_);
  }

  /// Returns an iterator over all the elements in the slice, visited in the
  /// same order they appear in the slice. The iterator gives mutable access to
  /// each element.
  constexpr SliceIterMut<T&> iter_mut() noexcept
    requires(!std::is_const_v<T>)
  {
    return SliceIterMut<T&>::with(data_, len_);
  }

  /// Converts the slice into an iterator that consumes the slice and returns
  /// each element in the same order they appear in the array.
  ///
  /// For a Slice<const T> the iterator will return `const T&`. For a Slice<T>
  /// the iterator will return `T&`.
  constexpr SliceIter<const T&> into_iter() && noexcept
    requires(std::is_const_v<T>)
  {
    return SliceIter<const T&>::with(data_, len_);
  }
  constexpr SliceIterMut<T&> into_iter() && noexcept
    requires(!std::is_const_v<T>)
  {
    return SliceIterMut<T&>::with(data_, len_);
  }

  /// Constructs a `Vec<T>` by cloning each value in the Slice.
  Vec<std::remove_const_t<T>> to_vec() const&
    requires(::sus::mem::Clone<T>);

 private:
  constexpr Slice(T* data sus_lifetimebound, usize len) noexcept
      : data_(data), len_(len) {}

  T* data_;
  ::sus::usize len_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(data_),
                                  decltype(len_));

  // Slice does not satisfy NeverValueField because it requires that the default
  // constructor is trivial, but Slice's default constructor needs to initialize
  // its fields.
};

template <class T>
Vec<std::remove_const_t<T>> Slice<T>::to_vec() const&
  requires(::sus::mem::Clone<T>)
{
  auto v = Vec<std::remove_const_t<T>>::with_capacity(len_);
  for (::sus::usize i; i < len_; i += 1u) {
    v.push(::sus::clone(data_[size_t{i}]));
  }
  return v;
}

// Implicit for-ranged loop iteration via `Slice::iter()`.
using ::sus::iter::__private::begin;
using ::sus::iter::__private::end;

}  // namespace sus::containers

// Promote Slice into the `sus` namespace.
namespace sus {
using ::sus::containers::Slice;
}  // namespace sus
