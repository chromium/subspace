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

// IWYU pragma: private
// IWYU pragma: friend "sus/.*"

///////////////////////////////////////////////////////////////////////////
//
// Declares (and defines) methods of Slice or Slice-like types that provide
// const access or no access to the contained objects. These methods are
// suitable to be exposed from a Slice for const access.
//
// TO USE THIS INC FILE:
//
// Include it into the body of your class.
//
// Define `_ptr_expr`, `_len_expr`, `_iter_refs_expr` and `_iter_refs_view_expr`
// when including this file to the expressions of getting the slice's data
// pointer and length.
//
// Define `_delete_rvalue` to true if rvalue methods that return a reference
// should be deleted.
///////////////////////////////////////////////////////////////////////////

/// Returns true if the slice has a length of 0.
sus_pure constexpr inline bool is_empty() const& noexcept {
  return len() == 0u;
}

/// Returns the number of elements in the slice.
sus_pure constexpr inline ::sus::num::usize len() const& noexcept {
  return _len_expr;
}

/// Returns a const pointer to the first element in the slice.
///
/// The caller must ensure that the collection outlives the pointer this
/// function returns, or else it will end up pointing to garbage.
///
/// Modifying the collection referenced by this slice may cause its buffer to
/// be reallocated, which would also make any pointers to it invalid.
sus_pure constexpr inline const T* as_ptr() const& noexcept {
  return _ptr_expr;
}

#if _delete_rvalue
constexpr inline const T* as_ptr() && = delete;
#endif

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
sus_pure constexpr ::sus::ops::Range<const T*> as_ptr_range() const& noexcept {
  return ::sus::ops::Range<const T*>(as_ptr(), as_ptr() + len());
}

#if _delete_rvalue
constexpr ::sus::ops::Range<const T*> as_ptr_range() && = delete;
#endif

/// Binary searches this slice for a given element. This behaves similarly
/// to contains if this slice is sorted.
///
/// If the value is found then `sus::Ok` is returned, with the index
/// of the matching element. If there are multiple matches, then any one of
/// the matches could be returned. The index is chosen deterministically, but
/// is subject to change in future versions of Subspace. If the value is not
/// found then `sus::Err` is returned, with the index where a matching
/// element could be inserted while maintaining sorted order.
constexpr ::sus::result::Result<::sus::num::usize, ::sus::num::usize>
binary_search(const T& x) const& noexcept
  requires(::sus::ops::Ord<T>)
{
  return binary_search_by([&x](const T& p) { return p <=> x; });
}

/// Binary searches this slice with a comparator function. This behaves
/// similarly to `contains` if this slice is sorted.
///
/// The comparator function should implement an order consistent with the
/// sort order of the underlying slice, returning a `std::strong_ordering`
/// that indicates whether its argument is less than, equal to or greater
/// than the desired target.
///
/// If the value is found then `sus::Ok` is returned, with the index
/// of the matching element. If there are multiple matches, then any one of
/// the matches could be returned. The index is chosen deterministically, but
/// is subject to change in future versions of Subspace. If the value is not
/// found then `sus::Err` is returned, with the index where a matching
/// element could be inserted while maintaining sorted order.
constexpr ::sus::result::Result<::sus::num::usize, ::sus::num::usize>
binary_search_by(
    ::sus::fn::FnMutRef<std::weak_ordering(const T&)> f) const& noexcept {
  using Result = ::sus::result::Result<::sus::num::usize, ::sus::num::usize>;

  // INVARIANTS:
  // - 0 <= left <= left + size = right <= self.len()
  // - f returns Less for everything in self[..left]
  // - f returns Greater for everything in self[right..]
  ::sus::num::usize size = len();
  ::sus::num::usize left = 0u;
  ::sus::num::usize right = size;
  while (left < right) {
    auto mid = left + size / 2u;

    // SAFETY: the while condition means `size` is strictly positive, so
    // `size/2 < size`.  Thus `left + size/2 < left + size`, which coupled
    // with the `left + size <= len()` invariant means we have
    // `left + size/2 < len()`, and this is in-bounds.
    auto cmp =
        ::sus::fn::call_mut(f, get_unchecked(::sus::marker::unsafe_fn, mid));

    // The order of these comparisons comes from the Rust stdlib and is
    // performance sensitive.
    if (cmp == std::strong_ordering::less) {
      left = mid + 1u;
    } else if (cmp == std::strong_ordering::greater) {
      right = mid;
    } else {
      // SAFETY: same as the `get_unchecked` above.
      sus_assume(::sus::marker::unsafe_fn,
                 mid.primitive_value < _len_expr.primitive_value);
      return Result(mid);
    }

    size = right - left;
  }

  // SAFETY: directly true from the overall invariant.
  // Note that this is `<=`, unlike the assume in the `Result::with()` path.
  sus_assume(::sus::marker::unsafe_fn,
             left.primitive_value <= _len_expr.primitive_value);
  return Result::with_err(left);
}

