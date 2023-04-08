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

/////////////////////////////////////////////////////////
//
// Declares (and defines) methods of SliceMut or SliceMut-like types that
// provide mutable access or no access to the contained objects. These methods
// are suitable to be exposed from a SliceMut for mutable access.
//
// TO USE THIS INC FILE:
//
// Include it into the body of your class.
//
// Define `_ptr_expr` and `_len_expr` when including this file to the
// expressions of getting the slice's data pointer and length.
/////////////////////////////////////////////////////////

// If we're deleting rvalue const access, we don't want to give rvalue mut
// access either. Use this in place of no-qualifer-at-all for methods that
// return a reference.
#if _delete_rvalue
#define _mut_ref &
#else
#define _mut_ref
#endif

/// Returns a mutable pointer to the first element in the slice.
///
/// The caller must ensure that the container outlives the pointer this
/// function returns, or else it will end up pointing to garbage.
///
/// Modifying the container referenced by this slice may cause its buffer to
/// be reallocated, which would also make any pointers to it invalid.
sus_pure constexpr inline T* as_mut_ptr() _mut_ref noexcept { return _ptr_expr; }

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
sus_pure constexpr ::sus::ops::Range<T*> as_mut_ptr_range() _mut_ref noexcept {
  return ::sus::ops::Range<T*>(as_mut_ptr(), as_mut_ptr() + len());
}

/// Returns an iterator over `chunk_size` elements of the slice at a time,
/// starting at the beginning of the slice.
///
/// The chunks are mutable slices, and do not overlap. If `chunk_size` does
/// not divide the length of the slice, then the last up to `chunk_size-1`
/// elements will be omitted and can be retrieved from the `remainder()`
/// function of the iterator.
///
/// TODO: Verify if: Due to each chunk having exactly `chunk_size` elements,
/// the compiler can often optimize the resulting code better than in the
/// case of chunks_mut.
///
/// See `chunks_mut()` for a variant of this iterator that also returns the
/// remainder as a smaller chunk, and `rchunks_exact_mut()` for the same
/// iterator but starting at the end of the slice.
///
constexpr ChunksExactMut<T> chunks_exact_mut(
    ::sus::num::usize chunk_size) _mut_ref noexcept {
  ::sus::check(chunk_size > 0u);
  return ChunksExactMut<T>::with(
      ::sus::containers::SliceMut<T>::from_raw_parts_mut(
          ::sus::marker::unsafe_fn, as_mut_ptr(), len()),
      chunk_size);
}

/// Returns an iterator over chunk_size elements of the slice at a time,
/// starting at the beginning of the slice.
///
/// The chunks are mutable slices, and do not overlap. If chunk_size does not
/// divide the length of the slice, then the last chunk will not have length
/// chunk_size.
///
/// See `chunks_exact_mut()` for a variant of this iterator that returns
/// chunks of always exactly chunk_size elements, and `rchunks_mut()` for the
/// same iterator but starting at the end of the slice.
///
/// # Panics
/// Panics if chunk_size is 0.
///
constexpr ChunksMut<T> chunks_mut(
    ::sus::num::usize chunk_size) _mut_ref noexcept {
  ::sus::check(chunk_size > 0u);
  return ChunksMut<T>::with(::sus::containers::SliceMut<T>::from_raw_parts_mut(
                                ::sus::marker::unsafe_fn, as_mut_ptr(), len()),
                            chunk_size);
}

/// Copies all elements from src into `*this`, using a `memcpy()` or equivalent.
///
/// The length of `src` must be the same as `*this`.
///
/// This function requires that `T` is `TrivialCopy` in order to give consistent
/// performance across types. If `T` is not `TrivialCopy`, use
/// `clone_from_slice()`.
///
/// # Panics This function will panic if the two slices have different lengths,
/// or if the two slices overlap.
constexpr void copy_from_slice(Slice<T> src) noexcept
  requires(::sus::mem::TrivialCopy<T>)
{
  const ::sus::num::usize src_len = src.len();
  const ::sus::num::usize dst_len = len();
  ::sus::check(dst_len == src_len);

  const T* const src_ptr = src.as_ptr();
  T* const dst_ptr = as_mut_ptr();
  ::sus::check((src_ptr < dst_ptr && src_ptr <= dst_ptr - src_len) ||
               (dst_ptr < src_ptr && dst_ptr <= src_ptr - dst_len));

  if (std::is_constant_evaluated()) {
    // Avoid memcpy() when the size is known at compile time, as the compiler
    // can vectorize better and because memcpy() is not constexpr.
    for (::sus::num::usize i; i < dst_len; i += 1u)
      *(dst_ptr + i) = *(src_ptr + i);
  } else {
    if constexpr (::sus::mem::size_of<T>() > 1u) {
      constexpr auto max_len =
          ::sus::num::usize::MAX / ::sus::mem::size_of<T>();
      // We can't use memcpy() if the number of bytes would overflow.
      if (dst_len > max_len) {
        for (::sus::num::usize i; i < dst_len; i += 1u)
          *(dst_ptr + i) = *(src_ptr + i);
        return;
      }
    }
    // SAFETY: We do size_t multiplcation here, but we checked above that the
    // `length <= usize::MAX / size_of<T>()`, or we would have returned. This
    // means that `length * size_of<T> <= usize::MAX`. Thus we can multiply here
    // as a primitive type size_t.
    ::sus::ptr::copy_nonoverlapping(::sus::marker::unsafe_fn, src_ptr, dst_ptr,
                                    dst_len);
  }
}

