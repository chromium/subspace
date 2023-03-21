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

#include <concepts>
#include <type_traits>

#include "subspace/containers/concat.h"
#include "subspace/mem/clone.h"
#include "subspace/num/unsigned_integer.h"
#include "subspace/tuple/tuple.h"

namespace sus::containers {
template <class T>
class Vec;
}

#define _sus__slice_defns(Self, _ptr_expr, _len_expr, constqual, mutqual)    \
  _sus__slice_len_defn(Self, _ptr_expr, _len_expr, constqual, mutqual);      \
  _sus__slice_as_ptr_defn(Self, _ptr_expr, _len_expr, constqual, mutqual);   \
  _sus__slice_binary_search_defn(Self, _ptr_expr, _len_expr, constqual,      \
                                 mutqual);                                   \
  _sus__slice_chunks_defn(Self, _ptr_expr, _len_expr, constqual, mutqual);   \
  _sus__slice_concat_defn(Self, _ptr_expr, _len_expr, constqual, mutqual);   \
  _sus__slice_get_defn(Self, _ptr_expr, _len_expr, constqual, mutqual);      \
  _sus__slice_range_defn(Self, _ptr_expr, _len_expr, constqual, mutqual);    \
  _sus__slice_iter_defn(Self, _ptr_expr, _len_expr, constqual, mutqual);     \
  _sus__slice_split_at_defn(Self, _ptr_expr, _len_expr, constqual, mutqual); \
  _sus__slice_to_vec_defn(Self, _ptr_expr, _len_expr, constqual, mutqual);   \
  static_assert(true)

#define _sus__slice_decls(Self, _ptr_expr, _len_expr, constqual, mutqual)  \
  _sus__slice_to_vec_decl(Self, _ptr_expr, _len_expr, constqual, mutqual); \
  static_assert(true)

#define _sus__slice_mut_defns(Self, _ptr_expr, _len_expr, constqual, mutqual)  \
  _sus__slice_as_mut_ptr_defn(Self, _ptr_expr, _len_expr, constqual, mutqual); \
  _sus__slice_chunks_mut_defn(Self, _ptr_expr, _len_expr, constqual, mutqual); \
  _sus__slice_get_mut_defn(Self, _ptr_expr, _len_expr, constqual, mutqual);    \
  _sus__slice_range_mut_defn(Self, _ptr_expr, _len_expr, constqual, mutqual);  \
  _sus__slice_iter_mut_defn(Self, _ptr_expr, _len_expr, constqual, mutqual);   \
  _sus__slice_sort_mut_defn(Self, _ptr_expr, _len_expr, constqual, mutqual);   \
  _sus__slice_split_at_mut_defn(Self, _ptr_expr, _len_expr, constqual,         \
                                mutqual);                                      \
  static_assert(true)

#define _sus__slice_mut_decls(Self, _ptr_expr, _len_expr, constqual, mutqual) \
  /**/                                                                        \
  static_assert(true)

#define _sus__slice_len_defn(Self, _ptr_expr, _len_expr, constqual, mutqual)   \
  /** Returns true if the slice has a length of 0.                             \
   */                                                                          \
  constexpr inline bool is_empty() const& noexcept { return _len_expr == 0u; } \
                                                                               \
  /** Returns the number of elements in the slice.                             \
   */                                                                          \
  constexpr inline usize len() const& noexcept { return _len_expr; }           \
  static_assert(true)

#define _sus__slice_as_ptr_defn(Self, _ptr_expr, _len_expr, constqual,         \
                                mutqual)                                       \
  /** Returns a const pointer to the first element in the slice.               \
   *                                                                           \
   * The caller must ensure that the container outlives the pointer this       \
   * function returns, or else it will end up pointing to garbage.             \
   *                                                                           \
   * Modifying the container referenced by this slice may cause its buffer to  \
   * be reallocated, which would also make any pointers to it invalid.         \
   *                                                                           \
   * # Panics                                                                  \
   * The slice must have a non-zero length, or this function will panic as the \
   * pointer would be invalid.                                                 \
   */                                                                          \
  inline const T* as_ptr() const& noexcept {                                   \
    check(_len_expr > 0_usize);                                                \
    return _ptr_expr;                                                          \
  }                                                                            \
                                                                               \
  /** Returns the two const pointers spanning the slice.                       \
   *                                                                           \
   * The returned range is half-open, which means that the end pointer points  \
   * one past the last element of the slice. This way, an empty slice is       \
   * represented by two equal pointers, and the difference between the two     \
   * pointers represents the size of the slice.                                \
   *                                                                           \
   * The end pointer requires caution, as it does not point to a valid element \
   * in the slice.                                                             \
   *                                                                           \
   * This function is useful for interacting with interfaces which use two     \
   * pointers to refer to a range of elements in memory, as is common in C++   \
   * stdlib algorthms. Note that the pointers can be unpacked from the Range   \
   * with structured bindings as in `auto [a, b] = s.as_ptr_range();`.         \
   *                                                                           \
   * # Panics                                                                  \
   * The slice must have a non-zero length, or this function will panic as the \
   * pointers would be invalid.                                                \
   */                                                                          \
  ::sus::ops::Range<const T*> as_ptr_range()& noexcept {                       \
    check(_len_expr > 0_usize);                                                \
    return ::sus::ops::Range<const T*>(_ptr_expr, _ptr_expr + _len_expr);      \
  }                                                                            \
  static_assert(true)