/// Binary searches this slice with a key extraction function. This behaves
/// similarly to `contains` if this slice is sorted.
///
/// Assumes that the slice is sorted by the key, for instance with
/// sort_by_key using the same key extraction function.
///
/// If the value is found then `sus::Ok` is returned, with the index
/// of the matching element. If there are multiple matches, then any one of
/// the matches could be returned. The index is chosen deterministically, but
/// is subject to change in future versions of Subspace. If the value is not
/// found then `sus::Err` is returned, with the index where a matching
/// element could be inserted while maintaining sorted order.
///
template <::sus::ops::StrongOrd Key>
::sus::result::
    Result<::sus::num::usize, ::sus::num::usize> constexpr binary_search_by_key(
        const Key& key,
        ::sus::fn::FnMut<Key(const T&)> auto&& f) const& noexcept {
  return binary_search_by([&key, &f](const T& p) -> std::strong_ordering {
    return ::sus::fn::call_mut(f, p) <=> key;
  });
}

/// Returns an iterator over `chunk_size` elements of the slice at a time,
/// starting at the beginning of the slice.
///
/// The chunks are slices and do not overlap. If `chunk_size` does not divide
/// the length of the slice, then the last chunk will not have length
/// `chunk_size`.
///
/// See `chunks_exact()` for a variant of this iterator that returns chunks
/// of always exactly `chunk_size` elements, and `rchunks()` for the same
/// iterator but starting at the end of the slice.
///
/// # Panics
/// Panics if chunk_size is 0.
constexpr Chunks<T> chunks(::sus::num::usize chunk_size) const& noexcept {
  return Chunks<T>::with(_iter_refs_expr, *this, chunk_size);
}

#if _delete_rvalue
constexpr Chunks<T> chunks(::sus::num::usize chunk_size) && = delete;
#endif

/// Returns an iterator over `chunk_size` elements of the slice at a time,
/// starting at the beginning of the slice.
///
/// The chunks are slices and do not overlap. If `chunk_size` does not divide
/// the length of the slice, then the last up to `chunk_size-1` elements will
/// be omitted and can be retrieved from the `remainder` function of the
/// iterator.
///
/// TODO: Verify if: due to each chunk having exactly `chunk_size` elements,
/// the compiler can often optimize the resulting code better than in the
/// case of `chunks()`.
///
/// See `chunks()` for a variant of this iterator that also returns the
/// remainder as a smaller chunk, and `rchunks_exact()` for the same iterator
/// but starting at the end of the slice.
///
/// # Panics
/// Panics if `chunk_size` is 0.
constexpr ChunksExact<T> chunks_exact(
    ::sus::num::usize chunk_size) const& noexcept {
  return ChunksExact<T>::with(_iter_refs_expr, *this, chunk_size);
}

#if _delete_rvalue
constexpr ChunksExact<T> chunks_exact(::sus::num::usize chunk_size) && = delete;
#endif

using ConcatOutputType = ::sus::collections::Vec<T>;

