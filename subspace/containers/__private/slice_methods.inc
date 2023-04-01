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
// Define `_ptr_expr` and `_len_expr` when including this file to the
// expressions of getting the slice's data pointer and length.
//
// Define _delete_rvalue to true if rvalue methods that return a reference
// should be deleted.
///////////////////////////////////////////////////////////////////////////

/// Returns true if the slice has a length of 0.
constexpr inline bool is_empty() const& noexcept { return _len_expr == 0u; }

/// Returns the number of elements in the slice.
constexpr inline ::sus::num::usize len() const& noexcept { return _len_expr; }

/// Returns a const pointer to the first element in the slice.
///
/// The caller must ensure that the container outlives the pointer this
/// function returns, or else it will end up pointing to garbage.
///
/// Modifying the container referenced by this slice may cause its buffer to
/// be reallocated, which would also make any pointers to it invalid.
///
/// # Panics
/// The slice must have a non-zero length, or this function will panic as the
/// pointer would be invalid.
///
constexpr inline const T* as_ptr() const& noexcept {
  ::sus::check(_len_expr > ::sus::num::usize(0u));
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
///
/// # Panics
/// The slice must have a non-zero length, or this function will panic as the
/// pointers would be invalid.
///
constexpr ::sus::ops::Range<const T*> as_ptr_range() const& noexcept {
  ::sus::check(_len_expr > ::sus::num::usize(0u));
  return ::sus::ops::Range<const T*>(_ptr_expr, _ptr_expr + _len_expr);
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
///
constexpr ::sus::result::Result<::sus::num::usize, ::sus::num::usize>
binary_search(const T& x) const& noexcept
  requires(::sus::ops::Ord<T>)
{
  return binary_search_by(
      [&x](const T& p) -> std::strong_ordering { return p <=> x; });
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
///
constexpr ::sus::result::Result<::sus::num::usize, ::sus::num::usize>
binary_search_by(
    ::sus::fn::FnMutRef<std::strong_ordering(const T&)> f) const& noexcept
  requires(::sus::ops::Ord<T>)
{
  using Result = ::sus::result::Result<::sus::num::usize, ::sus::num::usize>;

  /*
   INVARIANTS:
   - 0 <= left <= left + size = right <= self.len()
   - f returns Less for everything in self[..left]
   - f returns Greater for everything in self[right..]
   */
  ::sus::num::usize size = _len_expr;
  ::sus::num::usize left = 0u;
  ::sus::num::usize right = size;
  while (left < right) {
    auto mid = left + size / 2u;

    /* SAFETY: the while condition means `size` is strictly positive, so
     `size/2 < size`.  Thus `left + size/2 < left + size`, which coupled
     with the `left + size <= _len_expr` invariant means we have
     `left + size/2 < _len_expr`, and this is in-bounds.
     */
    auto cmp = f(get_unchecked(::sus::marker::unsafe_fn, mid));

    /* The order of these comparisons comes from the Rust stdlib and is
     performance sensitive. */
    if (cmp == std::strong_ordering::less) {
      left = mid + 1u;
    } else if (cmp == std::strong_ordering::greater) {
      right = mid;
    } else { /* SAFETY: same as the `get_unchecked` above. */
      sus_assume(::sus::marker::unsafe_fn,
                 mid.primitive_value < _len_expr.primitive_value);
      return Result::with(mid);
    }

    size = right - left;
  }

  /* SAFETY: directly true from the overall invariant.
   Note that this is `<=`, unlike the assume in the `Result::ok()` path. */
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
template <::sus::ops::Ord Key>
::sus::result::
    Result<::sus::num::usize, ::sus::num::usize> constexpr binary_search_by_key(
        const Key& key,
        ::sus::fn::FnMut<Key(const T&)> auto f) const& noexcept {
  return binary_search_by(
      [&key, &f](const T& p) -> std::strong_ordering { return f(p) <=> key; });
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
///
constexpr Chunks<T> chunks(::sus::num::usize chunk_size) const& noexcept {
  ::sus::check(chunk_size > 0u);
  return Chunks<T>::with(::sus::containers::Slice<T>::from_raw_parts(
                             ::sus::marker::unsafe_fn, _ptr_expr, _len_expr),
                         chunk_size);
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
///
constexpr ChunksExact<T> chunks_exact(
    ::sus::num::usize chunk_size) const& noexcept {
  ::sus::check(chunk_size > 0u);
  return ChunksExact<T>::with(
      ::sus::containers::Slice<T>::from_raw_parts(::sus::marker::unsafe_fn,
                                                  _ptr_expr, _len_expr),
      chunk_size);
}

#if _delete_rvalue
constexpr ChunksExact<T> chunks_exact(::sus::num::usize chunk_size) && = delete;
#endif

using ConcatOutputType = ::sus::containers::Vec<T>;

/// Flattens and concatenates the items in the Slice.
///
/// The items of type `T` are flattened into a container of type
/// `T::ConcatOutputType`. This method is only supported for types that
/// satisfy the `sus::containers::Concat<T>` concept.
///
/// Slice itself satisfies Concat, with its output being `Vec`, so that a
/// `Slice` of `Slice<T>`s can be concat() together into a single `Vec<T>`.
///
/// # Example
/// ```
/// i32 a1[] = {1, 2}, a2[] = {3, 4};
/// Slice<i32> as[] = {Slice<i32>::from(a1), Slice<i32>::from(a2)};
/// Vec<i32> v = Slice<Slice<i32>>::from(as).concat();
/// // TODO: sus::check(v == sus::vec(1_i32, 2_i32, 3_i32,
/// 4_i32).construct()); sus::check(v[0u] == 1); sus::check(v[1u] == 2);
/// sus::check(v[2u] == 3);
/// sus::check(v[3u] == 4);
/// ```
///
constexpr auto concat() const& noexcept
  requires(::sus::containers::Concat<T>)
{
  const auto length = _len_expr;
  ::sus::num::usize cap;
  for (::sus::num::usize i; i < length; i += 1u) {
    cap += get_unchecked(::sus::marker::unsafe_fn, i).len();
  }
  using Container = typename T::ConcatOutputType;
  auto out = Container::with_capacity(cap);
  for (::sus::num::usize i; i < length; i += 1u) {
    get_unchecked(::sus::marker::unsafe_fn, i).concat_into(out);
  }
  return out;
}

/// Concatenates a clone of each element in the slice into `vec`.
///
/// This method exists to satisfy `sus::containers::Concat<Slice<T>>`, for
/// concat() to append the elements in each Slice onto `vec`.
///
constexpr void concat_into(ConcatOutputType& vec) const& noexcept
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
  const auto length = _len_expr;
  const auto* p = _ptr_expr;
  for (::sus::num::usize i; i < length; i += 1u) {
    if (*(p + i) == x) return true;
  }
  return false;
}

/// Returns a reference to the element at position `i` in the Slice.
///
/// # Panics
/// If the index `i` is beyond the end of the slice, the function will panic.
/// #[doc.overloads=slice.index.usize]
///
constexpr inline const T& operator[](::sus::num::usize i) const& noexcept {
  ::sus::check(i < _len_expr);
  return _ptr_expr[i.primitive_value];
}

#if _delete_rvalue
constexpr inline const T& operator[](::sus::num::usize i) && = delete;
#endif

/// Returns a const reference to the element at index `i`, or `None` if
/// `i` is beyond the end of the Slice.
///
constexpr Option<const T&> get(::sus::num::usize i) const& noexcept {
  if (i < _len_expr) [[likely]]
    return Option<const T&>::some(_ptr_expr[i.primitive_value]);
  else
    return Option<const T&>::none();
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
///
constexpr inline const T& get_unchecked(::sus::marker::UnsafeFnMarker,
                                        ::sus::num::usize i) const& noexcept {
  sus_assume(::sus::marker::unsafe_fn,
             i.primitive_value < _len_expr.primitive_value);
  return _ptr_expr[i.primitive_value];
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
/// As such, `r.get_range(Range(0u, r._len_expr()))` returns a slice over the
/// full set of elements in `r`.
///
/// # Panics
/// If the Range would otherwise contain an element that is out of bounds,
/// the function will panic.
/// #[doc.overloads=slice.index.range]
///
constexpr inline Slice<T> operator[](
    const ::sus::ops::RangeBounds<::sus::num::usize> auto range)
    const& noexcept {
  const ::sus::num::usize rstart = range.start_bound().unwrap_or(0u);
  const ::sus::num::usize rend = range.end_bound().unwrap_or(_len_expr);
  const ::sus::num::usize rlen = rend >= rstart ? rend - rstart : 0u;
  ::sus::check(rlen <= _len_expr);
  /* Avoid underflow below. */ /* We allow rstart == _len_expr
                                * && rend == _len_expr, which
                                * returns an     \ empty slice.
                                */
  ::sus::check(rstart <= _len_expr && rstart <= _len_expr - rlen);
  return Slice<T>(_ptr_expr + rstart, rlen);
}

#if _delete_rvalue
constexpr inline Slice<T> operator[](
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
/// Returns None if the Range would otherwise contain an element that is out
/// of bounds.
///
constexpr Option<Slice<T>> get_range(
    const ::sus::ops::RangeBounds<::sus::num::usize> auto range)
    const& noexcept {
  const ::sus::num::usize rstart = range.start_bound().unwrap_or(0u);
  const ::sus::num::usize rend = range.end_bound().unwrap_or(_len_expr);
  const ::sus::num::usize rlen = rend >= rstart ? rend - rstart : 0u;
  if (rlen > _len_expr)
    return sus::none(); /* Avoid underflow below. */ /* We allow rstart ==
                                                      * _len_expr && rend ==
                                                      * _len_expr, which returns
                                                      * an     \ empty slice. */
  if (rstart > _len_expr || rstart > _len_expr - rlen) return sus::none();
  return Option<Slice<T>>::some(Slice(_ptr_expr + rstart, rlen));
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
///
constexpr Slice<T> get_range_unchecked(
    ::sus::marker::UnsafeFnMarker,
    const ::sus::ops::RangeBounds<::sus::num::usize> auto range)
    const& noexcept {
  const ::sus::num::usize rstart = range.start_bound().unwrap_or(0u);
  const ::sus::num::usize rend = range.end_bound().unwrap_or(_len_expr);
  const ::sus::num::usize rlen = rend >= rstart ? rend - rstart : 0u;
  return Slice<T>(_ptr_expr + rstart, rlen);
}

#if _delete_rvalue
constexpr Slice<T> get_range_unchecked(
    ::sus::marker::UnsafeFnMarker,
    const ::sus::ops::RangeBounds<::sus::num::usize> auto range) && = delete;
#endif

/// Returns an iterator over all the elements in the slice, visited in the
/// same order they appear in the slice. The iterator gives const access to
/// each element.
///
constexpr SliceIter<const T&> iter() const& noexcept {
  return SliceIter<const T&>::with(_ptr_expr, _len_expr);
}

#if _delete_rvalue
constexpr SliceIter<const T&> iter() && = delete;
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
///
constexpr ::sus::Tuple<Slice<T>, Slice<T>> split_at_unchecked(
    ::sus::marker::UnsafeFnMarker, ::sus::num::usize mid) const& noexcept {
  const ::sus::num::usize length = _len_expr;

  /* SAFETY: Caller has to check that `0 <= mid <= _len_expr` */
  sus_assume(::sus::marker::unsafe_fn,
             mid.primitive_value <= length.primitive_value);
  return ::sus::Tuple<Slice<T>, Slice<T>>::with(
      Slice<T>::from_raw_parts(::sus::marker::unsafe_fn, _ptr_expr, mid),
      Slice<T>::from_raw_parts(::sus::marker::unsafe_fn, _ptr_expr + mid,
                               length - mid));
}

#if _delete_rvalue
constexpr ::sus::Tuple<Slice<T>, Slice<T>> split_at_unchecked(
    ::sus::marker::UnsafeFnMarker, ::sus::num::usize mid) && = delete;
#endif

/// Constructs a `Vec<T>` by cloning each value in the Slice. */
Vec<T> to_vec() const& noexcept
  requires(::sus::mem::Clone<T>);

#undef _mut_ref