#define _sus__slice_as_mut_ptr_defn(Self, _ptr_expr, _len_expr, constqual,     \
                                    mutqual)                                   \
  /** Returns a mutable pointer to the first element in the slice.             \
   *                                                                           \
   * The caller must ensure that the container outlives the pointer this       \
   * function returns, or else it will end up pointing to garbage.             \
   *                                                                           \
   * Modifying the container referenced by this slice may cause its buffer to  \
   * be reallocated, which would also make any pointers to it invalid.         \
   *                                                                           \
   * # Panics                                                                  \
   * The slice must have a non-zero length, or this function will panic as the \
   * pointer would be invalid.                                                 \
   */                                                                          \
  inline T* as_mut_ptr()& noexcept                                             \
    requires(!std::is_const_v<T>)                                              \
  {                                                                            \
    check(_len_expr > 0_usize);                                                \
    return _ptr_expr;                                                          \
  }                                                                            \
                                                                               \
  /** Returns the two mutable pointers spanning the slice.                     \
   *                                                                           \
   * The returned range is half-open, which means that the end pointer points  \
   * one past the last element of the slice. This way, an empty slice is       \
   * represented by two equal pointers, and the difference between the two     \
   * pointers represents the size of the slice.                                \
   *                                                                           \
   * The end pointer requires caution, as it does not point to a valid element \
   * in the slice.                                                             \
   *                                                                           \
   * This function is useful for interacting with interfaces which use two     \
   * pointers to refer to a range of elements in memory, as is common in C++   \
   * stdlib algorthms. Note that the pointers can be unpacked from the Range   \
   * with structured bindings as in `auto [a, b] = s.as_mut_ptr_range();`.     \
   *                                                                           \
   * # Panics                                                                  \
   * The slice must have a non-zero length, or this function will panic as the \
   * pointers would be invalid.                                                \
   */                                                                          \
  ::sus::ops::Range<T*> as_mut_ptr_range()& noexcept                           \
    requires(!std::is_const_v<T>)                                              \
  {                                                                            \
    check(_len_expr > 0_usize);                                                \
    return ::sus::ops::Range<T*>(_ptr_expr, _ptr_expr + _len_expr);            \
  }                                                                            \
  static_assert(true)

#define _sus__slice_binary_search_defn(Self, _ptr_expr, _len_expr, constqual,  \
                                       mutqual)                                \
  /** Binary searches this slice for a given element. This behaves similarly   \
   * to contains if this slice is sorted.                                      \
   *                                                                           \
   * If the value is found then `sus::Ok` is returned, with the index          \
   * of the matching element. If there are multiple matches, then any one of   \
   * the matches could be returned. The index is chosen deterministically, but \
   * is subject to change in future versions of Subspace. If the value is not  \
   * found then `sus::Err` is returned, with the index where a matching        \
   * element could be inserted while maintaining sorted order.                 \
   */                                                                          \
  ::sus::result::Result<::sus::num::usize, ::sus::num::usize> binary_search(   \
      const T& x) const& noexcept                                              \
    requires(::sus::ops::Ord<T>)                                               \
  {                                                                            \
    return binary_search_by(                                                   \
        [&x](const T& p) -> std::strong_ordering { return p <=> x; });         \
  }                                                                            \
                                                                               \
  /** Binary searches this slice with a comparator function. This behaves      \
   * similarly to `contains` if this slice is sorted.                          \
   *                                                                           \
   * The comparator function should implement an order consistent with the     \
   * sort order of the underlying slice, returning a `std::strong_ordering`    \
   * that indicates whether its argument is less than, equal to or greater     \
   * than the desired target.                                                  \
   *                                                                           \
   * If the value is found then `sus::Ok` is returned, with the index          \
   * of the matching element. If there are multiple matches, then any one of   \
   * the matches could be returned. The index is chosen deterministically, but \
   * is subject to change in future versions of Subspace. If the value is not  \
   * found then `sus::Err` is returned, with the index where a matching        \
   * element could be inserted while maintaining sorted order.                 \
   */                                                                          \
  ::sus::result::Result<::sus::num::usize, ::sus::num::usize>                  \
  binary_search_by(::sus::fn::FnMutRef<std::strong_ordering(const T&)> f)      \
      const& noexcept                                                          \
    requires(::sus::ops::Ord<T>)                                               \
  {                                                                            \
    using Result =                                                             \
        ::sus::result::Result<::sus::num::usize, ::sus::num::usize>;           \
                                                                               \
    /*                                                                         \
     INVARIANTS:                                                               \
     - 0 <= left <= left + size = right <= self.len()                          \
     - f returns Less for everything in self[..left]                           \
     - f returns Greater for everything in self[right..]                       \
     */                                                                        \
    ::sus::num::usize size = _len_expr;                                        \
    ::sus::num::usize left = 0u;                                               \
    ::sus::num::usize right = size;                                            \
    while (left < right) {                                                     \
      auto mid = left + size / 2u;                                             \
                                                                               \
      /* SAFETY: the while condition means `size` is strictly positive, so     \
       `size/2 < size`.  Thus `left + size/2 < left + size`, which coupled     \
       with the `left + size <= _len_expr` invariant means we have             \
       `left + size/2 < _len_expr`, and this is in-bounds.                     \
       */                                                                      \
      auto cmp = f(get_unchecked(::sus::marker::unsafe_fn, mid));              \
                                                                               \
      /* The order of these comparisons comes from the Rust stdlib and is      \
       performance sensitive. */                                               \
      if (cmp == std::strong_ordering::less) {                                 \
        left = mid + 1u;                                                       \
      } else if (cmp == std::strong_ordering::greater) {                       \
        right = mid;                                                           \
      } else {                                                                 \
        /* SAFETY: same as the `get_unchecked` above. */                       \
        sus_assume(::sus::marker::unsafe_fn,                                   \
                   mid.primitive_value < _len_expr.primitive_value);           \
        return Result::with(mid);                                              \
      }                                                                        \
                                                                               \
      size = right - left;                                                     \
    }                                                                          \
                                                                               \
    /* SAFETY: directly true from the overall invariant.                       \
     Note that this is `<=`, unlike the assume in the `Result::ok()` path. */  \
    sus_assume(::sus::marker::unsafe_fn,                                       \
               left.primitive_value <= _len_expr.primitive_value);             \
    return Result::with_err(left);                                             \
  }                                                                            \
                                                                               \
  /** Binary searches this slice with a key extraction function. This behaves  \
   * similarly to `contains` if this slice is sorted.                          \
   *                                                                           \
   * Assumes that the slice is sorted by the key, for instance with            \
   * sort_by_key using the same key extraction function.                       \
   *                                                                           \
   * If the value is found then `sus::Ok` is returned, with the index          \
   * of the matching element. If there are multiple matches, then any one of   \
   * the matches could be returned. The index is chosen deterministically, but \
   * is subject to change in future versions of Subspace. If the value is not  \
   * found then `sus::Err` is returned, with the index where a matching        \
   * element could be inserted while maintaining sorted order.                 \
   */                                                                          \
  template <::sus::ops::Ord Key>                                               \
  ::sus::result::Result<::sus::num::usize, ::sus::num::usize>                  \
  binary_search_by_key(const Key& key, ::sus::fn::FnMut<Key(const T&)> auto f) \
      const& noexcept {                                                        \
    return binary_search_by([&key, &f](const T& p) -> std::strong_ordering {   \
      return f(p) <=> key;                                                     \
    });                                                                        \
  }                                                                            \
  static_assert(true)