/// Flattens and concatenates the items in the Slice.
///
/// The items of type `T` are flattened into a collection of type
/// `T::ConcatOutputType`. This method is only supported for types that
/// satisfy the `sus::collections::Concat<T>` concept.
///
/// `Slice` itself satisfies `Concat`, with its output being `Vec`, so that a
/// `Slice` of `Slice<T>`s can be `concat()` together into a single `Vec<T>`.
///
/// # Example
/// ```
/// i32 a1[] = {1, 2}, a2[] = {3, 4};
/// Slice<i32> as[] = {Slice<i32>::from(a1), Slice<i32>::from(a2)};
/// Vec<i32> v = Slice<Slice<i32>>::from(as).concat();
/// sus::check(v == Slice<i32>::from({1, 2, 3, 4}));
/// ```
constexpr auto concat() const& noexcept
  requires(::sus::collections::Concat<T>)
{
  const auto length = len();
  ::sus::num::usize cap;
  for (::sus::num::usize i; i < length; i += 1u) {
    cap += get_unchecked(::sus::marker::unsafe_fn, i).len();
  }
  using Collection = typename T::ConcatOutputType;
  auto out = Collection::with_capacity(cap);
  for (::sus::num::usize i; i < length; i += 1u) {
    get_unchecked(::sus::marker::unsafe_fn, i).concat_into(out);
  }
  return out;
}

/// Concatenates a clone of each element in the slice into `vec`.
///
/// This method exists to satisfy `sus::collections::Concat<Slice<T>>`, for
/// `concat()` to append the elements in each Slice onto `vec`.
constexpr void concat_into(::sus::collections::Vec<T>& vec) const& noexcept
  requires(::sus::mem::Clone<T>)
{
  vec.extend_from_slice(*this);
}

/// Returns `true` if the slice contains an element with the given value.
///
/// This operation is O(n).
///
/// Note that if you have a sorted slice, `binary_search()` may be faster.
constexpr bool contains(const T& x) const& noexcept
  requires(::sus::ops::Eq<T>)
{
  const auto length = len();
  const auto* const p = as_ptr();
  for (::sus::num::usize i; i < length; i += 1u) {
    if (*(p + i) == x) return true;
  }
  return false;
}

/// Returns `true` if `suffix` is a suffix of the slice.
constexpr bool ends_with(const Slice<T>& suffix) const& noexcept
  requires(::sus::ops::Eq<T>)
{
  const auto m = len();
  const auto n = suffix.len();
  return m >= n && suffix == (*this)[::sus::ops::RangeFrom(m - n)];
}

/// Returns the first element of the slice, or `None` if it is empty.
sus_pure constexpr ::sus::Option<const T&> first() const& noexcept {
  if (len() > 0u) {
    return ::sus::Option<const T&>(
        get_unchecked(::sus::marker::unsafe_fn, 0u));
  } else {
    return ::sus::Option<const T&>();
  }
}

#if _delete_rvalue
constexpr ::sus::Option<const T&> first() && = delete;
#endif

/// Returns a const reference to the element at index `i`, or `None` if
/// `i` is beyond the end of the Slice.
sus_pure constexpr Option<const T&> get(::sus::num::usize i) const& noexcept {
  if (i < len()) [[likely]]
    return Option<const T&>(get_unchecked(::sus::marker::unsafe_fn, i));
  else
    return Option<const T&>();
}

#if _delete_rvalue
constexpr Option<const T&> get(::sus::num::usize i) && = delete;
#endif

/// Returns a const reference to the element at index `i`.
///
/// # Safety
/// The index `i` must be inside the bounds of the slice or Undefined
/// Behaviour results. The size of the slice must therefore also have a
/// length of at least 1.
sus_pure constexpr inline const T& get_unchecked(
    ::sus::marker::UnsafeFnMarker, ::sus::num::usize i) const& noexcept {
  sus_debug_check(i.primitive_value < _len_expr.primitive_value);
  return *(as_ptr() + i);
}

#if _delete_rvalue
constexpr inline const T& get_unchecked(::sus::marker::UnsafeFnMarker,
                                        ::sus::num::usize i) && = delete;
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
sus_pure constexpr Option<Slice<T>> get_range(
    const ::sus::ops::RangeBounds<::sus::num::usize> auto range)
    const& noexcept {
  const ::sus::num::usize length = len();
  const ::sus::num::usize rstart = range.start_bound().unwrap_or(0u);
  const ::sus::num::usize rend = range.end_bound().unwrap_or(length);
  const ::sus::num::usize rlen = rend >= rstart ? rend - rstart : 0u;
  if (rlen > length) return sus::none();  // Avoid underflow below.
  // We allow rstart == len() && rend == len(), which returns an empty
  // slice.
  if (rstart > length || rstart > length - rlen) return sus::none();
  return Option<Slice<T>>(Slice<T>::from_raw_collection(
      ::sus::marker::unsafe_fn, _iter_refs_view_expr, as_ptr() + rstart, rlen));
}