/// Copies all elements from src into `*this`, using a `memcpy()` or equivalent.
///
/// This function requires that `T` is trivially copy-assignable in order to
/// give consistent performance across types. If `T` is not trivially copy-
/// assignable, use `clone_from_slice()`.
///
/// # Safety
/// The following conditions must hold, or Undefined Behaviour results:
/// * The length of `src` must be at least as large as `*this`.
/// * The length of `*this` (and `src`) must be greater than 0, and must not
/// overflow when multiplied by the size of `T`.
/// * The `src` slice must not overlap (aka alias) with `*this` in memory.
constexpr void copy_from_slice_unchecked(::sus::marker::UnsafeFnMarker,
                                         Slice<T> src) noexcept
  requires(std::is_trivially_copy_assignable_v<T>)
{
  const ::sus::num::usize dst_len = len();
  const T* const src_ptr = src.as_ptr();
  T* const dst_ptr = as_mut_ptr();
  sus_assume(
      ::sus::marker::unsafe_fn,
      (src_ptr < dst_ptr && src_ptr <= dst_ptr - dst_len.primitive_value) ||
          (dst_ptr < src_ptr && dst_ptr <= src_ptr - dst_len.primitive_value));

  if (std::is_constant_evaluated()) {
    // Avoid memcpy() when the size is known at compile time, as the compiler
    // can vectorize better and because memcpy() is not constexpr.
    for (::sus::num::usize i; i < dst_len; i += 1u)
      *(dst_ptr + i) = *(src_ptr + i);
  } else {
    ::sus::ptr::copy_nonoverlapping(::sus::marker::unsafe_fn, src_ptr, dst_ptr,
                                    dst_len);
  }
}

/// Copies the elements from `src` into `*this`.
///
/// The length of `src` must be the same as `*this`.
///
/// # Panics
/// This function will panic if the two slices have different lengths.
constexpr void clone_from_slice(Slice<T> src) noexcept
  requires(::sus::mem::Clone<T>)
{
  const ::sus::num::usize src_len = src.len();
  const ::sus::num::usize dst_len = len();
  ::sus::check(dst_len == src_len);
  const T* const src_ptr = src.as_ptr();
  T* dst_ptr = as_mut_ptr();
  for (::sus::num::usize i; i < dst_len; i += 1u) {
    ::sus::clone_into(::sus::mem::mref(*(dst_ptr + i)), *(src_ptr + i));
  }
}

/// Fills the slice with elements by cloning `value`.
constexpr void fill(T value) noexcept
  requires(::sus::mem::Clone<T>)
{
  // This method receives `value` by value to avoid the possiblity that it
  // aliases with an element in the slice. If `value` is modified by cloning
  // into an aliased element, the `value` may clone differently thereafter.
  T* ptr = as_mut_ptr();
  T* const end_ptr = ptr + len();
  while (ptr != end_ptr) {
    ::sus::clone_into(::sus::mem::mref(*ptr), value);
    ptr += 1u;
  }
}

/// Fills the slice with elements returned by calling a closure repeatedly.
///
/// This method uses a closure to create new values. If you’d rather `Clone` a
/// given value, use `fill()`. If you want to default-construct elements for a
/// type that satisfies `sus::construct::Default`, use `fill_with_default()`.
constexpr void fill_with(::sus::fn::FnMutRef<T()> f) noexcept {
  T* ptr = as_mut_ptr();
  T* const end_ptr = ptr + len();
  while (ptr != end_ptr) {
    *ptr = f();
    ptr += 1u;
  }
}

/// Fills the slice with default-constructed elements of type `T`.
constexpr void fill_with_default() noexcept
  requires(sus::construct::Default<T>)
{
  T* ptr = as_mut_ptr();
  T* const end_ptr = ptr + len();
  while (ptr != end_ptr) {
    *ptr = T();
    ptr += 1u;
  }
}

