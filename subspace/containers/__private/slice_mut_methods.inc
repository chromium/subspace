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
// Define `_ptr_expr`, `_len_expr`, `_iter_refs_expr` and `_iter_refs_view_expr`
// when including this file to the expressions of getting the slice's data
// pointer and length.
/////////////////////////////////////////////////////////

// If we're deleting rvalue const access, we don't want to give rvalue mut
// access either. If we're not deleting rvalue usage, the type does not own the
// data being pointed to, so it can give a mutable pointer when const (since it
// can just be copied from const to become mutable anyway then). Use this in
// place of no-qualifer-at-all for methods that return a reference.
#if _delete_rvalue
#define RETURN_REF &
#define NO_RETURN_REF
#else
#define RETURN_REF const&
#define NO_RETURN_REF const&
#endif

/// Returns a mutable pointer to the first element in the slice.
///
/// The caller must ensure that the container outlives the pointer this
/// function returns, or else it will end up pointing to garbage.
///
/// Modifying the container referenced by this slice may cause its buffer to
/// be reallocated, which would also make any pointers to it invalid.
sus_pure constexpr inline T* as_mut_ptr() RETURN_REF noexcept {
  return _ptr_expr;
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
sus_pure constexpr ::sus::ops::Range<T*> as_mut_ptr_range()
    RETURN_REF noexcept {
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
constexpr ChunksExactMut<T> chunks_exact_mut(::sus::num::usize chunk_size)
    RETURN_REF noexcept {
  ::sus::check(chunk_size > 0u);
  return ChunksExactMut<T>::with(_iter_refs_expr, *this, chunk_size);
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
constexpr ChunksMut<T> chunks_mut(::sus::num::usize chunk_size)
    RETURN_REF noexcept {
  ::sus::check(chunk_size > 0u);
  return ChunksMut<T>::with(_iter_refs_expr, *this, chunk_size);
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
constexpr void copy_from_slice(const Slice<T>& src) NO_RETURN_REF noexcept
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
constexpr void copy_from_slice_unchecked(
    ::sus::marker::UnsafeFnMarker, const Slice<T>& src) NO_RETURN_REF noexcept
  requires(std::is_trivially_copy_assignable_v<T>)
{
  const ::sus::num::usize dst_len = len();
  const T* const src_ptr = src.as_ptr();
  T* const dst_ptr = as_mut_ptr();
  sus_debug_check(
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
constexpr void clone_from_slice(const Slice<T>& src) NO_RETURN_REF noexcept
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
constexpr void fill(T value) NO_RETURN_REF noexcept
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
constexpr void fill_with(::sus::fn::FnMutRef<T()> f) NO_RETURN_REF noexcept {
  T* ptr = as_mut_ptr();
  T* const end_ptr = ptr + len();
  while (ptr != end_ptr) {
    *ptr = f();
    ptr += 1u;
  }
}

/// Fills the slice with default-constructed elements of type `T`.
constexpr void fill_with_default() NO_RETURN_REF noexcept
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
sus_pure constexpr ::sus::Option<T&> first_mut() RETURN_REF noexcept {
  if (len() > 0u)
    return ::sus::Option<T&>::with(*as_mut_ptr());
  else
    return ::sus::Option<T&>();
}

/// Returns a mutable reference to the element at index `i`, or `None` if
/// `i` is beyond the end of the Slice.
sus_pure constexpr Option<T&> get_mut(::sus::num::usize i) RETURN_REF noexcept {
  if (i < len()) [[likely]]
    return Option<T&>::with(get_unchecked_mut(::sus::marker::unsafe_fn, i));
  else
    return Option<T&>();
}

/// Returns a mutable reference to the element at index `i`.
///
/// # Safety
/// The index `i` must be inside the bounds of the slice or Undefined
/// Behaviour results. The size of the slice must therefore also have a
/// length of at least 1.
sus_pure constexpr inline T& get_unchecked_mut(
    ::sus::marker::UnsafeFnMarker, ::sus::num::usize i) RETURN_REF noexcept {
  sus_debug_check(i.primitive_value < _len_expr.primitive_value);
  return as_mut_ptr()[size_t{i}];
}

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
sus_pure constexpr Option<SliceMut<T>> get_range_mut(
    const ::sus::ops::RangeBounds<::sus::num::usize> auto range)
    RETURN_REF noexcept {
  const ::sus::num::usize length = len();
  const ::sus::num::usize rstart = range.start_bound().unwrap_or(0u);
  const ::sus::num::usize rend = range.end_bound().unwrap_or(length);
  const ::sus::num::usize rlen = rend >= rstart ? rend - rstart : 0u;
  if (rlen > length) return sus::none();  // Avoid underflow below.
  // We allow rstart == len() && rend == len(), which returns an empty
  // slice.
  if (rstart > length || rstart > length - rlen) return sus::none();
  return Option<SliceMut<T>>::with(SliceMut<T>::from_raw_parts_mut(
      ::sus::marker::unsafe_fn, _iter_refs_view_expr, as_mut_ptr() + rstart,
      rlen));
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
sus_pure constexpr SliceMut<T> get_range_mut_unchecked(
    ::sus::marker::UnsafeFnMarker,
    const ::sus::ops::RangeBounds<::sus::num::usize> auto range)
    RETURN_REF noexcept {
  const ::sus::num::usize rstart = range.start_bound().unwrap_or(0u);
  const ::sus::num::usize rend = range.end_bound().unwrap_or(len());
  const ::sus::num::usize rlen = rend >= rstart ? rend - rstart : 0u;
  return SliceMut<T>::from_raw_parts_mut(::sus::marker::unsafe_fn,
                                         _iter_refs_view_expr,
                                         as_mut_ptr() + rstart, rlen);
}

/// Returns an iterator over all the elements in the slice, visited in the
/// same order they appear in the slice. The iterator gives mutable access to
/// each element.
sus_pure constexpr SliceIterMut<T&> iter_mut() RETURN_REF noexcept {
  return SliceIterMut<T&>::with(_iter_refs_expr, as_mut_ptr(), len());
}

/// Returns a mutable reference to the last element of the slice, or None if it
/// is empty.
sus_pure constexpr ::sus::Option<T&> last_mut() RETURN_REF noexcept {
  const ::sus::num::usize length = len();
  if (length > 0u)
    return ::sus::Option<T&>::with(*(as_mut_ptr() + length - 1u));
  else
    return ::sus::Option<T&>();
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
constexpr RChunksExactMut<T> rchunks_exact_mut(::sus::num::usize chunk_size)
    RETURN_REF noexcept {
  ::sus::check(chunk_size > 0u);
  return RChunksExactMut<T>::with(_iter_refs_expr, *this, chunk_size);
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
constexpr RChunksMut<T> rchunks_mut(::sus::num::usize chunk_size)
    RETURN_REF noexcept {
  ::sus::check(chunk_size > 0u);
  return RChunksMut<T>::with(_iter_refs_expr, *this, chunk_size);
}

/// Reverses the order of elements in the slice, in place.
///
/// # Examples
/// ```
/// auto forward = sus::Vec<i32>::with(1, 2, 3);
/// auto sf = forward[".."_r];
/// auto backward = sus::Vec<i32>::with(3, 2, 1);
/// auto sb = backward[".."_r];
/// sf.reverse();
/// sus::check(sf == sb);
/// ```
constexpr void reverse() NO_RETURN_REF noexcept
  requires(::sus::mem::Move<T>)
{
  const auto half_len = len() / 2u;
  const auto [start, end] = as_mut_ptr_range();

  // Note, `front_half` and `back_half` will omit the middle element in an
  // odd-sized slice.

  // SAFETY: The slice is valid, as it points to the start of `*this` and
  // half_len < len() by construction above.
  auto front_half = ::sus::containers::SliceMut<T>::from_raw_parts_mut(
      ::sus::marker::unsafe_fn, _iter_refs_view_expr, start, half_len);
  // SAFETY: The slice is valid, as it points to the half_len elements from the
  // end of `*this`, and `len() - half_len + half_len == len()`.
  auto back_half = ::sus::containers::SliceMut<T>::from_raw_parts_mut(
      ::sus::marker::unsafe_fn, _iter_refs_view_expr, end - half_len, half_len);

  for (::sus::num::usize i; i < half_len; i += 1u) {
    // SAFETY: The `front_half` refers to the first `half_len` elements, and the
    // `back_half` refers to the last `half_len` elements. As theres >= `2 *
    // half_len` elements, these ranges don't overlap. We loop over `half_len`
    // here, visiting one element in the front and back halves on each
    // iteration, the elements thus do not alias.
    //
    // TODO: Verify that the bounds checks are elided here since the compiler
    // knows the length of each slice is `half_len`
    ::sus::mem::swap_nonoverlapping(::sus::marker::unsafe_fn, front_half[i],
                                    back_half[half_len - 1u - i]);
  }
}

/// Rotates the slice in-place such that the first `mid` elements of the slice
/// move to the end while the last `self.len() - mid` elements move to the
/// front. After calling `rotate_left()`, the element previously at index `mid`
/// will become the first element in the slice.
///
/// # Panics
/// This function will panic if `mid` is greater than the length of the slice.
/// Note that `mid == len()` does not panic and is a no-op rotation.
///
/// # Complexity
/// Takes linear (in `len()`) time.
void rotate_left(::sus::num::usize mid) NO_RETURN_REF noexcept
  requires(::sus::mem::Move<T>)
{
  const ::sus::num::usize length = len();
  ::sus::check(mid <= length);
  const ::sus::num::usize k = length - mid;
  T* const p = as_mut_ptr();

  if constexpr (::sus::mem::relocate_by_memcpy<T>) {
    if (!std::is_constant_evaluated()) {
      // TODO: Implement this faster path.

#if 0
      // SAFETY: The range `[p + mid - mid, p + mid + k)` is trivially
      // valid for reading and writing, as required by `ptr_rotate`.
      ::sus::container::__private::ptr_rotate(mid, p + mid, k);
      return;
#endif
    }
  }

  // In these cases, the pointers below swap with themselves, which makes them
  // overlapping.
  if (mid == 0u || k == 0u) return;

  // This portion moves everything up to `k` into place. If `k > mid` then this
  // solves the whole slice, with an even/odd mismatch fixup below.
  for (::sus::num::usize i; i < k; i += 1u) {
    ::sus::mem::swap_nonoverlapping(::sus::marker::unsafe_fn, *(p + i),
                                    *(p + mid + i));
  }
  if (k < mid) {
    // If `k < mid` The elements in the range `[k, mid)` were not moved yet. The
    // first element at `mid` now needs to move to `k`. We can solve this by
    // recursion.
    // SAFETY: `k <= len()` as `k` is constructed by subtracting `len() - mid`.
    get_range_mut_unchecked(::sus::marker::unsafe_fn, ::sus::ops::RangeFrom(k))
        .rotate_left(mid - k);
  } else if (k > mid && mid % 2u != (k - mid) % 2u) {
    // If there's a mismatch between the number of elements visited and the
    // number left in terms even/odd, then we need to adjust the last `mid`
    // elements.
    if ((k - mid) % 2u == 1u) {
      // The last elements need to be rotated right one position if we had an
      // even number of elements in [mid, k) and an odd number in [k, length).
      for (::sus::num::usize i = k; i < length - 1u; i += 1u) {
        ::sus::mem::swap_nonoverlapping(::sus::marker::unsafe_fn, *(p + i),
                                        *(p + length - 1u));
      }
    } else {
      // The last elements need to be rotated left one position if we had an
      // odd number of elements in [mid, k) and an even number in [k, length).
      for (::sus::num::usize i = k; i < length - 1u; i += 1u) {
        ::sus::mem::swap_nonoverlapping(::sus::marker::unsafe_fn, *(p + i),
                                        *(p + i + 1u));
      }
    }
  }
}

/// Rotates the slice in-place such that the first `len() - k` elements of
/// the slice move to the end while the last `k` elements move to the front.
/// After calling `rotate_right()`, the element previously at index `len() - k`
/// will become the first element in the slice.
///
/// # Panics
/// This function will panic if `k` is greater than the length of the slice.
/// Note that `k == len()` does not panic and is a no-op rotation.
///
/// # Complexity
/// Takes linear (in `len()`) time.
void rotate_right(::sus::num::usize k) NO_RETURN_REF noexcept
  requires(::sus::mem::Move<T>)
{
  rotate_left(len() - k);
}

/// Returns an iterator over mutable subslices separated by elements that match
/// `pred`, starting at the end of the slice and working backwards. The matched
/// element is not contained in the subslices.
///
/// As with `split_mut()`, if the first or last element is matched, an empty
/// slice will be the first (or last) item returned by the iterator.
constexpr RSplitMut<T> rsplit_mut(::sus::fn::FnMutRef<bool(const T&)> pred)
    RETURN_REF noexcept {
  return RSplitMut<T>::with(
      SplitMut<T>::with(_iter_refs_expr, *this, ::sus::move(pred)));
}

/// Returns an iterator over subslices separated by elements that match `pred`
/// limited to returning at most `n` items. This starts at the end of the slice
/// and works backwards. The matched element is not contained in the subslices.
///
/// The last element returned, if any, will contain the remainder of the slice.
constexpr RSplitNMut<T> rsplitn_mut(
    usize n, ::sus::fn::FnMutRef<bool(const T&)> pred) RETURN_REF noexcept {
  return RSplitNMut<T>::with(RSplitMut<T>::with(SplitMut<T>::with(
                                 _iter_refs_expr, *this, ::sus::move(pred))),
                             n);
}

#if 0
/// Reorder the slice such that the element at `index` is at its final sorted
/// position.
///
/// This reordering has the additional property that any value at position `i <
/// index` will be less than or equal to any value at a position `j > index`.
/// Additionally, this reordering is unstable (i.e. any number of equal elements
/// may end up at position `index`), in-place (i.e. does not allocate), and
/// O(*n*) worst-case. This function is also known as “kth element” in other
/// libraries. It returns a triplet of the following from the reordered slice:
/// the subslice prior to index, the element at `index`, and the subslice after
/// `index`; accordingly, the values in those two subslices will respectively
/// all be less-than-or-equal-to and greater-than-or-equal-to the value of the
/// element at `index`.
///
/// # Panics
/// Panics when `index >= len()`, meaning it always panics on empty slices.
::sus::Tuple<SliceMut<T>, T&, SliceMut<T>> select_nth_unstable(
    usize index) RETURN_REF noexcept
  requires(::sus::ops::Ord<T>)
{
  return select_nth_unstable_by(index,
                                [](const T& a, const T& b) { return a <=> b; });
}

/// Reorder the slice with a comparator function such that the element at
/// `index` is at its final sorted position.
///
/// This reordering has the additional property that any value at position` i <
/// index` will be less than or equal to any value at a position `j > index`
/// using the comparator function. Additionally, this reordering is unstable
/// (i.e. any number of equal elements may end up at position `index`), in-place
/// (i.e. does not allocate), and O(*n*) worst-case. This function is also known
/// as “kth element” in other libraries. It returns a triplet of the following
/// from the slice reordered according to the provided comparator function: the
/// subslice prior to `index`, the element at `index`, and the subslice after
/// `index`; accordingly, the values in those two subslices will respectively
/// all be less-than-or-equal-to and greater-than-or-equal-to the value of the
/// element at index.
///
/// # Panics
/// Panics when `index >= len()`, meaning it always panics on empty slices.
::sus::Tuple<SliceMut<T>, T&, SliceMut<T>> select_nth_unstable_by(
    usize index,
    ::sus::fn::FnMutRef<std::strong_ordering(const T&, const T&)> compare)
    RETURN_REF noexcept {
  ::sus::check_with_message(
      index < len(), *"partition_at_index index greater than length of slice");

  // TODO: Requires Iterator::enumerate(), max_by(), min_by()
  ::sus::unreachable();
}

/// Reorder the slice with a key extraction function such that the element at
/// `index` is at its final sorted position.
///
/// This reordering has the additional property that any value at position `i <
/// index` will be less than or equal to any value at a position `j > index`
/// using the key extraction function. Additionally, this reordering is unstable
/// (i.e. any number of equal elements may end up at position `index`), in-place
/// (i.e. does not allocate), and O(*n*) worst-case. This function is also known
/// as “kth element” in other libraries. It returns a triplet of the following
/// from the slice reordered according to the provided key extraction function:
/// the subslice prior to `index`, the element at `index`, and the subslice
/// after `index`; accordingly, the values in those two subslices will
/// respectively all be less-than-or-equal-to and greater-than-or-equal-to the
/// value of the element at `index`.
///
/// # Panics
/// Panics when `index >= len()`, meaning it always panics on empty slices.
template <::sus::fn::FnMut<::sus::fn::NonVoid(const T&)> KeyFn, int&...,
          class Key = std::invoke_result_t<KeyFn&, const T&>>
  requires(::sus::ops::Ord<Key>)
::sus::Tuple<SliceMut<T>, T&, SliceMut<T>> select_nth_unstable_by_key(
    usize index, KeyFn f) RETURN_REF noexcept {
  return select_nth_unstable_by(
      index, [&f](const T& a, const T& b) { return f(a) <=> f(b); });
}
#endif

/// Sorts the slice.
///
/// This sort is stable (i.e., does not reorder equal elements) and
/// O(n * log(n)^2) worst-case.
///
/// When applicable, unstable sorting is preferred because it is generally
/// faster than stable sorting and it doesn’t allocate auxiliary memory. See
/// `sort_unstable()`.
///
/// # Current implementation
/// The current implementation is std::stable_sort().
///
/// TODO: Rust's stable sort is O(n * log(n)), so this can be improved. It can
/// also be constexpr?
void sort() NO_RETURN_REF noexcept
  requires(::sus::ops::Ord<T>)
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
/// # Current implementation
/// The current implementation is std::stable_sort().
///
/// TODO: Rust's stable sort is O(n * log(n)), so this can be improved. It can
/// also be constexpr?
void sort_by(::sus::fn::FnMutRef<std::strong_ordering(const T&, const T&)>
                 compare) NO_RETURN_REF noexcept {
  if (len() > ::sus::num::usize(0u)) {
    std::stable_sort(
        as_mut_ptr(), as_mut_ptr() + len(),
        [&compare](const T& l, const T& r) { return compare(l, r) < 0; });
  }
}

/// Sorts the slice with a key extraction function.
///
/// This sort is stable (i.e., does not reorder equal elements) and O(m * n *
/// log(n)) worst-case, where the key function is O(m).
///
/// For expensive key functions (e.g. functions that are not simple property
/// accesses or basic operations), `sort_by_cached_key()` is likely to be
/// significantly faster, as it does not recompute element keys.
///
/// When applicable, unstable sorting is preferred because it is generally
/// faster than stable sorting and it doesn’t allocate auxiliary memory. See
/// `sort_unstable_by_key()`.
///
/// # Current implementation
/// The current implementation is std::stable_sort().
///
/// TODO: Rust's stable sort is O(n * log(n)), so this can be improved. It can
/// also be constexpr?
template <::sus::fn::FnMut<::sus::fn::NonVoid(const T&)> KeyFn, int&...,
          class Key = std::invoke_result_t<KeyFn&, const T&>>
  requires(::sus::ops::Ord<Key>)
void sort_by_key(KeyFn f) NO_RETURN_REF noexcept {
  return sort_by([&f](const T& a, const T& b) { return f(a) <=> f(b); });
}

template <::sus::fn::FnMut<::sus::fn::NonVoid(const T&)> KeyFn, int&...,
          class Key = std::invoke_result_t<KeyFn&, const T&>>
  requires(::sus::ops::Ord<Key>)
void sort_by_cached_key(KeyFn f) NO_RETURN_REF noexcept {
  constexpr auto sz_u8 = ::sus::mem::size_of<::sus::Tuple<Key, u8>>();
  constexpr auto sz_u16 = ::sus::mem::size_of<::sus::Tuple<Key, u16>>();
  constexpr auto sz_u32 = ::sus::mem::size_of<::sus::Tuple<Key, u32>>();
  constexpr auto sz_usize = ::sus::mem::size_of<::sus::Tuple<Key, usize>>();

  const auto length = len();
  if (length < 2u) {
    return;
  }
  if constexpr (sz_u8 < sz_u16) {
    if (length <= u8::MAX) {
      __private::sort_slice_by_cached_key<u8, Key, T>(*this, f);
      return;
    }
  }
  if constexpr (sz_u16 < sz_u32) {
    if (length <= u16::MAX) {
      __private::sort_slice_by_cached_key<u16, Key, T>(*this, f);
      return;
    }
  }
  if constexpr (sz_u32 < sz_usize) {
    if (length <= u32::MAX) {
      __private::sort_slice_by_cached_key<u32, Key, T>(*this, f);
      return;
    }
  }
  __private::sort_slice_by_cached_key<usize, Key, T>(*this, f);
}

/// Sorts the slice, but might not preserve the order of equal elements.
///
/// # Current implementation
/// The current implementation is std::sort(), which is O(n * log(n)) but can
/// allocate while sorting.
///
/// TODO: Rust's sort is unstable (i.e., may reorder equal elements), in-place
/// (i.e., does not allocate), and O(n * log(n)) worst-case.
constexpr void sort_unstable() NO_RETURN_REF noexcept
  requires(::sus::ops::Ord<T>)
{
  if (len() > 0u) {
    std::sort(as_mut_ptr(), as_mut_ptr() + len());
  }
}

/// Sorts the slice with a comparator function, but might not preserve the
/// order of equal elements.
///
/// The comparator function must define a total ordering for the elements in
/// the slice. If the ordering is not total, the order of the elements is
/// unspecified.
///
/// # Current implementation
/// The current implementation is std::sort(), which is O(n * log(n)) but can
/// allocate while sorting.
///
/// TODO: Rust's sort is unstable (i.e., may reorder equal elements), in-place
/// (i.e., does not allocate), and O(n * log(n)) worst-case.
constexpr void sort_unstable_by(
    ::sus::fn::FnMutRef<std::strong_ordering(const T&, const T&)> compare)
    NO_RETURN_REF noexcept {
  if (len() > 0u) {
    std::sort(as_mut_ptr(), as_mut_ptr() + len(),
              [&compare](const T& l, const T& r) { return compare(l, r) < 0; });
  }
}

/// Sorts the slice with a key extraction function, but might not preserve the
/// order of equal elements.
///
/// # Current implementation
/// The current implementation is std::sort(), which is O(n * log(n)) but can
/// allocate while sorting.
///
/// TODO: Rust's sort is unstable (i.e., may reorder equal elements), in-place
/// (i.e., does not allocate), and O(n * log(n)) worst-case.
template <::sus::fn::FnMut<::sus::fn::NonVoid(const T&)> KeyFn, int&...,
          class Key = std::invoke_result_t<KeyFn&, const T&>>
  requires(::sus::ops::Ord<Key>)
void sort_unstable_by_key(KeyFn f) NO_RETURN_REF noexcept {
  return sort_unstable_by(
      [&f](const T& a, const T& b) { return f(a) <=> f(b); });
}

/// Returns an iterator over mutable subslices separated by elements that match
/// `pred`. The matched element is not contained in the subslices.
///
/// If the first element is matched, an empty slice will be the first item
/// returned by the iterator. Similarly, if the last element in the slice is
/// matched, an empty slice will be the last item returned by the iterator.
constexpr SplitMut<T> split_mut(::sus::fn::FnMutRef<bool(const T&)> pred)
    RETURN_REF noexcept {
  return SplitMut<T>::with(_iter_refs_expr, *this, ::sus::move(pred));
}

/// Divides one mutable slice into two at an index.
///
/// The first will contain all indices from `[0, mid)` (excluding the index
/// `mid` itself) and the second will contain all indices from `[mid, len())`
/// (excluding the index `len()` itself).
///
/// # Panics
/// Panics if `mid > len()`.
sus_pure constexpr ::sus::Tuple<SliceMut<T>, SliceMut<T>> split_at_mut(
    ::sus::num::usize mid) RETURN_REF noexcept {
  ::sus::check(mid <= len());
  return split_at_mut_unchecked(::sus::marker::unsafe_fn, mid);
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
sus_pure constexpr ::sus::Tuple<SliceMut<T>, SliceMut<T>>
split_at_mut_unchecked(::sus::marker::UnsafeFnMarker,
                       ::sus::num::usize mid) RETURN_REF noexcept {
  // SAFETY: Caller has to check that `0 <= mid <= len()`.
  sus_debug_check(mid.primitive_value <= _len_expr.primitive_value);
  return ::sus::Tuple<SliceMut<T>, SliceMut<T>>::with(
      SliceMut<T>::from_raw_parts_mut(::sus::marker::unsafe_fn,
                                      _iter_refs_view_expr, as_mut_ptr(), mid),
      SliceMut<T>::from_raw_parts_mut(::sus::marker::unsafe_fn,
                                      _iter_refs_view_expr, as_mut_ptr() + mid,
                                      len() - mid));
}

/// Returns the first and all the rest of the elements of the slice, or `None`
/// if it is empty.
sus_pure constexpr ::sus::Option<::sus::Tuple<T&, SliceMut<T>>>
split_first_mut() RETURN_REF noexcept {
  if (is_empty()) return ::sus::none();
  return ::sus::some(
      ::sus::tuple(*as_mut_ptr(),
                   // SAFETY: `last` must be <= len(), and it is by construction
                   // from `1` with len() > 0.
                   get_range_mut_unchecked(::sus::marker::unsafe_fn,
                                           ::sus::ops::RangeFrom<usize>(1u))));
}

/// Returns an iterator over subslices separated by elements that match `pred`.
/// The matched element is contained in the end of the previous subslice as a
/// terminator.
///
/// If the last element of the slice is matched, that element will be considered
/// the terminator of the preceding slice. That slice will be the last item
/// returned by the iterator.
constexpr SplitInclusiveMut<T> split_inclusive_mut(
    ::sus::fn::FnMutRef<bool(const T&)> pred) RETURN_REF noexcept {
  return SplitInclusiveMut<T>::with(_iter_refs_expr, *this, ::sus::move(pred));
}

/// Returns the last and all the rest of the elements of the slice, or `None` if
/// it is empty.
sus_pure constexpr ::sus::Option<::sus::Tuple<T&, SliceMut<T>>> split_last_mut()
    RETURN_REF noexcept {
  if (is_empty()) return ::sus::none();
  const usize last = len() - 1u;
  return ::sus::some(
      ::sus::tuple(*(as_mut_ptr() + last),
                   // SAFETY: `last` must be <= len(), and it is by construction
                   // from `len() - 1`.
                   get_range_mut_unchecked(::sus::marker::unsafe_fn,
                                           ::sus::ops::RangeTo<usize>(last))));
}

/// Returns an iterator over mutable subslices separated by elements that match
/// `pred`, limited to returning at most `n` items. The matched element is not
/// contained in the subslices.
///
/// The last element returned, if any, will contain the remainder of the slice.
constexpr SplitNMut<T> splitn_mut(
    usize n, ::sus::fn::FnMutRef<bool(const T&)> pred) RETURN_REF noexcept {
  return SplitNMut<T>::with(
      SplitMut<T>::with(_iter_refs_expr, *this, ::sus::move(pred)), n);
}

/// Returns a subslice with the `prefix` removed.
///
/// If the slice starts with `prefix`, returns the subslice after the `prefix`,
/// wrapped in `Some`. If `prefix` is empty, simply returns the original slice.
///
/// If the slice does not start with `prefix`, returns `None`.
///
/// TODO: Accept a `SlicePattern<T>` concept instead of just a `Slice<T>`.
constexpr ::sus::Option<SliceMut<T>> strip_prefix_mut(const Slice<T>& prefix)
    RETURN_REF noexcept
  requires(::sus::ops::Eq<T>)
{
  const usize n = prefix.len();
  if (n <= len()) {
    auto [head, tail] = split_at_mut(n);
    if (head == prefix) {
      return ::sus::Option<SliceMut<T>>::with(tail);
    }
  }
  return ::sus::Option<SliceMut<T>>();
}

/// Returns a subslice with the `suffix` removed.
///
/// If the slice ends with `suffix`, returns the subslice before the `suffix`,
/// wrapped in `Some`. If `suffix` is empty, simply returns the original slice.
///
/// If the slice does not end with `suffix`, returns `None`.
constexpr ::sus::Option<SliceMut<T>> strip_suffix_mut(const Slice<T>& suffix)
    RETURN_REF noexcept
  requires(::sus::ops::Eq<T>)
{
  const auto l = len();
  const auto n = suffix.len();
  if (n <= l) {
    auto [head, tail] = split_at_mut(l - n);
    if (tail == suffix) {
      return ::sus::Option<SliceMut<T>>::with(head);
    }
  }
  return ::sus::Option<SliceMut<T>>();
}

/// Swaps two elements in the slice.
///
/// # Arguments
/// `a` - The index of the first element
/// `b` - The index of the second element
///
/// # Panics
/// Panics if a or b are out of bounds.
constexpr void swap(usize a, usize b) NO_RETURN_REF noexcept {
  ::sus::check(a < len() && b < len());
  // SAFETY: The check above ensures `a` and `b` are in range.
  ::sus::mem::swap(*(as_mut_ptr() + a), *(as_mut_ptr() + b));
}

/// Swaps two elements in the slice. The arguments must not both refer to the
/// same element.
///
/// For a safe alternative see `swap()`. For an unsafe variant that allows
/// overlapping `a` and `b`, see `swap_unchecked()`.
///
/// # Arguments
/// `a` - The index of the first element
/// `b` - The index of the second element
///
/// # Safety
/// If `a` or `b` are out of bounds, or if `a` and `b` refer to the same
/// element, Undefined Behaviour will occur.
constexpr void swap_nonoverlapping(::sus::marker::UnsafeFnMarker, usize a,
                                   usize b) NO_RETURN_REF noexcept {
  sus_debug_check(a.primitive_value != b.primitive_value);
  sus_debug_check(a.primitive_value < _len_expr.primitive_value);
  sus_debug_check(b.primitive_value < _len_expr.primitive_value);
  // SAFETY: The pointers are non-overlapping, as the caller is responsible to
  // check for that. The elements do not have objects overlapping with their
  // tail padding as they are part of a slice (an array).
  ::sus::mem::swap_nonoverlapping(::sus::marker::unsafe_fn, *(as_mut_ptr() + a),
                                  *(as_mut_ptr() + b));
}

/// Swaps two elements in the slice.
///
/// For a safe alternative see `swap()`.
///
/// # Arguments
/// `a` - The index of the first element
/// `b` - The index of the second element
///
/// # Safety
/// If `a` or `b` are out of bounds, Undefined Behaviour will occur.
constexpr void swap_unchecked(::sus::marker::UnsafeFnMarker, usize a,
                              usize b) NO_RETURN_REF noexcept {
  sus_debug_check(a.primitive_value < _len_expr.primitive_value);
  sus_debug_check(b.primitive_value < _len_expr.primitive_value);
  ::sus::mem::swap(*(as_mut_ptr() + a), *(as_mut_ptr() + b));
}

/// Swaps all elements in `*this` with those in `other`.
///
/// The length of `other` must be the same as `*this`.
///
/// # Panics
/// This function will panic if the two slices have different lengths, or if
/// other overlaps with `*this`.
constexpr void swap_with_slice(const SliceMut<T>& other)
    NO_RETURN_REF noexcept {
  auto const self_len = len();
  auto const other_len = other.len();
  T* const self_ptr = as_mut_ptr();
  T* const other_ptr = other.as_mut_ptr();

  ::sus::check(self_len == other_len);
  if (other_ptr <= self_ptr) {
    ::sus::check(other_ptr + other_len <= self_ptr);
  } else {
    ::sus::check(self_ptr + self_len <= other_ptr);
  }
  // SAFETY: Slice pointers are aligned, and the length has already been
  // verified to be the same for both slices, so it is valid for both pointers.
  // The `other` and `*this` slices have been checked to be non-overlapping.
  ::sus::ptr::swap_nonoverlapping(::sus::marker::unsafe_fn, self_ptr, other_ptr,
                                  self_len);
}

/// Returns an iterator over all contiguous windows of length `size`. The
/// windows overlap. If the slice is shorter than `size`, the iterator returns
/// no values.
///
/// # Panics
/// Panics if `size` is 0.
sus_pure constexpr WindowsMut<T> windows_mut(usize size) RETURN_REF noexcept {
  ::sus::check(size > 0u);
  return WindowsMut<T>::with(_iter_refs_expr, *this, size);
}

#undef RETURN_REF
#undef NO_RETURN_REF

// TODO: into_vec(Own<Slice<T>>) &&

#undef _ptr_expr
#undef _len_expr
#undef _delete_rvalue
#undef _iter_refs_expr
#undef _iter_refs_view_expr