#if _delete_rvalue
constexpr Option<Slice<T>> get_range(
    const ::sus::ops::RangeBounds<::sus::num::usize> auto range) && = delete;
#endif

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
sus_pure constexpr Slice<T> get_range_unchecked(
    ::sus::marker::UnsafeFnMarker,
    const ::sus::ops::RangeBounds<::sus::num::usize> auto range)
    const& noexcept {
  const ::sus::num::usize rstart = range.start_bound().unwrap_or(0u);
  const ::sus::num::usize rend = range.end_bound().unwrap_or(len());
  const ::sus::num::usize rlen = rend >= rstart ? rend - rstart : 0u;
  return Slice<T>::from_raw_collection(
      ::sus::marker::unsafe_fn, _iter_refs_view_expr, as_ptr() + rstart, rlen);
}

#if _delete_rvalue
constexpr Slice<T> get_range_unchecked(
    ::sus::marker::UnsafeFnMarker,
    const ::sus::ops::RangeBounds<::sus::num::usize> auto range) && = delete;
#endif

/// Returns an iterator over all the elements in the slice, visited in the
/// same order they appear in the slice. The iterator gives const access to
/// each element.
sus_pure constexpr SliceIter<const T&> iter() const& noexcept {
  return SliceIter<const T&>::with(_iter_refs_expr, as_ptr(), len());
}

#if _delete_rvalue
constexpr SliceIter<const T&> iter() && = delete;
#endif

using JoinOutputType = ::sus::collections::Vec<T>;

/// Flattens and concatenates the items in the Slice, cloning a `separator`
/// between each item.
///
/// The items of type `T` are flattened into a collection of type
/// `T::JoinOutputType`. This method is only supported for types that
/// satisfy the `sus::collections::Join<T>` concept.
///
/// `Slice` itself satisfies `Join`, with its output being `Vec`, so that a
/// `Slice` of `Slice<T>`s can be `join()` together into a single `Vec<T>`.
///
/// # Example
/// ```
/// i32 a1[] = {1, 2}, a2[] = {3, 4}, asep[] = {10, 11, 12};
/// Slice<i32> as[] = {Slice<i32>::from(a1), Slice<i32>::from(a2)};
///
/// // Join slices with a slice between.
/// Vec<i32> v = Slice<Slice<i32>>::from(as).join(Slice<i32>::from(asep));
/// sus::check(v == sus::Vec<i32>(1, 2, 10, 11, 12, 3, 4));
///
/// // Join slices with a single item between.
/// Vec<i32> v2 = Slice<Slice<i32>>::from(as).join(99);
/// sus::check(v2 == sus::Vec<i32>(1, 2, 99, 3, 4));
/// ```
template <class Sep>
constexpr auto join(const Sep& separator) const& noexcept
  requires(::sus::collections::Join<T, Sep>)
{
  const auto length = len();
  ::sus::num::usize cap;
  for (::sus::num::usize i; i < length; i += 1u) {
    cap += get_unchecked(::sus::marker::unsafe_fn, i).len();
  }
  using Collection = typename T::JoinOutputType;
  auto out = Collection::with_capacity(cap);
  if (length > 0u) get_unchecked(::sus::marker::unsafe_fn, 0u).join_into(out);
  for (::sus::num::usize i = 1u; i < length; i += 1u) {
    T::join_sep_into(out, separator);
    get_unchecked(::sus::marker::unsafe_fn, i).join_into(out);
  }
  return out;
}

/// Joins a clone of each element in the slice into `vec`.
///
/// This method exists to satisfy `sus::collections::Join<Slice<T>, U>`,
/// for join() to append the elements in each Slice onto `vec`.
constexpr void join_into(::sus::collections::Vec<T>& vec) const& noexcept
  requires(::sus::mem::Clone<T>)
{
  vec.extend_from_slice(*this);
}