/// Returns a mutable reference to the first element of the slice, or None if it
/// is empty.
sus_pure constexpr ::sus::Option<T&> first_mut() _mut_ref noexcept {
  if (len() > 0u)
    return ::sus::Option<T&>::some(*as_mut_ptr());
  else
    return ::sus::Option<T&>::none();
}

// Documented on the const overload.
sus_pure constexpr T& operator[](::sus::num::usize i) & noexcept {
  check(i < len());
  return get_unchecked_mut(::sus::marker::unsafe_fn, i);
}

#if !_delete_rvalue
// Documented on the const overload.
constexpr T& operator[](::sus::num::usize i) && noexcept {
  check(i < len());
  return get_unchecked_mut(::sus::marker::unsafe_fn, i);
}
#endif

/// Returns a mutable reference to the element at index `i`, or `None` if
/// `i` is beyond the end of the Slice.
///
sus_pure constexpr Option<T&> get_mut(::sus::num::usize i) _mut_ref noexcept {
  if (i < len()) [[likely]]
    return Option<T&>::some(get_unchecked_mut(::sus::marker::unsafe_fn, i));
  else
    return Option<T&>::none();
}

/// Returns a mutable reference to the element at index `i`.
///
/// # Safety
/// The index `i` must be inside the bounds of the slice or Undefined
/// Behaviour results. The size of the slice must therefore also have a
/// length of at least 1.
///
sus_pure constexpr inline T& get_unchecked_mut(::sus::marker::UnsafeFnMarker,
                                      ::sus::num::usize i) _mut_ref noexcept {
  sus_assume(::sus::marker::unsafe_fn,
             i.primitive_value < _len_expr.primitive_value);
  return as_mut_ptr()[size_t{i}];
}

// Documented on the const overload of operator[](RangeBounds).
sus_pure constexpr inline SliceMut<T> operator[](
    const ::sus::ops::RangeBounds<::sus::num::usize> auto range) & noexcept {
  const ::sus::num::usize length = len();
  const ::sus::num::usize rstart = range.start_bound().unwrap_or(0u);
  const ::sus::num::usize rend = range.end_bound().unwrap_or(length);
  const ::sus::num::usize rlen = rend >= rstart ? rend - rstart : 0u;
  ::sus::check(rlen <= length);  // Avoid underflow below.
  // We allow rstart == len() && rend == len(), which returns an empty
  // slice.
  ::sus::check(rstart <= length && rstart <= length - rlen);
  return SliceMut<T>(as_mut_ptr() + rstart, rlen);
}

#if !_delete_rvalue
// Documented on the const overload of operator[](RangeBounds).
sus_pure constexpr inline SliceMut<T> operator[](
    const ::sus::ops::RangeBounds<::sus::num::usize> auto range) && noexcept {
  const ::sus::num::usize length = len();
  const ::sus::num::usize rstart = range.start_bound().unwrap_or(0u);
  const ::sus::num::usize rend = range.end_bound().unwrap_or(length);
  const ::sus::num::usize rlen = rend >= rstart ? rend - rstart : 0u;
  ::sus::check(rlen <= length);  // Avoid underflow below.
  // We allow rstart == len() && rend == len(), which returns an empty
  // slice.
  ::sus::check(rstart <= length && rstart <= length - rlen);
  return SliceMut<T>(as_mut_ptr() + rstart, rlen);
}
#endif

/// Returns a subslice which contains elements in `range`, which specifies a
/// start and a length.
///
/// The start is the index of the first element to be returned in the
/// subslice, and the length is the number of elements in the output slice.
/// As such, `r.get_range(Range(0u, r.len()))` returns a slice over the full
/// set of elements in `r`.
///
/// Returns None if the Range would otherwise contain an element that is out
/// of bounds.
///
sus_pure constexpr Option<SliceMut<T>> get_range_mut(
    const ::sus::ops::RangeBounds<::sus::num::usize> auto range)
    _mut_ref noexcept {
  const ::sus::num::usize length = len();
  const ::sus::num::usize rstart = range.start_bound().unwrap_or(0u);
  const ::sus::num::usize rend = range.end_bound().unwrap_or(length);
  const ::sus::num::usize rlen = rend >= rstart ? rend - rstart : 0u;
  if (rlen > length) return sus::none();  // Avoid underflow below.
  // We allow rstart == len() && rend == len(), which returns an empty
  // slice.
  if (rstart > length || rstart > length - rlen) return sus::none();
  return Option<SliceMut<T>>::some(SliceMut(as_mut_ptr() + rstart, rlen));
}