#define _sus__slice_chunks_defn(Self, _ptr_expr, _len_expr, constqual,         \
                                mutqual)                                       \
  /** Returns an iterator over `chunk_size` elements of the slice at a time,   \
   * starting at the beginning of the slice.                                   \
   *                                                                           \
   * The chunks are slices and do not overlap. If `chunk_size` does not divide \
   * the length of the slice, then the last chunk will not have length         \
   * `chunk_size`.                                                             \
   *                                                                           \
   * See `chunks_exact()` for a variant of this iterator that returns chunks   \
   * of always exactly `chunk_size` elements, and `rchunks()` for the same     \
   * iterator but starting at the end of the slice.                            \
   *                                                                           \
   * # Panics                                                                  \
   * Panics if chunk_size is 0.                                                \
   */                                                                          \
  constexpr Chunks<const T> chunks(usize chunk_size) const& noexcept {         \
    ::sus::check(chunk_size > 0u);                                             \
    return Chunks<const T>::with(                                              \
        ::sus::containers::Slice<const T>::from_raw_parts(                     \
            ::sus::marker::unsafe_fn, _ptr_expr, _len_expr),                   \
        chunk_size);                                                           \
  }                                                                            \
                                                                               \
  /** Returns an iterator over `chunk_size` elements of the slice at a time,   \
   * starting at the beginning of the slice.                                   \
   *                                                                           \
   * The chunks are slices and do not overlap. If `chunk_size` does not divide \
   * the length of the slice, then the last up to `chunk_size-1` elements will \
   * be omitted and can be retrieved from the `remainder` function of the      \
   * iterator.                                                                 \
   *                                                                           \
   * TODO: Verify if: due to each chunk having exactly `chunk_size` elements,  \
   * the compiler can often optimize the resulting code better than in the     \
   * case of `chunks()`.                                                       \
   *                                                                           \
   * See `chunks()` for a variant of this iterator that also returns the       \
   * remainder as a smaller chunk, and `rchunks_exact()` for the same iterator \
   * but starting at the end of the slice.                                     \
   *                                                                           \
   * # Panics                                                                  \
   * Panics if `chunk_size` is 0.                                              \
   */                                                                          \
  constexpr ChunksExact<const T> chunks_exact(usize chunk_size)                \
      const& noexcept {                                                        \
    ::sus::check(chunk_size > 0u);                                             \
    return ChunksExact<const T>::with(                                         \
        ::sus::containers::Slice<const T>::from_raw_parts(                     \
            ::sus::marker::unsafe_fn, _ptr_expr, _len_expr),                   \
        chunk_size);                                                           \
  }                                                                            \
  static_assert(true)