/// Joins a clone of each element in the separator into `vec`.
///
/// This method exists to satisfy `sus::collections::Join<Slice<T>, Slice<T>>`,
/// for join() to append the elements in a separator Slice onto `vec`.
///
/// #[doc.overloads=slice.join.sep.slice]
static constexpr void join_sep_into(::sus::collections::Vec<T>& vec,
                                    const Slice<T>& separator) noexcept
  requires(::sus::mem::Clone<T>)
{
  vec.extend_from_slice(separator);
}

/// Joins a clone of each element in the separator into `vec`.
///
/// This method exists to satisfy `sus::collections::Join<Slice<T>, T>`,
/// for join() to append the elements in a separator Slice onto `vec`.
///
/// #[doc.overloads=slice.join.sep.t]
static constexpr void join_sep_into(::sus::collections::Vec<T>& vec,
                                    const T& separator) noexcept
  requires(::sus::mem::Clone<T>)
{
  vec.push(::sus::clone(separator));
}

/// Returns the last element of the slice, or `None` if it is empty.
sus_pure constexpr ::sus::Option<const T&> last() const& noexcept {
  const auto length = len();
  if (length > 0u)
    return ::sus::Option<const T&>(*(as_ptr() + length - 1u));
  else
    return ::sus::Option<const T&>();
}

#if _delete_rvalue
constexpr ::sus::Option<const T&> last() && = delete;
#endif

/// Returns the index of the partition point according to the given predicate
/// (the index of the first element of the second partition).
///
/// The slice is assumed to be partitioned according to the given predicate.
/// This means that all elements for which the predicate returns true are at the
/// start of the slice and all elements for which the predicate returns false
/// are at the end. For example, `[7, 15, 3, 5, 4, 12, 6]` is partitioned under
/// the predicate `x % 2 != 0` (all odd numbers are at the start, all even at
/// the end).
///
/// If this slice is not partitioned, the returned result is unspecified and
/// meaningless, as this method performs a kind of binary search.
///
/// See also `binary_search()`, `binary_search_by()`, and
/// `binary_search_by_key()`.
constexpr ::sus::num::usize partition_point(
    ::sus::fn::FnMutRef<bool(const T&)> pred) const& noexcept {
  return binary_search_by([pred = ::sus::move(pred)](const T& x) {
           if (::sus::fn::call_mut(pred, x)) {
             return std::strong_ordering::less;
           } else {
             return std::strong_ordering::greater;
           }
         })
      .unwrap_or_else([](::sus::num::usize i) { return i; });
}

/// Returns an iterator over `chunk_size` elements of the slice at a time,
/// starting at the end of the slice.
///
/// The chunks are slices and do not overlap. If `chunk_size` does not divide
/// the length of the slice, then the last chunk will not have length
/// `chunk_size`.
///
/// See `rchunks_exact()` for a variant of this iterator that returns chunks of
/// always exactly `chunk_size` elements, and `chunks()` for the same iterator
/// but starting at the beginning of the slice.
///
/// # Panics
/// Panics if `chunk_size` is 0.
constexpr RChunks<T> rchunks(::sus::num::usize chunk_size) const& noexcept {
  return RChunks<T>::with(_iter_refs_expr, *this, chunk_size);
}

#if _delete_rvalue
constexpr RChunks<T> rchunks(::sus::num::usize chunk_size) && = delete;
#endif

/// Returns an iterator over `chunk_size` elements of the slice at a time,
/// starting at the end of the slice.
///
/// The chunks are slices and do not overlap. If `chunk_size` does not divide
/// the length of the slice, then the last up to `chunk_size-1` elements will be
/// omitted and can be retrieved from the `remainder()` function of the
/// iterator.
///
/// TODO: Verify if: Due to each chunk having exactly `chunk_size` elements, the
/// compiler can often optimize the resulting code better than in the case of
/// `rchunks()`.
///
/// See `rchunks()` for a variant of this iterator that also returns the
/// remainder as a smaller chunk, and `chunks_exact()` for the same iterator but
/// starting at the beginning of the slice.
///
/// # Panics
/// Panics if `chunk_size` is 0.
constexpr RChunksExact<T> rchunks_exact(
    ::sus::num::usize chunk_size) const& noexcept {
  return RChunksExact<T>::with(_iter_refs_expr, *this, chunk_size);
}