/// Returns a subslice which contains elements in `range`, which specifies a
/// start and a length.
///
/// The start is the index of the first element to be returned in the
/// subslice, and the length is the number of elements in the output slice.
/// As such, `r.get_range(Range(0u, r.len()))` returns a slice over the full
/// set of elements in `r`.
///
/// # Safety
/// It is possible to specify a Range contains an element that is out
/// of bounds of the Slice, which can result in Undefined Behaviour.
///
sus_pure constexpr SliceMut<T> get_range_mut_unchecked(
    ::sus::marker::UnsafeFnMarker,
    const ::sus::ops::RangeBounds<::sus::num::usize> auto range)
    _mut_ref noexcept {
  const ::sus::num::usize rstart = range.start_bound().unwrap_or(0u);
  const ::sus::num::usize rend = range.end_bound().unwrap_or(len());
  const ::sus::num::usize rlen = rend >= rstart ? rend - rstart : 0u;
  return SliceMut<T>(as_mut_ptr() + rstart, rlen);
}

/// Returns an iterator over all the elements in the slice, visited in the
/// same order they appear in the slice. The iterator gives mutable access to
/// each element.
///
sus_pure constexpr SliceIterMut<T&> iter_mut() _mut_ref noexcept {
  return SliceIterMut<T&>::with(as_mut_ptr(), len());
}

/// Returns a mutable reference to the last element of the slice, or None if it
/// is empty.
sus_pure constexpr ::sus::Option<T&> last_mut() _mut_ref noexcept {
  const ::sus::num::usize length = len();
  if (length > 0u)
    return ::sus::Option<T&>::some(*(as_mut_ptr() + length - 1u));
  else
    return ::sus::Option<T&>::none();
}

/// Returns an iterator over `chunk_size` elements of the slice at a time,
/// starting at the end of the slice.
///
/// The chunks are mutable slices, and do not overlap. If `chunk_size` does not
/// divide the length of the slice, then the last up to `chunk_size-1` elements
/// will be omitted and can be retrieved from the `remainder()` function of the
/// iterator.
///
/// Due to each chunk having exactly `chunk_size` elements, the compiler can
/// often optimize the resulting code better than in the case of chunks_mut.
///
/// See `rchunks_mut()` for a variant of this iterator that also returns the
/// remainder as a smaller chunk, and `chunks_exact_mut()` for the same iterator
/// but starting at the beginning of the slice.
///
/// # Panics
/// Panics if `chunk_size` is 0.
constexpr RChunksExactMut<T> rchunks_exact_mut(
    ::sus::num::usize chunk_size) _mut_ref noexcept {
  ::sus::check(chunk_size > 0u);
  return RChunksExactMut<T>::with(
      ::sus::containers::SliceMut<T>::from_raw_parts_mut(
          ::sus::marker::unsafe_fn, as_mut_ptr(), len()),
      chunk_size);
}

/// Returns an iterator over `chunk_size` elements of the slice at a time,
/// starting at the end of the slice.
///
/// The chunks are mutable slices, and do not overlap. If `chunk_size` does not
/// divide the length of the slice, then the last chunk will not have length
/// `chunk_size`.
///
/// See `rchunks_exact_mut()` for a variant of this iterator that returns chunks
/// of always exactly chunk_size elements, and `chunks_mut()` for the same
/// iterator but starting at the beginning of the slice.
///
/// # Panics
/// Panics if `chunk_size` is 0.
constexpr RChunksMut<T> rchunks_mut(
    ::sus::num::usize chunk_size) _mut_ref noexcept {
  ::sus::check(chunk_size > 0u);
  return RChunksMut<T>::with(
      ::sus::containers::SliceMut<T>::from_raw_parts_mut(
          ::sus::marker::unsafe_fn, as_mut_ptr(), len()),
      chunk_size);
}