#define _sus__slice_chunks_mut_defn(Self, _ptr_expr, _len_expr, constqual,     \
                                    mutqual)                                   \
  /** Returns an iterator over `chunk_size` elements of the slice at a time,   \
   * starting at the beginning of the slice.                                   \
   *                                                                           \
   * The chunks are mutable slices, and do not overlap. If `chunk_size` does   \
   * not divide the length of the slice, then the last up to `chunk_size-1`    \
   * elements will be omitted and can be retrieved from the `remainder()`      \
   * function of the iterator.                                                 \
   *                                                                           \
   * TODO: Verify if: Due to each chunk having exactly `chunk_size` elements,  \
   * the compiler can often optimize the resulting code better than in the     \
   * case of chunks_mut.                                                       \
   *                                                                           \
   * See `chunks_mut()` for a variant of this iterator that also returns the   \
   * remainder as a smaller chunk, and `rchunks_exact_mut()` for the same      \
   * iterator but starting at the end of the slice.                            \
   */                                                                          \
  constexpr ChunksExactMut<T> chunks_exact_mut(usize chunk_size)               \
      const& noexcept                                                          \
    requires(!std::is_const_v<T>)                                              \
  {                                                                            \
    ::sus::check(chunk_size > 0u);                                             \
    return ChunksExactMut<T>::with(                                            \
        ::sus::containers::Slice<T>::from_raw_parts(::sus::marker::unsafe_fn,  \
                                                    _ptr_expr, _len_expr),     \
        chunk_size);                                                           \
  }                                                                            \
                                                                               \
  /** Returns an iterator over chunk_size elements of the slice at a time,     \
   * starting at the beginning of the slice.                                   \
   *                                                                           \
   * The chunks are mutable slices, and do not overlap. If chunk_size does not \
   * divide the length of the slice, then the last chunk will not have length  \
   * chunk_size.                                                               \
   *                                                                           \
   * See `chunks_exact_mut()` for a variant of this iterator that returns      \
   * chunks of always exactly chunk_size elements, and `rchunks_mut()` for the \
   * same iterator but starting at the end of the slice.                       \
   *                                                                           \
   * # Panics                                                                  \
   * Panics if chunk_size is 0.                                                \
   */                                                                          \
  constexpr ChunksMut<T> chunks_mut(usize chunk_size) const& noexcept          \
    requires(!std::is_const_v<T>)                                              \
  {                                                                            \
    ::sus::check(chunk_size > 0u);                                             \
    return ChunksMut<T>::with(                                                 \
        ::sus::containers::Slice<T>::from_raw_parts(::sus::marker::unsafe_fn,  \
                                                    _ptr_expr, _len_expr),     \
        chunk_size);                                                           \
  }                                                                            \
  static_assert(true)

#define _sus__slice_concat_defn(Self, _ptr_expr, _len_expr, constqual,       \
                                mutqual)                                     \
  using ConcatOutputType = ::sus::containers::Vec<std::remove_const_t<T>>;   \
                                                                             \
  /** Flattens and concatenates the items in the Slice.                      \
   *                                                                         \
   * The items of type `T` are flattened into a container of type            \
   * `T::ConcatOutputType`. This method is only supported for types that     \
   * satisfy the `sus::containers::Concat<T>` concept.                       \
   *                                                                         \
   * Slice itself satisfies Concat, with its output being `Vec`, so that a   \
   * `Slice` of `Slice<T>`s can be concat() together into a single `Vec<T>`. \
   *                                                                         \
   * # Example                                                               \
   * ```                                                                     \
   * i32 a1[] = {1, 2}, a2[] = {3, 4};                                       \
   * Slice<i32> as[] = {Slice<i32>::from(a1), Slice<i32>::from(a2)};         \
   * Vec<i32> v = Slice<Slice<i32>>::from(as).concat();                      \
   * // TODO: sus::check(v == sus::vec(1_i32, 2_i32, 3_i32,                  \
   * 4_i32).construct()); sus::check(v[0u] == 1); sus::check(v[1u] == 2);    \
   * sus::check(v[2u] == 3);                                                 \
   * sus::check(v[3u] == 4);                                                 \
   * ```                                                                     \
   */                                                                        \
  constexpr auto concat() const& noexcept                                    \
    requires(::sus::containers::Concat<T>)                                   \
  {                                                                          \
    usize cap;                                                               \
    for (usize i; i < _len_expr; i += 1u) {                                  \
      cap += get_unchecked(::sus::marker::unsafe_fn, i).len();               \
    }                                                                        \
    using Container = typename T::ConcatOutputType;                          \
    auto out = Container::with_capacity(cap);                                \
    for (usize i; i < _len_expr; i += 1u) {                                  \
      get_unchecked(::sus::marker::unsafe_fn, i).concat_into(out);           \
    }                                                                        \
    return out;                                                              \
  }                                                                          \
                                                                             \
  /** Concatenates a clone of each element in the slice into `vec`.          \
   *                                                                         \
   * This method exists to satisfy `sus::containers::Concat<Slice<T>>`, for  \
   * concat() to append the elements in each Slice onto `vec`.               \
   */                                                                        \
  void concat_into(ConcatOutputType& vec) const& noexcept                    \
    requires(::sus::mem::Clone<T>)                                           \
  {                                                                          \
    vec.extend_from_slice(*this);                                            \
  }                                                                          \
  static_assert(true)