#if _delete_rvalue
constexpr RChunksExact<T> rchunks_exact(::sus::num::usize chunk_size) && =
    delete;
#endif

/// Creates a vector by copying a slice n times.
///
/// # Panics
/// This function will panic if the capacity would become larger than
/// `isize::MAX`.
///
/// # Examples
///
/// ```
/// auto v = sus::Vec<i32>(1, 2);
/// check(v[".."_r].repeat(3) == sus::vec(1, 2, 1, 2, 1, 2).construct<i32>());
/// ```
constexpr ::sus::collections::Vec<T> repeat(usize n) const& noexcept
  requires(::sus::mem::TrivialCopy<T>)
{
  auto buf = ::sus::collections::Vec<T>();
  if (n == 0u) return buf;

  const usize l = len();
  // If capacity ends up larger than isize::MAX, the Vec::reserve() call will
  // panic, so we don't need to check for overflow here too.
  const usize capacity = l.saturating_mul(n);
  buf.reserve(capacity);

  // If `n` is larger than zero, it can be split as
  // `n = 2^expn + rem (2^expn > rem, expn >= 0, rem >= 0)`.
  // `2^expn` is the number represented by the leftmost '1' bit of `n`,
  // and `rem` is the remaining part of `n`.

  buf.extend(iter());
  {
    usize m = n >> 1u;
    // If `m > 0`, there are remaining bits up to the leftmost '1'.
    while (m > 0u) {
      // `buf.extend(buf)`:
      ::sus::ptr::copy_nonoverlapping(::sus::marker::unsafe_fn, buf.as_ptr(),
                                      buf.as_mut_ptr() + buf.len(), buf.len());
      // `buf` has capacity of `self.len() * n`.
      const usize buf_len = buf.len();
      buf.set_len(::sus::marker::unsafe_fn, buf_len * 2u);
      m >>= 1u;
    }
  }

  // `rem` (`= n - 2^expn`) repetition is done by copying first `rem`
  // repetitions from `buf` itself.
  const usize rem_len = capacity - buf.len();  // `self.len() * rem`
  if (rem_len > 0u) {
    // `buf.extend(buf[0 .. rem_len])`:
    // This is non-overlapping since `2^expn > rem`.
    ::sus::ptr::copy_nonoverlapping(::sus::marker::unsafe_fn, buf.as_ptr(),
                                    buf.as_mut_ptr() + buf.len(), rem_len);
    // `buf.len() + rem_len` equals to `buf.capacity()` (`= self.len() * n`).
    buf.set_len(::sus::marker::unsafe_fn, capacity);
  }
  return buf;
}

/// Returns an iterator over subslices separated by elements that match `pred`,
/// starting at the end of the slice and working backwards. The matched element
/// is not contained in the subslices.
///
/// As with `split()`, if the first or last element is matched, an empty slice
/// will be the first (or last) item returned by the iterator.
constexpr RSplit<T> rsplit(
    ::sus::fn::FnMutRef<bool(const T&)> pred) const& noexcept {
  return RSplit<T>::with(
      Split<T>::with(_iter_refs_expr, *this, ::sus::move(pred)));
}

#if _delete_rvalue
constexpr Split<T> rsplit(::sus::fn::FnMutRef<bool(const T&)>) && = delete;
#endif

/// Returns an iterator over subslices separated by elements that match `pred`
/// limited to returning at most `n` items. This starts at the end of the slice
/// and works backwards. The matched element is not contained in the subslices.
///
/// The last element returned, if any, will contain the remainder of the slice.
constexpr RSplitN<T> rsplitn(
    usize n, ::sus::fn::FnMutRef<bool(const T&)> pred) const& noexcept {
  return RSplitN<T>::with(RSplit<T>::with(Split<T>::with(_iter_refs_expr, *this,
                                                         ::sus::move(pred))),
                          n);
}

#if _delete_rvalue
constexpr RSplit<T> rsplitn(
    usize n, ::sus::fn::FnMutRef<bool(const T&)> pred) && = delete;
#endif