/// Reverses the order of elements in the slice, in place.
///
/// # Examples
/// ```
/// auto forward = sus::Vec<i32>::with_values(1, 2, 3);
/// auto sf = forward[".."_r];
/// auto backward = sus::Vec<i32>::with_values(3, 2, 1);
/// auto sb = backward[".."_r];
/// sf.reverse();
/// sus::check(sf == sb);
/// ```
constexpr void reverse() noexcept
  requires(::sus::mem::Move<T>)
{
  const auto half_len = len() / 2;
  const auto [start, end] = as_mut_ptr_range();

  // SAFETY: The slice is valid, as it points to the start of `*this` and
  // half_len < len() by construction above.
  auto front_half = ::sus::containers::SliceMut<T>::from_raw_parts_mut(
      ::sus::marker::unsafe_fn, start, half_len);
  // SAFETY: The slice is valid, as it points to the half_len elements from the
  // end of `*this`, and half_len < len() by construction above.
  auto back_half = ::sus::containers::SliceMut<T>::from_raw_parts_mut(
      ::sus::marker::unsafe_fn, end - half_len, half_len);

  for (::sus::num::usize i; i < half_len; i += 1u) {
    // SAFETY: The `front_half` refers to the first `half_len` elements, and the
    // `back_half` refers to the last `half_len` elements. As theres >= `2 *
    // half_len` elements, these ranges don't overlap. We loop over `half_len`
    // here, visiting one element in the front and back halves on each
    // iteration, the elements thus do not alias.
    //
    // TODO: Verify that the bounds checks are elided here since the compiler
    // knows the length of each slice is `half_len`
    ::sus::mem::swap_no_alias_unchecked(::sus::marker::unsafe_fn, front_half[i],
                                        back_half[half_len - 1u - i]);
  }
}

/// Sorts the slice.
///
/// This sort is stable (i.e., does not reorder equal elements) and
/// O(n * log(n)^2) worst-case.
///
/// When applicable, unstable sorting is preferred because it is generally
/// faster than stable sorting and it doesn’t allocate auxiliary memory. See
/// `sort_unstable()`.
///
//
// TODO: Rust's stable sort is O(n * log(n)), so this can be improved. It can
// also be constexpr?
void sort() noexcept
  requires(!std::is_const_v<T> && ::sus::ops::Ord<T>)
{
  if (len() > ::sus::num::usize(0u)) {
    std::stable_sort(as_mut_ptr(), as_mut_ptr() + len());
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
///
//
// TODO: Rust's stable sort is O(n * log(n)), so this can be improved. It can
// also be constexpr?
template <::sus::fn::FnMut<::sus::fn::Anything(const T&, const T&)> F, int&...,
          class R = std::invoke_result_t<F, const T&, const T&>>
  requires(::sus::ops::Ordering<R>)
void sort_by(F compare) noexcept {
  if (len() > ::sus::num::usize(0u)) {
    std::stable_sort(
        as_mut_ptr(), as_mut_ptr() + len(),
        [&compare](const T& l, const T& r) { return compare(l, r) < 0; });
  }
}

/// Sorts the slice, but might not preserve the order of equal elements.
///
/// This sort is unstable (i.e., may reorder equal elements), in-place (i.e.,
/// does not allocate), and O(n * log(n)) worst-case.
///
constexpr void sort_unstable() noexcept
  requires(!std::is_const_v<T> && ::sus::ops::Ord<T>)
{
  if (len() > ::sus::num::usize(0u)) {
    std::sort(as_mut_ptr(), as_mut_ptr() + len());
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
///
template <::sus::fn::FnMut<::sus::fn::Anything(const T&, const T&)> F, int&...,
          class R = std::invoke_result_t<F, const T&, const T&>>
  requires(::sus::ops::Ordering<R>)
constexpr void sort_unstable_by(F compare) noexcept {
  if (len() > ::sus::num::usize(0u)) {
    std::sort(as_mut_ptr(), as_mut_ptr() + len(),
              [&compare](const T& l, const T& r) { return compare(l, r) < 0; });
  }
}

/// Divides one slice of mutable references into two at an index, without
/// doing bounds checking.
///
/// The first will contain all indices from [0, mid) (excluding the index mid
/// itself) and the second will contain all indices from [mid, len)
/// (excluding the index len itself).
///
/// For a safe alternative see `split_at_mut()`.
///
/// # Safety
/// Calling this method with an out-of-bounds index is undefined behavior
/// even if the resulting reference is not used. The caller has to ensure
/// that `0 <= mid <= len()`.
///
sus_pure constexpr ::sus::Tuple<SliceMut<T>, SliceMut<T>> split_at_mut_unchecked(
    ::sus::marker::UnsafeFnMarker, ::sus::num::usize mid) _mut_ref noexcept {
  // SAFETY: Caller has to check that `0 <= mid <= len()`.
  sus_assume(::sus::marker::unsafe_fn,
             mid.primitive_value <= len().primitive_value);
  return ::sus::Tuple<SliceMut<T>, SliceMut<T>>::with(
      SliceMut<T>::from_raw_parts_mut(::sus::marker::unsafe_fn, as_mut_ptr(),
                                      mid),
      SliceMut<T>::from_raw_parts_mut(::sus::marker::unsafe_fn,
                                      as_mut_ptr() + mid, len() - mid));
}

#undef _mut_ref

// TODO: into_vec(Own<Slice<T>>) &&