#define _sus__slice_get_defn(Self, _ptr_expr, _len_expr, constqual, mutqual)   \
  /** Returns a reference to the element at position `i` in the Slice.         \
   *                                                                           \
   * # Panics                                                                  \
   * If the index `i` is beyond the end of the slice, the function will panic. \
   * #[doc.overloads=slice.index.usize]                                        \
   */                                                                          \
  constexpr inline const T& operator[](usize i) constqual noexcept {           \
    check(i < _len_expr);                                                      \
    return _ptr_expr[i.primitive_value];                                       \
  }                                                                            \
                                                                               \
  /** Returns a const reference to the element at index `i`, or `None` if      \
   * `i` is beyond the end of the Slice.                                       \
   */                                                                          \
  constexpr Option<const T&> get(usize i) constqual noexcept {                 \
    if (i < _len_expr) [[likely]]                                              \
      return Option<const T&>::some(_ptr_expr[i.primitive_value]);             \
    else                                                                       \
      return Option<const T&>::none();                                         \
  }                                                                            \
                                                                               \
  /** Returns a const reference to the element at index `i`.                   \
   *                                                                           \
   * # Safety                                                                  \
   * The index `i` must be inside the bounds of the slice or Undefined         \
   * Behaviour results. The size of the slice must therefore also have a       \
   * length of at least 1.                                                     \
   */                                                                          \
  constexpr inline const T& get_unchecked(::sus::marker::UnsafeFnMarker,       \
                                          usize i) constqual noexcept {        \
    sus_assume(::sus::marker::unsafe_fn,                                       \
               i.primitive_value < _len_expr.primitive_value);                 \
    return _ptr_expr[i.primitive_value];                                       \
  }                                                                            \
  static_assert(true)

#define _sus__slice_get_mut_defn(Self, _ptr_expr, _len_expr, constqual,     \
                                 mutqual)                                   \
  /* Documented on the const overload. */                                   \
  constexpr T& operator[](usize i) mutqual noexcept                         \
    requires(!std::is_const_v<T>)                                           \
  {                                                                         \
    check(i < _len_expr);                                                   \
    return _ptr_expr[i.primitive_value];                                    \
  }                                                                         \
                                                                            \
  /** Returns a mutable reference to the element at index `i`, or `None` if \
   * `i` is beyond the end of the Slice.                                    \
   */                                                                       \
  constexpr Option<T&> get_mut(usize i) mutqual noexcept                    \
    requires(!std::is_const_v<T>)                                           \
  {                                                                         \
    if (i < _len_expr) [[likely]]                                           \
      return Option<T&>::some(mref(_ptr_expr[i.primitive_value]));          \
    else                                                                    \
      return Option<T&>::none();                                            \
  }                                                                         \
                                                                            \
  /** Returns a mutable reference to the element at index `i`.              \
   *                                                                        \
   * # Safety                                                               \
   * The index `i` must be inside the bounds of the slice or Undefined      \
   * Behaviour results. The size of the slice must therefore also have a    \
   * length of at least 1.                                                  \
   */                                                                       \
  constexpr inline T& get_unchecked_mut(::sus::marker::UnsafeFnMarker,      \
                                        usize i) mutqual noexcept           \
    requires(!std::is_const_v<T>)                                           \
  {                                                                         \
    sus_assume(::sus::marker::unsafe_fn,                                    \
               i.primitive_value < _len_expr.primitive_value);              \
    return _ptr_expr[i.primitive_value];                                    \
  }                                                                         \
  static_assert(true)