/// Returns an iterator over subslices separated by elements that match `pred`.
/// The matched element is not contained in the subslices.
///
/// If the first element is matched, an empty slice will be the first item
/// returned by the iterator. Similarly, if the last element in the slice is
/// matched, an empty slice will be the last item returned by the iterator.
constexpr Split<T> split(
    ::sus::fn::FnMutRef<bool(const T&)> pred) const& noexcept {
  return Split<T>::with(_iter_refs_expr, *this, ::sus::move(pred));
}

#if _delete_rvalue
constexpr Split<T> split(::sus::fn::FnMutRef<bool(const T&)>) && = delete;
#endif

/// Divides one slice into two at an index.
///
/// The first will contain all indices from `[0, mid)` (excluding the index
/// `mid` itself) and the second will contain all indices from `[mid, len())`
/// (excluding the index `len()` itself).
///
/// # Panics
/// Panics if `mid > len()`.
sus_pure constexpr ::sus::Tuple<Slice<T>, Slice<T>> split_at(
    ::sus::num::usize mid) const& noexcept {
  ::sus::check(mid <= len());
  return split_at_unchecked(::sus::marker::unsafe_fn, mid);
}

#if _delete_rvalue
constexpr ::sus::Tuple<Slice<T>, Slice<T>> split_at(::sus::num::usize mid) && =
    delete;
#endif

/// Divides one slice into two at an index, without doing bounds checking.
///
/// The first will contain all indices from [0, mid) (excluding the index mid
/// itself) and the second will contain all indices from [mid, len)
/// (excluding the index len itself).
///
/// For a safe alternative see `split_at()`.
///
/// # Safety
/// Calling this method with an out-of-bounds index is undefined behavior
/// even if the resulting reference is not used. The caller has to ensure
/// that `0 <= mid <= len()`.
sus_pure constexpr ::sus::Tuple<Slice<T>, Slice<T>> split_at_unchecked(
    ::sus::marker::UnsafeFnMarker, ::sus::num::usize mid) const& noexcept {
  // SAFETY: Caller has to check that `0 <= mid <= len()`.
  sus_debug_check(mid.primitive_value <= _len_expr.primitive_value);

  // SAFETY: Since `0 <= min <= len()` subtracting `len() - mid` produces a
  // non-negative result that is <= len().
  const ::sus::num::usize rem =
      len().unchecked_sub(::sus::marker::unsafe_fn, mid);

  return ::sus::Tuple<Slice<T>, Slice<T>>(
      Slice<T>::from_raw_collection(::sus::marker::unsafe_fn,
                                    _iter_refs_view_expr, as_ptr(), mid),
      Slice<T>::from_raw_collection(::sus::marker::unsafe_fn,
                                    _iter_refs_view_expr, as_ptr() + mid, rem));
}

#if _delete_rvalue
constexpr ::sus::Tuple<Slice<T>, Slice<T>> split_at_unchecked(
    ::sus::marker::UnsafeFnMarker, ::sus::num::usize mid) && = delete;
#endif

/// Returns the first and all the rest of the elements of the slice, or `None`
/// if it is empty.
sus_pure constexpr ::sus::Option<::sus::Tuple<const T&, Slice<T>>> split_first()
    const& noexcept {
  if (is_empty()) return ::sus::none();
  return ::sus::some(
      ::sus::tuple(*as_ptr(), (*this)[::sus::ops::RangeFrom<usize>(1u)]));
}

#if _delete_rvalue
constexpr ::sus::Option<::sus::Tuple<const T&, Slice<T>>> split_first() && =
    delete;
#endif

/// Returns an iterator over subslices separated by elements that match `pred`.
/// The matched element is contained in the end of the previous subslice as a
/// terminator.
///
/// If the last element of the slice is matched, that element will be considered
/// the terminator of the preceding slice. That slice will be the last item
/// returned by the iterator.
constexpr SplitInclusive<T> split_inclusive(
    ::sus::fn::FnMutRef<bool(const T&)> pred) const& noexcept {
  return SplitInclusive<T>::with(_iter_refs_expr, *this, ::sus::move(pred));
}

#if _delete_rvalue
constexpr SplitInclusive<T> split_inclusive(
    ::sus::fn::FnMutRef<bool(const T&)>) && = delete;
