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
#include "subspace/iter/iterator_defn.h"
#include "subspace/macros/assume.h"
#include "subspace/macros/lifetimebound.h"
#include "subspace/marker/unsafe.h"
#include "subspace/mem/addressof.h"
#include "subspace/mem/clone.h"
#include "subspace/num/unsigned_integer.h"
#include "subspace/ops/ord.h"
#include "subspace/ops/range.h"
#include "subspace/option/option.h"
#include "subspace/result/result.h"

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
      std::stable_sort(
          data_, data_ + size_t{len_},
          [&compare](const T& l, const T& r) { return compare(l, r) < 0; });
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
      std::sort(
          data_, data_ + size_t{len_},
          [&compare](const T& l, const T& r) { return compare(l, r) < 0; });
    }
  }

  /// Returns a const pointer to the first element in the slice.
  ///
  /// The caller must ensure that the container outlives the pointer this
  /// function returns, or else it will end up pointing to garbage.
  ///
  /// Modifying the container referenced by this slice may cause its buffer to
  /// be reallocated, which would also make any pointers to it invalid.
  inline const T* as_ptr() const& noexcept {
    check(len_ > 0_usize);
    return data_;
  }

  /// Returns the two const pointers spanning the slice.
  ///
  /// The returned range is half-open, which means that the end pointer points
  /// one past the last element of the slice. This way, an empty slice is
  /// represented by two equal pointers, and the difference between the two
  /// pointers represents the size of the slice.
  ///
  /// The end pointer requires caution, as it does not point to a valid element
  /// in the slice.
  ///
  /// This function is useful for interacting with interfaces which use two
  /// pointers to refer to a range of elements in memory, as is common in C++
  /// stdlib algorthms. Note that the pointers can be unpacked from the Range
  /// with structured bindings as in `auto [a, b] = s.as_ptr_range();`.
  ::sus::ops::Range<const T*> as_ptr_range() & noexcept {
    return ::sus::ops::Range<const T*>(data_, data_ + size_t{len_});
  }

  /// Returns a mutable pointer to the first element in the slice.
  ///
  /// The caller must ensure that the container outlives the pointer this
  /// function returns, or else it will end up pointing to garbage.
  ///
  /// Modifying the container referenced by this slice may cause its buffer to
  /// be reallocated, which would also make any pointers to it invalid.
  inline T* as_mut_ptr() & noexcept
    requires(!std::is_const_v<T>)
  {
    check(len_ > 0_usize);
    return data_;
  }

  /// Returns the two mutable pointers spanning the slice.
  ///
  /// The returned range is half-open, which means that the end pointer points
  /// one past the last element of the slice. This way, an empty slice is
  /// represented by two equal pointers, and the difference between the two
  /// pointers represents the size of the slice.
  ///
  /// The end pointer requires caution, as it does not point to a valid element
  /// in the slice.
  ///
  /// This function is useful for interacting with interfaces which use two
  /// pointers to refer to a range of elements in memory, as is common in C++
  /// stdlib algorthms. Note that the pointers can be unpacked from the Range
  /// with structured bindings as in `auto [a, b] = s.as_mut_ptr_range();`.
  ::sus::ops::Range<T*> as_mut_ptr_range() & noexcept
    requires(!std::is_const_v<T>)
  {
    return ::sus::ops::Range<T*>(data_, data_ + size_t{len_});
  }

  /// Binary searches this slice for a given element. This behaves similarly to
  /// contains if this slice is sorted.
  ///
  /// If the value is found then `sus::Ok` is returned, with the index
  /// of the matching element. If there are multiple matches, then any one of
  /// the matches could be returned. The index is chosen deterministically, but
  /// is subject to change in future versions of Subspace. If the value is not
  /// found then `sus::Err` is returned, with the index where a matching
  /// element could be inserted while maintaining sorted order.
  ::sus::Result<::sus::num::usize, ::sus::num::usize> binary_search(
      const T& x) const& noexcept
    requires(::sus::ops::Ord<T>)
  {
    const T* x_ptr = ::sus::mem::addressof(x);
    return binary_search_by(sus_bind_mut(
        sus_store(sus_unsafe_pointer(x_ptr)),
        [&](const T& p) -> std::strong_ordering { return p <=> *x_ptr; }));
  }

  /// Binary searches this slice with a comparator function. This behaves
  /// similarly to `contains` if this slice is sorted.
  ///
  /// The comparator function should implement an order consistent with the sort
  /// order of the underlying slice, returning a `std::strong_ordering` that
  /// indicates whether its argument is less than, equal to or greater than the
  /// desired target.
  ///
  /// If the value is found then `sus::Ok` is returned, with the index
  /// of the matching element. If there are multiple matches, then any one of
  /// the matches could be returned. The index is chosen deterministically, but
  /// is subject to change in future versions of Subspace. If the value is not
  /// found then `sus::Err` is returned, with the index where a matching
  /// element could be inserted while maintaining sorted order.
  ::sus::Result<::sus::num::usize, ::sus::num::usize> binary_search_by(
      ::sus::fn::FnMut<std::strong_ordering(const T&)> f) const& noexcept
    requires(::sus::ops::Ord<T>)
  {
    using Result = ::sus::Result<::sus::num::usize, ::sus::num::usize>;

    // INVARIANTS:
    // - 0 <= left <= left + size = right <= self.len()
    // - f returns Less for everything in self[..left]
    // - f returns Greater for everything in self[right..]
    ::sus::num::usize size = len_;
    ::sus::num::usize left = 0u;
    ::sus::num::usize right = size;
    while (left < right) {
      auto mid = left + size / 2u;

      // SAFETY: the while condition means `size` is strictly positive, so
      // `size/2 < size`.  Thus `left + size/2 < left + size`, which coupled
      // with the `left + size <= len_` invariant means we have
      // `left + size/2 < len_`, and this is in-bounds.
      auto cmp = f(get_unchecked(::sus::marker::unsafe_fn, mid));

      // The order of these comparisons comes from the Rust stdlib and is
      // performance sensitive.
      if (cmp == std::strong_ordering::less) {
        left = mid + 1u;
      } else if (cmp == std::strong_ordering::greater) {
        right = mid;
      } else {
        // SAFETY: same as the `get_unchecked` above.
        sus_assume(::sus::marker::unsafe_fn, mid < len_);
        return Result::with(mid);
      }

      size = right - left;
    }

    // SAFETY: directly true from the overall invariant.
    // Note that this is `<=`, unlike the assume in the `Result::ok()` path.
    sus_assume(::sus::marker::unsafe_fn, left <= len_);
    return Result::with_err(left);
  }

  /// Binary searches this slice with a key extraction function. This behaves
  /// similarly to `contains` if this slice is sorted.
  ///
  /// Assumes that the slice is sorted by the key, for instance with sort_by_key
  /// using the same key extraction function.
  ///
  /// If the value is found then `sus::Ok` is returned, with the index
  /// of the matching element. If there are multiple matches, then any one of
  /// the matches could be returned. The index is chosen deterministically, but
  /// is subject to change in future versions of Subspace. If the value is not
  /// found then `sus::Err` is returned, with the index where a matching
  /// element could be inserted while maintaining sorted order.
  template <::sus::ops::Ord Key>
  ::sus::Result<::sus::num::usize, ::sus::num::usize> binary_search_by_key(
      const Key& key,
      ::sus::construct::Into<::sus::fn::FnMut<Key(const T&)>> auto f)
      const& noexcept {
    ::sus::fn::FnMut<Key(const T&)> fnmut = ::sus::into(::sus::move(f));
    const auto* key_ptr = ::sus::mem::addressof(key);
    auto* f_ptr = ::sus::mem::addressof(fnmut);
    return binary_search_by(sus_bind_mut(
        sus_store(sus_unsafe_pointer(key_ptr), sus_unsafe_pointer(f_ptr)),
        [&](const T& p) { return (*f_ptr)(p) <=> *key_ptr; }));
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

  T* const data_;
  ::sus::usize const len_;

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