#define _sus__slice_range_defn(Self, _ptr_expr, _len_expr, constqual, mutqual) \
  /** Returns a subslice which contains elements in `range`, which specifies a \
   * start and a length.                                                       \
   *                                                                           \
   * The start is the index of the first element to be returned in the         \
   * subslice, and the length is the number of elements in the output slice.   \
   * As such, `r.get_range(Range(0u, r._len_expr()))` returns a slice over the \
   * full set of elements in `r`.                                              \
   *                                                                           \
   * # Panics                                                                  \
   * If the Range would otherwise contain an element that is out of bounds,    \
   * the function will panic.                                                  \
   * #[doc.overloads=slice.index.range]                                        \
   */                                                                          \
  constexpr inline Slice<const T> operator[](                                  \
      const ::sus::ops::RangeBounds<usize> auto range) constqual noexcept {    \
    const usize rstart = range.start_bound().unwrap_or(0u);                    \
    const usize rend = range.end_bound().unwrap_or(_len_expr);                 \
    const usize rlen = rend >= rstart ? rend - rstart : 0u;                    \
    ::sus::check(rlen <= _len_expr); /* Avoid underflow below. */              \
    /* We allow rstart == _len_expr && rend == _len_expr, which returns an     \
     * empty slice. */                                                         \
    ::sus::check(rstart <= _len_expr && rstart <= _len_expr - rlen);           \
    return Slice<const T>(_ptr_expr + rstart, rlen);                           \
  }                                                                            \
                                                                               \
  /** Returns a subslice which contains elements in `range`, which specifies a \
   * start and a length.                                                       \
   *                                                                           \
   * The start is the index of the first element to be returned in the         \
   * subslice, and the length is the number of elements in the output slice.   \
   * As such, `r.get_range(Range(0u, r.len()))` returns a slice over the full  \
   * set of elements in `r`.                                                   \
   *                                                                           \
   * Returns None if the Range would otherwise contain an element that is out  \
   * of bounds.                                                                \
   */                                                                          \
  constexpr Option<Slice<const T>> get_range(                                  \
      const ::sus::ops::RangeBounds<usize> auto range) constqual noexcept {    \
    const usize rstart = range.start_bound().unwrap_or(0u);                    \
    const usize rend = range.end_bound().unwrap_or(_len_expr);                 \
    const usize rlen = rend >= rstart ? rend - rstart : 0u;                    \
    if (rlen > _len_expr) return sus::none(); /* Avoid underflow below. */     \
    /* We allow rstart == _len_expr && rend == _len_expr, which returns an     \
     * empty slice. */                                                         \
    if (rstart > _len_expr || rstart > _len_expr - rlen) return sus::none();   \
    return Option<Slice<const T>>::some(Slice(_ptr_expr + rstart, rlen));      \
  }                                                                            \
                                                                               \
  /** Returns a subslice which contains elements in `range`, which specifies a \
   * start and a length.                                                       \
   *                                                                           \
   * The start is the index of the first element to be returned in the         \
   * subslice, and the length is the number of elements in the output slice.   \
   * As such, `r.get_range(Range(0u, r.len()))` returns a slice over the full  \
   * set of elements in `r`.                                                   \
   *                                                                           \
   * # Safety                                                                  \
   * It is possible to specify a Range contains an element that is out         \
   * of bounds of the Slice, which can result in Undefined Behaviour.          \
   */                                                                          \
  constexpr Slice<const T> get_range_unchecked(                                \
      ::sus::marker::UnsafeFnMarker,                                           \
      const ::sus::ops::RangeBounds<usize> auto range) constqual noexcept {    \
    const usize rstart = range.start_bound().unwrap_or(0u);                    \
    const usize rend = range.end_bound().unwrap_or(_len_expr);                 \
    const usize rlen = rend >= rstart ? rend - rstart : 0u;                    \
    return Slice<const T>(_ptr_expr + rstart, rlen);                           \
  }                                                                            \
  static_assert(true)

#define _sus__slice_range_mut_defn(Self, _ptr_expr, _len_expr, constqual,      \
                                   mutqual)                                    \
  /* Documented on the const overload of operator[](RangeBounds). */           \
  constexpr inline Slice<T> operator[](                                        \
      const ::sus::ops::RangeBounds<usize> auto range) mutqual noexcept {      \
    const usize rstart = range.start_bound().unwrap_or(0u);                    \
    const usize rend = range.end_bound().unwrap_or(_len_expr);                 \
    const usize rlen = rend >= rstart ? rend - rstart : 0u;                    \
    ::sus::check(rlen <= _len_expr); /* Avoid underflow below. */              \
    /* We allow rstart == _len_expr && rend == _len_expr, which returns an     \
     * empty slice. */                                                         \
    ::sus::check(rstart <= _len_expr && rstart <= _len_expr - rlen);           \
    return Slice<T>(_ptr_expr + rstart, rlen);                                 \
  }                                                                            \
                                                                               \
  /** Returns a subslice which contains elements in `range`, which specifies a \
   * start and a length.                                                       \
   *                                                                           \
   * The start is the index of the first element to be returned in the         \
   * subslice, and the length is the number of elements in the output slice.   \
   * As such, `r.get_range(Range(0u, r.len()))` returns a slice over the full  \
   * set of elements in `r`.                                                   \
   *                                                                           \
   * Returns None if the Range would otherwise contain an element that is out  \
   * of bounds.                                                                \
   */                                                                          \
  constexpr Option<Slice<T>> get_range_mut(                                    \
      const ::sus::ops::RangeBounds<usize> auto range) mutqual noexcept {      \
    const usize rstart = range.start_bound().unwrap_or(0u);                    \
    const usize rend = range.end_bound().unwrap_or(_len_expr);                 \
    const usize rlen = rend >= rstart ? rend - rstart : 0u;                    \
    if (rlen > _len_expr) return sus::none(); /* Avoid underflow below. */     \
    /* We allow rstart == _len_expr && rend == _len_expr, which returns an     \
     * empty slice. */                                                         \
    if (rstart > _len_expr || rstart > _len_expr - rlen) return sus::none();   \
    return Option<Slice<T>>::some(Slice(_ptr_expr + rstart, rlen));            \
  }                                                                            \
                                                                               \
  /** Returns a subslice which contains elements in `range`, which specifies a \
   * start and a length.                                                       \
   *                                                                           \
   * The start is the index of the first element to be returned in the         \
   * subslice, and the length is the number of elements in the output slice.   \
   * As such, `r.get_range(Range(0u, r.len()))` returns a slice over the full  \
   * set of elements in `r`.                                                   \
   *                                                                           \
   * # Safety                                                                  \
   * It is possible to specify a Range contains an element that is out         \
   * of bounds of the Slice, which can result in Undefined Behaviour.          \
   */                                                                          \
  constexpr Slice<T> get_range_mut_unchecked(                                  \
      ::sus::marker::UnsafeFnMarker,                                           \
      const ::sus::ops::RangeBounds<usize> auto range) mutqual noexcept {      \
    const usize rstart = range.start_bound().unwrap_or(0u);                    \
    const usize rend = range.end_bound().unwrap_or(_len_expr);                 \
    const usize rlen = rend >= rstart ? rend - rstart : 0u;                    \
    return Slice<T>(_ptr_expr + rstart, rlen);                                 \
  }                                                                            \
  static_assert(true)