#endif

/// Returns the last and all the rest of the elements of the slice, or `None` if
/// it is empty.
sus_pure constexpr ::sus::Option<::sus::Tuple<const T&, Slice<T>>> split_last()
    const& noexcept {
  if (is_empty()) return ::sus::none();
  const usize last = len() - 1u;
  return ::sus::some(::sus::tuple(*(as_ptr() + last),
                                  (*this)[::sus::ops::RangeTo<usize>(last)]));
}

#if _delete_rvalue
constexpr ::sus::Option<::sus::Tuple<const T&, Slice<T>>> split_last() && =
    delete;
#endif

/// Returns an iterator over subslices separated by elements that match `pred`,
/// limited to returning at most `n` items. The matched element is not contained
/// in the subslices.
///
/// The last element returned, if any, will contain the remainder of the slice.
constexpr SplitN<T> splitn(
    usize n, ::sus::fn::FnMutRef<bool(const T&)> pred) const& noexcept {
  return SplitN<T>::with(
      Split<T>::with(_iter_refs_expr, *this, ::sus::move(pred)), n);
}

#if _delete_rvalue
constexpr Split<T> splitn(usize n,
                          ::sus::fn::FnMutRef<bool(const T&)> pred) && = delete;
#endif

/// Returns `true` if `needle` is a prefix of the slice.
constexpr bool starts_with(const Slice<T>& needle) const&
  requires(::sus::ops::Eq<T>)
{
  const auto n = needle.len();
  return len() >= n && needle == (*this)[::sus::ops::RangeTo(n)];
}

/// Returns a subslice with the `prefix` removed.
///
/// If the slice starts with `prefix`, returns the subslice after the `prefix`,
/// wrapped in `Some`. If `prefix` is empty, simply returns the original slice.
///
/// If the slice does not start with `prefix`, returns `None`.
///
/// TODO: Accept a `SlicePattern<T>` concept instead of just a `Slice<T>`.
constexpr ::sus::Option<Slice<T>> strip_prefix(
    const Slice<T>& prefix) const& noexcept
  requires(::sus::ops::Eq<T>)
{
  const usize n = prefix.len();
  if (n <= len()) {
    auto [head, tail] = split_at(n);
    if (head == prefix) {
      return ::sus::Option<Slice<T>>(tail);
    }
  }
  return ::sus::Option<Slice<T>>();
}

#if _delete_rvalue
constexpr ::sus::Option<Slice<T>> strip_prefix(const Slice<T>& prefix) const& =
    delete;
#endif

/// Returns a subslice with the `suffix` removed.
///
/// If the slice ends with `suffix`, returns the subslice before the `suffix`,
/// wrapped in `Some`. If `suffix` is empty, simply returns the original slice.
///
/// If the slice does not end with `suffix`, returns `None`.
constexpr ::sus::Option<Slice<T>> strip_suffix(
    const Slice<T>& suffix) const& noexcept
  requires(::sus::ops::Eq<T>)
{
  const auto l = len();
  const auto n = suffix.len();
  if (n <= l) {
    auto [head, tail] = split_at(l - n);
    if (tail == suffix) {
      return ::sus::Option<Slice<T>>(head);
    }
  }
  return ::sus::Option<Slice<T>>();
}

#if _delete_rvalue
constexpr ::sus::Option<Slice<T>> strip_suffix(const Slice<T>& suffix) const& =
    delete;
#endif

/// Constructs a `Vec<T>` by cloning each value in the Slice.
///
/// The caller can choose traits for the Vec by specifying the trait type.
Vec<T> to_vec() const& noexcept
  requires(::sus::mem::Clone<T>);

/// Returns an iterator over all contiguous windows of length `size`. The
/// windows overlap. If the slice is shorter than `size`, the iterator returns
/// no values.
///
/// # Panics
/// Panics if `size` is 0.
sus_pure constexpr Windows<T> windows(usize size) const& noexcept {
  ::sus::check(size > 0u);
  return Windows<T>::with(_iter_refs_expr, *this, size);
}

#undef _ptr_expr
#undef _len_expr
#undef _delete_rvalue
#undef _iter_refs_expr
#undef _iter_refs_view_expr