#define _sus__slice_iter_defn(Self, _ptr_expr, _len_expr, constqual, mutqual) \
  /** Returns an iterator over all the elements in the slice, visited in the  \
   * same order they appear in the slice. The iterator gives const access to  \
   * each element.                                                            \
   */                                                                         \
  constexpr SliceIter<const T&> iter() constqual noexcept {                   \
    return SliceIter<const T&>::with(_ptr_expr, _len_expr);                   \
  }                                                                           \
                                                                              \
  /** Converts the slice into an iterator that consumes the slice and returns \
   * each element in the same order they appear in the array.                 \
   */                                                                         \
  constexpr SliceIter<const T&> into_iter()&& noexcept                        \
    requires(std::is_const_v<T>)                                              \
  {                                                                           \
    return SliceIter<const T&>::with(_ptr_expr, _len_expr);                   \
  }                                                                           \
  static_assert(true)

#define _sus__slice_iter_mut_defn(Self, _ptr_expr, _len_expr, constqual,       \
                                  mutqual)                                     \
  /** Returns an iterator over all the elements in the slice, visited in the   \
   * same order they appear in the slice. The iterator gives mutable access to \
   * each element.                                                             \
   */                                                                          \
  constexpr SliceIterMut<T&> iter_mut() mutqual noexcept                       \
    requires(!std::is_const_v<T>)                                              \
  {                                                                            \
    return SliceIterMut<T&>::with(_ptr_expr, _len_expr);                       \
  }                                                                            \
                                                                               \
  /* Documented in the const overload of into_iter(). */                       \
  constexpr SliceIterMut<T&> into_iter()&& noexcept                            \
    requires(!std::is_const_v<T>)                                              \
  {                                                                            \
    return SliceIterMut<T&>::with(_ptr_expr, _len_expr);                       \
  }                                                                            \
  static_assert(true)

#define _sus__slice_sort_mut_defn(Self, _ptr_expr, _len_expr, constqual,        \
                                  mutqual)                                      \
  /** Sorts the slice.                                                          \
   *                                                                            \
   * This sort is stable (i.e., does not reorder equal elements) and O(n *      \
   * log(n)^2) worst-case.                                                      \
   *                                                                            \
   * When applicable, unstable sorting is preferred because it is generally     \
   * faster than stable sorting and it doesn’t allocate auxiliary memory. See \
   * `sort_unstable()`.                                                         \
   */                                                                           \
  /* TODO: Rust's stable sort is O(n * log(n)), so this can be improved. */     \
  void sort() mutqual noexcept                                                  \
    requires(!std::is_const_v<T> && ::sus::ops::Ord<T>)                         \
  {                                                                             \
    if (_len_expr > 0_usize) {                                                  \
      std::stable_sort(_ptr_expr, _ptr_expr + _len_expr);                       \
    }                                                                           \
  }                                                                             \
                                                                                \
  /** Sorts the slice with a comparator function.                               \
   *                                                                            \
   * This sort is stable (i.e., does not reorder equal elements) and O(n *      \
   * log(n)^2) worst-case.                                                      \
   *                                                                            \
   * The comparator function must define a total ordering for the elements in   \
   * the slice. If the ordering is not total, the order of the elements is      \
   * unspecified.                                                               \
   */                                                                           \
  /* TODO: Rust's stable sort is O(n * log(n)), so this can be improved. */     \
  template <::sus::fn::FnMut<::sus::fn::Anything(const T&, const T&)> F,        \
            int&..., class R = std::invoke_result_t<F, const T&, const T&>>     \
    requires(::sus::ops::Ordering<R>)                                           \
  void sort_by(F compare) mutqual noexcept                                      \
    requires(!std::is_const_v<T>)                                               \
  {                                                                             \
    if (_len_expr > 0_usize) {                                                  \
      std::stable_sort(                                                         \
          _ptr_expr, _ptr_expr + _len_expr,                                     \
          [&compare](const T& l, const T& r) { return compare(l, r) < 0; });    \
    }                                                                           \
  }                                                                             \
                                                                                \
  /** Sorts the slice, but might not preserve the order of equal elements.      \
   *                                                                            \
   * This sort is unstable (i.e., may reorder equal elements), in-place (i.e.,  \
   * does not allocate), and O(n * log(n)) worst-case.                          \
   */                                                                           \
  void sort_unstable() mutqual noexcept                                         \
    requires(!std::is_const_v<T> && ::sus::ops::Ord<T>)                         \
  {                                                                             \
    if (_len_expr > 0_usize) {                                                  \
      std::sort(_ptr_expr, _ptr_expr + _len_expr);                              \
    }                                                                           \
  }                                                                             \
                                                                                \
  /** Sorts the slice with a comparator function, but might not preserve the    \
   * order of equal elements.                                                   \
   *                                                                            \
   * This sort is unstable (i.e., may reorder equal elements), in-place (i.e.,  \
   * does not allocate), and O(n * log(n)) worst-case.                          \
   *                                                                            \
   * The comparator function must define a total ordering for the elements in   \
   * the slice. If the ordering is not total, the order of the elements is      \
   * unspecified.                                                               \
   */                                                                           \
  template <::sus::fn::FnMut<::sus::fn::Anything(const T&, const T&)> F,        \
            int&..., class R = std::invoke_result_t<F, const T&, const T&>>     \
    requires(::sus::ops::Ordering<R>)                                           \
  void sort_unstable_by(F compare) mutqual noexcept                             \
    requires(!std::is_const_v<T>)                                               \
  {                                                                             \
    if (_len_expr > 0_usize) {                                                  \
      std::sort(                                                                \
          _ptr_expr, _ptr_expr + _len_expr,                                     \
          [&compare](const T& l, const T& r) { return compare(l, r) < 0; });    \
    }                                                                           \
  }                                                                             \
  static_assert(true)

#define _sus__slice_split_at_defn(Self, _ptr_expr, _len_expr, constqual,       \
                                  mutqual)                                     \
  /** Divides one slice into two at an index, without doing bounds checking.   \
   *                                                                           \
   * The first will contain all indices from [0, mid) (excluding the index mid \
   * itself) and the second will contain all indices from [mid, len)           \
   * (excluding the index len itself).                                         \
   *                                                                           \
   * For a safe alternative see `split_at()`.                                  \
   *                                                                           \
   * # Safety                                                                  \
   * Calling this method with an out-of-bounds index is undefined behavior     \
   * even if the resulting reference is not used. The caller has to ensure     \
   * that `0 <= mid <= len()`.                                                 \
   */                                                                          \
  constexpr ::sus::Tuple<Slice<const T>, Slice<const T>> split_at_unchecked(   \
      ::sus::marker::UnsafeFnMarker, ::sus::num::usize mid)                    \
      constqual noexcept {                                                     \
    const ::sus::num::usize length = _len_expr;                                \
                                                                               \
    /* SAFETY: Caller has to check that `0 <= mid <= _len_expr` */             \
    sus_assume(::sus::marker::unsafe_fn,                                       \
               mid.primitive_value <= length.primitive_value);                 \
    return ::sus::Tuple<Slice<const T>, Slice<const T>>::with(                 \
        Slice<const T>::from_raw_parts(::sus::marker::unsafe_fn, _ptr_expr,    \
                                       mid),                                   \
        Slice<const T>::from_raw_parts(::sus::marker::unsafe_fn,               \
                                       _ptr_expr + mid, length - mid));        \
  }                                                                            \
  static_assert(true)

#define _sus__slice_split_at_mut_defn(Self, _ptr_expr, _len_expr, constqual,   \
                                      mutqual)                                 \
  /** Divides one slice of mutable references into two at an index, without    \
   * doing bounds checking.                                                    \
   *                                                                           \
   * The first will contain all indices from [0, mid) (excluding the index mid \
   * itself) and the second will contain all indices from [mid, len)           \
   * (excluding the index len itself).                                         \
   *                                                                           \
   * For a safe alternative see `split_at_mut()`.                              \
   *                                                                           \
   * # Safety                                                                  \
   * Calling this method with an out-of-bounds index is undefined behavior     \
   * even if the resulting reference is not used. The caller has to ensure     \
   * that `0 <= mid <= len()`.                                                 \
   */                                                                          \
  constexpr ::sus::Tuple<Slice<T>, Slice<T>> split_at_mut_unchecked(           \
      ::sus::marker::UnsafeFnMarker, usize mid) constqual noexcept             \
    requires(!std::is_const_v<T>)                                              \
  {                                                                            \
    const ::sus::num::usize length = _len_expr;                                \
                                                                               \
    /* SAFETY: Caller has to check that `0 <= mid <= _len_expr` */             \
    sus_assume(::sus::marker::unsafe_fn,                                       \
               mid.primitive_value <= length.primitive_value);                 \
    return ::sus::Tuple<Slice<T>, Slice<T>>::with(                             \
        Slice<T>::from_raw_parts(::sus::marker::unsafe_fn, _ptr_expr, mid),    \
        Slice<T>::from_raw_parts(::sus::marker::unsafe_fn, _ptr_expr + mid,    \
                                 length - mid));                               \
  }                                                                            \
  static_assert(true)

#define _sus__slice_to_vec_defn(Self, _ptr_expr, _len_expr, constqual, \
                                mutqual)                               \
  /** Constructs a `Vec<T>` by cloning each value in the Slice. */     \
  Vec<std::remove_const_t<T>> to_vec() constqual                       \
    requires(::sus::mem::Clone<T>)

#define _sus__slice_to_vec_decl(Self, _ptr_expr, _len_expr, constqual, \
                                mutqual)                               \
  template <class T>                                                   \
  Vec<std::remove_const_t<T>> Self<T>::to_vec() constqual              \
    requires(::sus::mem::Clone<T>)                                     \
  {                                                                    \
    auto v = Vec<std::remove_const_t<T>>::with_capacity(_len_expr);    \
    for (::sus::usize i; i < _len_expr; i += 1u) {                     \
      v.push(::sus::clone(*(_ptr_expr + i)));                          \
    }                                                                  \
    return v;                                                          \
  }                                                                    \
  static_assert(true)
