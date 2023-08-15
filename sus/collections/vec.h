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

// IWYU pragma: private, include "sus/prelude.h"
// IWYU pragma: friend "sus/.*"
#pragma once

#include <stdint.h>
#include <stdlib.h>

#include <concepts>
#include <memory>

#include "fmt/core.h"
#include "sus/assertions/check.h"
#include "sus/assertions/debug_check.h"
#include "sus/collections/__private/vec_marker.h"
#include "sus/collections/collections.h"
#include "sus/collections/concat.h"
#include "sus/collections/iterators/chunks.h"
#include "sus/collections/iterators/drain.h"
#include "sus/collections/iterators/slice_iter.h"
#include "sus/collections/iterators/vec_iter.h"
#include "sus/collections/slice.h"
#include "sus/fn/fn_concepts.h"
#include "sus/fn/fn_ref.h"
#include "sus/iter/from_iterator.h"
#include "sus/iter/into_iterator.h"
#include "sus/iter/iterator_ref.h"
#include "sus/lib/__private/forward_decl.h"
#include "sus/macros/assume.h"
#include "sus/macros/lifetimebound.h"
#include "sus/marker/unsafe.h"
#include "sus/mem/clone.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"
#include "sus/mem/replace.h"
#include "sus/mem/size_of.h"
#include "sus/num/integer_concepts.h"
#include "sus/num/signed_integer.h"
#include "sus/num/transmogrify.h"
#include "sus/num/unsigned_integer.h"
#include "sus/ops/ord.h"
#include "sus/ops/range.h"
#include "sus/option/option.h"
#include "sus/ptr/copy.h"
#include "sus/result/result.h"
#include "sus/string/__private/any_formatter.h"
#include "sus/string/__private/format_to_stream.h"
#include "sus/tuple/tuple.h"

// TODO: sort_by_key()
// TODO: sort_by_cached_key()
// TODO: sort_unstable_by_key()

namespace sus::collections {

// TODO: Invalidate/drain iterators in every mutable method.

// TODO: Use after move of Vec is NOT allowed. Should we rely on clang-tidy
// and/or compiler warnings to check for misuse? Or should we add checks(). And
// then allow them to be disabled when you are using warnings?

/// A resizeable contiguous buffer of type `T`.
///
/// Vec requires Move for its items:
/// - They can't be references as a pointer to reference is not valid.
/// - On realloc, items need to be moved between allocations.
/// Vec requires items are not references:
/// - References can not be moved in the vector as assignment modifies the
///   pointee, and Vec does not wrap references to store them as pointers
///   (for now).
/// Vec requires items are not const:
/// - A const Vec<T> contains const values, it does not give mutable access to
///   its contents, so the const internal type would be redundant.
template <class T>
class [[sus_trivial_abi]] Vec final {
  static_assert(
      !std::is_reference_v<T>,
      "Vec<T&> is invalid as Vec must hold value types. Use Vec<T*> instead.");
  static_assert(!std::is_const_v<T>,
                "`Vec<const T>` should be written `const Vec<T>`, as const "
                "applies transitively.");

 public:
  /// Constructs a `Vec`, which constructs objects of type `T` from the given
  /// values.
  ///
  /// This constructor also satisfies `sus::construct::Default` by accepting no
  /// arguments to create an empty `Vec`.
  ///
  /// The vector will be able to hold at least the elements created from the
  /// arguments. This method is allowed to allocate for more elements than
  /// needed. If no arguments are passed, it creates an empty `Vec` and will not
  /// allocate.
  template <std::convertible_to<T>... Ts>
  explicit constexpr Vec(Ts&&... values) noexcept
      : Vec(FROM_PARTS, sizeof...(values),
            sizeof...(values) == 0u
                ? nullptr
                : std::allocator<T>().allocate(sizeof...(values)),
            0_usize) {
    (..., push(::sus::forward<Ts>(values)));
  }

  /// Creates a Vec<T> with at least the specified capacity.
  ///
  /// The vector will be able to hold at least `capacity` elements without
  /// reallocating. This method is allowed to allocate for more elements than
  /// capacity. If capacity is 0, the vector will not allocate.
  ///
  /// It is important to note that although the returned vector has the minimum
  /// capacity specified, the vector will have a zero length.
  ///
  /// A `Vec<T>` can be implicitly converted to a `Slice<T>`. If it is not
  /// const, it can also be converted to a `SliceMut<T>`.
  ///
  /// Unlike `std::vector` and `std::span`, `Vec<T>` can also convert to a
  /// `const Slice<T>&` or `Slice<T>&` (or `const SliceMut<T>` or
  /// `SliceMut<T>&`) without generating any constructor or destructor, which
  /// means smaller and faster binaries when slices are received in function
  /// parameters as const references.
  ///
  /// # Panics
  /// Panics if the capacity exceeds `isize::MAX` bytes.
  sus_pure static constexpr Vec with_capacity(usize capacity) noexcept {
    check(::sus::mem::size_of<T>() * capacity <= ::sus::mog<usize>(isize::MAX));
    auto v = Vec(FROM_PARTS, 0_usize, nullptr, 0_usize);
    // TODO: Consider rounding up to nearest 2^N for some N? A min capacity?
    v.grow_to_exact(capacity);
    return v;
  }

  /// Creates a Vec<T> directly from a pointer, a capacity, and a length.
  ///
  /// # Safety
  ///
  /// This is highly unsafe, due to the number of invariants that aren’t
  /// checked:
  ///
  /// * `ptr` must be heap allocated with the same method as Vec uses
  ///   internally, which is not currently stable. (TODO: Want our own global
  ///   allocator API.) The only safe way to get this pointer is from
  ///   `from_raw_parts()`.
  /// * `T` needs to have an alignment no more than what `ptr` was allocated
  ///   with.
  /// * The size of `T` times the `capacity` (ie. the allocated size in bytes)
  ///   needs to be the same size the pointer was allocated with.
  /// * `length` needs to be less than or equal to `capacity`.
  /// * The first `length` values must be properly initialized values of type
  ///   `T`.
  /// * The allocated size in bytes must be no larger than `isize::MAX`.
  /// * If `ptr` is null, then `length` and `capacity` must be `0_usize`, and
  ///   vice versa.
  sus_pure static constexpr Vec from_raw_parts(::sus::marker::UnsafeFnMarker,
                                               T* ptr, usize length,
                                               usize capacity) noexcept {
    return Vec(FROM_PARTS, capacity, ptr, length);
  }

  /// sus::construct::From<Slice<T>> trait.
  ///
  /// #[doc.overloads=from.slice.const]
  static constexpr Vec from(::sus::Slice<T> slice) noexcept
    requires(sus::mem::Clone<T>)
  {
    auto v = Vec::with_capacity(slice.len());
    for (const T& t : slice) v.push(::sus::clone(t));
    return v;
  }

  /// sus::construct::From<SliceMut<T>> trait.
  ///
  /// #[doc.overloads=from.slice.mut]
  static constexpr Vec from(::sus::SliceMut<T> slice) noexcept
    requires(sus::mem::Clone<T>)
  {
    auto v = Vec::with_capacity(slice.len());
    for (const T& t : slice) v.push(::sus::clone(t));
    return v;
  }

  /// Allocate a `Vec<u8>` and fill it with a string from a char array.
  ///
  /// # Panics
  /// This function expects the input string to be null-terminated, and it will
  /// panic otherwise.
  template <class C, size_t N>
    requires(std::same_as<T, u8> &&  //
             (std::same_as<C, char> || std::same_as<C, signed char> ||
              std::same_as<C, unsigned char>) &&
             N <= ::sus::mog<usize>(isize::MAX))
  static constexpr Vec from(const C (&arr)[N]) {
    auto s = sus::Slice<C>::from(arr);
    auto v = Vec::with_capacity(N - 1);
    for (auto c : s[sus::ops::RangeTo<usize>(N - 1)])
      v.push(sus::mog<uint8_t>(c));
    ::sus::check(s[N - 1] == 0);  // Null terminated.
    return v;
  }

  constexpr ~Vec() {
    // `is_alloced()` is false when Vec is moved-from.
    if (is_alloced()) free_storage();
  }

  constexpr Vec(Vec&& o) noexcept
      : capacity_(::sus::mem::replace(o.capacity_, kMovedFromCapacity)),
        iter_refs_(o.iter_refs_.take_for_owner()),
        data_(::sus::mem::replace(o.data_, nullptr)),
        len_(::sus::mem::replace(o.len_, kMovedFromLen)) {
    check(!is_moved_from());
    check(!has_iterators());
  }
  constexpr Vec& operator=(Vec&& o) noexcept {
    check(!o.is_moved_from());
    check(!has_iterators());
    check(!o.has_iterators());
    if (is_alloced()) free_storage();
    data_ = ::sus::mem::replace(o.data_, nullptr);
    len_ = ::sus::mem::replace(o.len_, kMovedFromLen);
    iter_refs_ = o.iter_refs_.take_for_owner();
    capacity_ = ::sus::mem::replace(o.capacity_, kMovedFromCapacity);
    return *this;
  }

  constexpr Vec clone() const& noexcept
    requires(::sus::mem::Clone<T>)
  {
    check(!is_moved_from());
    auto v = Vec::with_capacity(capacity_);
    const auto self_len = len();
    for (usize i; i < self_len; i += 1u) {
      std::construct_at(
          v.data_ + i,  //
          ::sus::clone(get_unchecked(::sus::marker::unsafe_fn, i)));
    }
    v.set_len(::sus::marker::unsafe_fn, self_len);
    return v;
  }

  constexpr void clone_from(const Vec& source) & noexcept
    requires(::sus::mem::Clone<T>)
  {
    check(!is_moved_from());
    check(!source.is_moved_from());
    check(!has_iterators());
    if (&source == this) [[unlikely]]
      return;
    if (source.capacity_ == 0_usize) {
      destroy_storage_objects();
      free_storage();
      data_ = nullptr;
      set_len(::sus::marker::unsafe_fn, 0_usize);
      capacity_ = 0_usize;
    } else {
      grow_to_exact(source.capacity_);

      const usize self_len = len();
      const usize source_len = source.len();
      const usize in_place_count = sus::ops::min(self_len, source_len);

      // Replace element positions that are present in both Vec.
      for (usize i; i < in_place_count; i += 1u) {
        ::sus::clone_into(*(data_ + i), *(source.data_ + i));
      }
      // Destroy element positions in self that aren't in source.
      for (usize i = in_place_count; i < self_len; i += 1u) {
        std::destroy_at(as_mut_ptr() + i);
      }
      // Append element positions that weren't in self but are in source.
      for (usize i = in_place_count; i < source_len; i += 1u) {
        std::construct_at(
            data_ + i,  //
            ::sus::clone(source.get_unchecked(::sus::marker::unsafe_fn, i)));
      }
      set_len(::sus::marker::unsafe_fn, source_len);
    }
  }

  /// Removes the specified range from the vector in bulk, returning all
  /// removed elements as an iterator. If the iterator is dropped before
  /// being fully consumed, it drops the remaining removed elements.
  ///
  /// The Vec becomes moved-from and will panic on use while the Drain iterator
  /// is in use, and will be usable again once Drain is destroyed or
  /// `Drain::keep_remain()` is called.
  ///
  /// # Panics
  ///
  /// Panics if the starting point is greater than the end point or if
  /// the end point is greater than the length of the vector.
  constexpr Drain<T> drain(::sus::ops::RangeBounds<usize> auto range) noexcept {
    check(!is_moved_from());
    check(!has_iterators());
    ::sus::ops::Range<usize> bounded_range =
        range.start_at(range.start_bound().unwrap_or(0u))
            .end_at(range.end_bound().unwrap_or(len()));
    return Drain<T>(::sus::move(*this), bounded_range);
  }

  /// Decomposes a `Vec<T>` into its raw components.
  ///
  /// Returns the raw pointer to the underlying data, the length of the vector
  /// (in elements), and the allocated capacity of the data (in elements). These
  /// are the same arguments in the same order as the arguments to
  /// `from_raw_parts()`.
  ///
  /// After calling this function, the caller is responsible for the memory
  /// previously managed by the `Vec`. The only way to do this is to convert the
  /// raw pointer, length, and capacity back into a `Vec` with the
  /// `from_raw_parts()` function, allowing the destructor to perform the
  /// cleanup.
  constexpr ::sus::Tuple<T*, usize, usize> into_raw_parts() && noexcept {
    check(!is_moved_from());
    check(!has_iterators());
    return sus::tuple(::sus::mem::replace(data_, nullptr),
                      ::sus::mem::replace(len_, kMovedFromLen),
                      ::sus::mem::replace(capacity_, kMovedFromCapacity));
  }

  /// Returns the number of elements there is space allocated for in the vector.
  ///
  /// This may be larger than the number of elements present, which is returned
  /// by `len()`.
  sus_pure constexpr inline usize capacity() const& noexcept {
    check(!is_moved_from());
    return capacity_;
  }

  /// Clears the vector, removing all values.
  ///
  /// Note that this method has no effect on the allocated capacity of the
  /// vector.
  constexpr void clear() noexcept {
    check(!is_moved_from());
    check(!has_iterators());
    destroy_storage_objects();
    set_len(::sus::marker::unsafe_fn, 0_usize);
  }

  /// Extends the `Vec` with the contents of an iterator.
  ///
  /// sus::iter::Extend<const T&> trait.
  ///
  /// If `T` is `Clone` but not `Copy`, then the elements should be cloned
  /// explicitly by the caller (possibly through `Iterator::cloned()`). Then use
  /// the `Extend<T>` concept method instead, moving the elements into the Vec.
  ///
  /// #[doc.overloads=vec.extend.const]
  constexpr void extend(sus::iter::IntoIterator<const T&> auto&& ii) noexcept
    requires(sus::mem::Copy<T> &&  //
             ::sus::mem::IsMoveRef<decltype(ii)>)
  {
    check(!is_moved_from());
    check(!has_iterators());
    // TODO: There's some serious improvements we can do here when the iterator
    // is over contiguous elements. See
    // https://doc.rust-lang.org/src/alloc/vec/spec_extend.rs.html
    auto&& it = sus::move(ii).into_iter();
    reserve(it.size_hint().lower);
    for (const T& v : ::sus::move(it)) {
      push(v);
    }
  }

  /// Extends the `Vec` with the contents of an iterator.
  ///
  /// sus::iter::Extend<T> trait.
  ///
  /// #[doc.overloads=vec.extend.val]
  constexpr void extend(sus::iter::IntoIterator<T> auto&& ii) noexcept
    requires(::sus::mem::IsMoveRef<decltype(ii)>)
  {
    check(!is_moved_from());
    check(!has_iterators());
    // TODO: There's some serious improvements we can do here when the iterator
    // is over contiguous elements. See
    // https://doc.rust-lang.org/src/alloc/vec/spec_extend.rs.html
    auto&& it = sus::move(ii).into_iter();
    reserve(it.size_hint().lower);
    for (T&& v : ::sus::move(it)) {
      push(::sus::move(v));
    }
  }

  /// Extends the Vec by cloning the contents of a slice.
  ///
  /// If `T` is `TrivialCopy`, then the copy is done by `memcpy()`.
  ///
  /// # Panics
  /// If the Slice is non-empty and points into the Vec, the function will
  /// panic, as resizing the Vec would invalidate the Slice.
  constexpr void extend_from_slice(::sus::collections::Slice<T> s) noexcept
    requires(sus::mem::Clone<T>)
  {
    check(!is_moved_from());
    check(!has_iterators());
    if (s.is_empty()) {
      return;
    }
    const auto self_len = len();
    const auto slice_len = s.len();
    const T* slice_ptr = s.as_ptr();
    if (is_alloced()) {
      const T* vec_ptr = data_;
      // If this check fails, the Slice aliases with the Vec, and the reserve()
      // call below would invalidate the Slice.
      //
      // TODO: Should we handle aliasing with a temp buffer?
      ::sus::check(!(slice_ptr >= vec_ptr && slice_ptr <= vec_ptr + self_len));
    }
    reserve(slice_len);
    if constexpr (sus::mem::TrivialCopy<T>) {
      ::sus::ptr::copy_nonoverlapping(::sus::marker::unsafe_fn, slice_ptr,
                                      data_ + self_len, slice_len);
      set_len(::sus::marker::unsafe_fn, self_len + slice_len);
    } else {
      for (const T& t : s) push(::sus::clone(t));
    }
  }

  /// Increase the capacity of the vector (the total number of elements that the
  /// vector can hold without requiring reallocation) to `cap`, if there is not
  /// already room. Does nothing if capacity is already sufficient.
  ///
  /// This is similar to std::vector::reserve().
  ///
  /// # Panics
  /// Panics if the new capacity exceeds `isize::MAX()` bytes.
  constexpr void grow_to_exact(usize cap) noexcept {
    check(!is_moved_from());
    check(!has_iterators());
    if (cap <= capacity_) return;  // Nothing to do.
    const auto bytes = ::sus::mem::size_of<T>() * cap;
    check(bytes <= ::sus::mog<usize>(isize::MAX));

    if (!is_alloced()) {
      data_ = std::allocator<T>().allocate(cap);
      capacity_ = cap;
      return;
    }

    T* new_allocation = std::allocator<T>().allocate(cap);
    T* old_t = data_;
    T* new_t = new_allocation;
    if constexpr (::sus::mem::relocate_by_memcpy<T>) {
      if (!std::is_constant_evaluated()) {
        // SAFETY: new_t was just allocated above, so does not alias with
        // `old_t` which was the previous allocation.
        ::sus::ptr::copy_nonoverlapping(::sus::marker::unsafe_fn, old_t, new_t,
                                        capacity_);
        std::allocator<T>().deallocate(data_, capacity_);
        data_ = new_allocation;
        capacity_ = cap;
        return;
      }
    }

    const ::sus::num::usize self_len = len();
    for (::sus::num::usize i; i < self_len; i += 1u) {
      std::construct_at(new_t, ::sus::move(*old_t));
      std::destroy_at(old_t);
      ++old_t;
      ++new_t;
    }
    std::allocator<T>().deallocate(data_, capacity_);
    data_ = new_allocation;
    capacity_ = cap;
  }

  /// Reserves capacity for at least `additional` more elements to be inserted
  /// in the given Vec<T>. The collection may reserve more space to
  /// speculatively avoid frequent reallocations. After calling reserve,
  /// capacity will be greater than or equal to self.len() + additional. Does
  /// nothing if capacity is already sufficient.
  ///
  /// The `grow_to_exact()` function is similar to std::vector::reserve(),
  /// taking a capacity instead of the number of elements to ensure space for.
  ///
  /// # Panics
  /// Panics if the new capacity exceeds `isize::MAX` bytes.
  constexpr void reserve(usize additional) noexcept {
    check(!is_moved_from());
    check(!has_iterators());
    if (len() + additional <= capacity_) return;  // Nothing to do.
    // TODO: Consider rounding up to nearest 2^N for some N?
    grow_to_exact(apply_growth_function(additional));
  }

  /// Reserves the minimum capacity for at least `additional` more elements to
  /// be inserted in the given Vec<T>. Unlike reserve, this will not
  /// deliberately over-allocate to speculatively avoid frequent allocations.
  /// After calling reserve_exact, capacity will be greater than or equal to
  /// self.len() + additional. Does nothing if the capacity is already
  /// sufficient.
  ///
  /// Note that the allocator may give the collection more space than it
  /// requests. Therefore, capacity can not be relied upon to be precisely
  /// minimal. Prefer reserve if future insertions are expected.
  ///
  /// # Panics
  /// Panics if the new capacity exceeds `isize::MAX` bytes.
  constexpr void reserve_exact(usize additional) noexcept {
    check(!is_moved_from());
    check(!has_iterators());
    const usize cap = len() + additional;
    if (cap <= capacity_) return;  // Nothing to do.
    grow_to_exact(cap);
  }

  /// Forces the length of the vector to new_len.
  ///
  /// This is a low-level operation that maintains none of the normal invariants
  /// of the type. Normally changing the length of a vector is done using one of
  /// the safe operations instead, such as `truncate()`, `resize()`, `extend()`,
  /// or `clear()`.
  ///
  /// # Safety
  /// * `new_len` must be less than or equal to `capacity()`.
  /// * The elements at `old_len..new_len` must be constructed before or after
  ///   the call.
  /// * The elements at `new_len..old_len` must be destructed before or after
  ///   the call.
  constexpr void set_len(::sus::marker::UnsafeFnMarker, usize new_len) {
    check(!is_moved_from());
    sus_debug_check(new_len <= capacity_);
    len_ = new_len;
  }

  /// Removes the last element from a vector and returns it, or None if it is
  /// empty.
  constexpr Option<T> pop() noexcept {
    check(!is_moved_from());
    check(!has_iterators());
    const auto self_len = len();
    if (self_len > 0u) {
      auto o = Option<T>::with(sus::move(
          get_unchecked_mut(::sus::marker::unsafe_fn, self_len - 1u)));
      std::destroy_at(as_mut_ptr() + self_len - 1u);
      set_len(::sus::marker::unsafe_fn, self_len - 1u);
      return o;
    } else {
      return Option<T>();
    }
  }

  /// Appends an element to the back of the vector.
  ///
  /// # Panics
  ///
  /// Panics if the new capacity exceeds `isize::MAX` bytes.
  //
  // Avoids use of a reference, and receives by value, to sidestep the whole
  // issue of the reference being to something inside the vector which
  // reserve() then invalidates.
  constexpr void push(T t) noexcept
    requires(::sus::mem::Move<T>)
  {
    check(!is_moved_from());
    check(!has_iterators());
    reserve(1_usize);
    const auto self_len = len();
    std::construct_at(data_ + self_len, ::sus::move(t));
    set_len(::sus::marker::unsafe_fn, self_len + 1_usize);
  }

  /// Constructs and appends an element to the back of the vector.
  ///
  /// The parameters to `emplace()` are used to construct the element. This
  /// typically works best for aggregate types, rather than types with a named
  /// static method constructor (such as `T::with_foo(foo)`). Prefer to use
  /// `push()` for most cases.
  ///
  /// Disallows construction from a reference to `T`, as `push()` should be
  /// used in that case to avoid invalidating the input reference while
  /// constructing from it.
  ///
  /// # Panics
  ///
  /// Panics if the new capacity exceeds `isize::MAX` bytes.
  template <class... Us>
  constexpr void emplace(Us&&... args) noexcept
    requires(::sus::mem::Move<T> &&
             !(sizeof...(Us) == 1u &&
               (... && std::same_as<std::decay_t<T>, std::decay_t<Us>>)))
  {
    check(!is_moved_from());
    check(!has_iterators());
    reserve(1_usize);
    const auto self_len = len();
    std::construct_at(data_ + self_len, ::sus::forward<Us>(args)...);
    set_len(::sus::marker::unsafe_fn, self_len + 1_usize);
  }

  // Returns a slice that references all the elements of the vector as const
  // references.
  sus_pure constexpr Slice<T> as_slice() const& noexcept sus_lifetimebound {
    return *this;
  }
  constexpr Slice<T> as_slice() && = delete;

  // Returns a slice that references all the elements of the vector as mutable
  // references.
  sus_pure constexpr SliceMut<T> as_mut_slice() & noexcept sus_lifetimebound {
    return *this;
  }

  /// Consumes the Vec into an iterator that will return each element in the
  /// same order they appear in the Vec.
  constexpr VecIntoIter<T> into_iter() && noexcept
    requires(::sus::mem::Move<T>)
  {
    check(!is_moved_from());
    return VecIntoIter<T>::with(::sus::move(*this));
  }

  /// sus::ops::Eq<Vec<T>, Vec<U>> trait.
  ///
  /// #[doc.overloads=vec.eq.vec]
  template <class U>
    requires(::sus::ops::Eq<T, U>)
  friend constexpr bool operator==(const Vec<T>& l, const Vec<U>& r) noexcept {
    return l.as_slice() == r.as_slice();
  }

  template <class U>
    requires(!::sus::ops::Eq<T, U>)
  friend constexpr bool operator==(const Vec<T>& l, const Vec<U>& r) = delete;

  /// sus::ops::Eq<<Vec<T>, Slice<U>> trait.
  ///
  /// #[doc.overloads=vec.eq.slice]
  template <class U>
    requires(::sus::ops::Eq<T, U>)
  friend constexpr bool operator==(const Vec<T>& l,
                                   const Slice<U>& r) noexcept {
    return l.as_slice() == r;
  }

  /// sus::ops::Eq<<Vec<T>, SliceMut<U>> trait.
  ///
  /// #[doc.overloads=vec.eq.slicemut]
  template <class U>
    requires(::sus::ops::Eq<T, U>)
  friend constexpr bool operator==(const Vec<T>& l,
                                   const SliceMut<U>& r) noexcept {
    return l.as_slice() == r.as_slice();
  }

  /// Returns a reference to the element at position `i` in the Vec.
  ///
  /// # Panics
  /// If the index `i` is beyond the end of the Vec, the function will panic.
  /// #[doc.overloads=vec.index.usize]
  sus_pure constexpr const T& operator[](::sus::num::usize i) const& noexcept {
    ::sus::check(i < len());
    return *(as_ptr() + i);
  }
  constexpr const T& operator[](::sus::num::usize i) && = delete;

  /// Returns a mutable reference to the element at position `i` in the Vec.
  ///
  /// # Panics
  /// If the index `i` is beyond the end of the Vec, the function will panic.
  /// #[doc.overloads=vec.index_mut.usize]
  sus_pure constexpr T& operator[](::sus::num::usize i) & noexcept {
    check(i < len());
    return *(as_mut_ptr() + i);
  }

  /// Returns a subslice which contains elements in `range`, which specifies a
  /// start and a length.
  ///
  /// The start is the index of the first element to be returned in the
  /// subslice, and the length is the number of elements in the output slice.
  /// As such, `r.get_range(Range(0u, r.len()))` returns a slice over the
  /// full set of elements in `r`.
  ///
  /// # Panics
  /// If the Range would otherwise contain an element that is out of bounds,
  /// the function will panic.
  /// #[doc.overloads=vec.index.range]
  sus_pure constexpr Slice<T> operator[](
      const ::sus::ops::RangeBounds<::sus::num::usize> auto range)
      const& noexcept {
    const ::sus::num::usize length = len();
    const ::sus::num::usize rstart = range.start_bound().unwrap_or(0u);
    const ::sus::num::usize rend = range.end_bound().unwrap_or(length);
    const ::sus::num::usize rlen = rend >= rstart ? rend - rstart : 0u;
    ::sus::check(rlen <= length);  // Avoid underflow below.
    // We allow rstart == len() && rend == len(), which returns an empty
    // slice.
    ::sus::check(rstart <= length && rstart <= length - rlen);
    return Slice<T>::from_raw_collection(::sus::marker::unsafe_fn,
                                         iter_refs_.to_view_from_owner(),
                                         as_ptr() + rstart, rlen);
  }
  constexpr Slice<T> operator[](
      const ::sus::ops::RangeBounds<::sus::num::usize> auto range) && = delete;

  /// Returns a mutable subslice which contains elements in `range`, which
  /// specifies a start and a length.
  ///
  /// The start is the index of the first element to be returned in the
  /// subslice, and the length is the number of elements in the output slice.
  /// As such, `r.get_range(Range(0u, r.len()))` returns a slice over the
  /// full set of elements in `r`.
  ///
  /// # Panics
  /// If the Range would otherwise contain an element that is out of bounds,
  /// the function will panic.
  /// #[doc.overloads=vec.index_mut.range]
  sus_pure constexpr SliceMut<T> operator[](
      const ::sus::ops::RangeBounds<::sus::num::usize> auto range) & noexcept {
    const ::sus::num::usize length = len();
    const ::sus::num::usize rstart = range.start_bound().unwrap_or(0u);
    const ::sus::num::usize rend = range.end_bound().unwrap_or(length);
    const ::sus::num::usize rlen = rend >= rstart ? rend - rstart : 0u;
    ::sus::check(rlen <= length);  // Avoid underflow below.
    // We allow rstart == len() && rend == len(), which returns an empty
    // slice.
    ::sus::check(rstart <= length && rstart <= length - rlen);
    return SliceMut<T>::from_raw_collection_mut(::sus::marker::unsafe_fn,
                                                iter_refs_.to_view_from_owner(),
                                                as_mut_ptr() + rstart, rlen);
  }

  // Const Vec can be used as a Slice.
  sus_pure constexpr operator Slice<T>() const& noexcept {
    check(!is_moved_from());
    return Slice<T>::from_raw_collection(
        ::sus::marker::unsafe_fn, iter_refs_.to_view_from_owner(), data_, len_);
  }
  sus_pure constexpr operator Slice<T>() && = delete;
  sus_pure constexpr operator Slice<T>() & noexcept {
    check(!is_moved_from());
    return Slice<T>::from_raw_collection(
        ::sus::marker::unsafe_fn, iter_refs_.to_view_from_owner(), data_, len_);
  }

  // Mutable Vec can be used as a SliceMut.
  sus_pure constexpr operator SliceMut<T>() & noexcept {
    check(!is_moved_from());
    return SliceMut<T>::from_raw_collection_mut(
        ::sus::marker::unsafe_fn, iter_refs_.to_view_from_owner(), data_, len_);
  }

#define _ptr_expr data_
#define _len_expr len_
#define _iter_refs_expr iter_refs_.to_iter_from_owner()
#define _iter_refs_view_expr iter_refs_.to_view_from_owner()
#define _delete_rvalue true
#include "__private/slice_methods.inc"
#define _ptr_expr data_
#define _len_expr len_
#define _iter_refs_expr iter_refs_.to_iter_from_owner()
#define _iter_refs_view_expr iter_refs_.to_view_from_owner()
#define _delete_rvalue true
#include "__private/slice_mut_methods.inc"

 private:
  enum FromParts { FROM_PARTS };
  constexpr Vec(FromParts, usize cap, T* ptr, usize len)
      : capacity_(cap),
        iter_refs_(sus::iter::IterRefCounter::for_owner()),
        data_(ptr),
        len_(len) {}

  constexpr usize apply_growth_function(usize additional) const noexcept {
    usize goal = additional + len();
    usize cap = capacity_;
    // TODO: What is a good growth function? Steal from WTF::Vector?
    while (cap < goal) {
      cap = (cap + 1u) * 3u;
      auto bytes = ::sus::mem::size_of<T>() * cap;
      check(bytes <= ::sus::mog<usize>(isize::MAX));
    }
    return cap;
  }

  constexpr void destroy_storage_objects() {
    if constexpr (!std::is_trivially_destructible_v<T>) {
      const auto self_len = len();
      for (::sus::num::usize i; i < self_len; i += 1u)
        std::destroy_at(data_ + i);
    }
  }

  constexpr void free_storage() {
    destroy_storage_objects();
    std::allocator<T>().deallocate(data_, capacity_);
  }

  /// Checks if Vec has storage allocated.
  constexpr inline bool is_alloced() const noexcept {
    return capacity_ > 0_usize;
  }

  /// Checks if Vec has been moved from.
  constexpr inline bool is_moved_from() const noexcept {
    return len() > capacity_;
  }

  constexpr inline bool has_iterators() const noexcept {
    return iter_refs_.count_from_owner() != 0u;
  }

  /// The length is set to this value when Vec is moved from. It is non-zero as
  /// `is_moved_from()` returns true when `length > capacity`.
  static constexpr usize kMovedFromLen = 1_usize;
  /// The capacity is set to this value when Vec is moved from. It is zero to
  /// signal that the Vec is unallocated, and it is less than kMovedFromLen to
  /// signal its moved-from state.
  static constexpr usize kMovedFromCapacity = 0_usize;

  usize capacity_;
  // These are in the same order as Slice/SliceMut, and come last to make it
  // easier to reuse the same stack space.
  [[sus_no_unique_address]] ::sus::iter::IterRefCounter iter_refs_;
  T* data_;
  usize len_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn,
                                  decltype(iter_refs_), decltype(capacity_),
                                  decltype(data_), decltype(len_));
};

#define _ptr_expr data_
#define _len_expr len_
#define _iter_refs_expr iter_refs_.to_iter_from_owner()
#define _iter_refs_view_expr iter_refs_.to_view_from_owner()
#define _delete_rvalue true
#define _self_template class T
#define _self Vec<T>
#include "__private/slice_methods_impl.inc"

// Implicit for-ranged loop iteration via `Vec::iter()`.
using ::sus::iter::__private::begin;
using ::sus::iter::__private::end;

/// Used to construct a `Vec<T>` with the parameters as its values.
///
/// Calling `vec()` produces a hint to make a `Vec<T>` but does not actually
/// construct `Vec<T>`, as the type `T` is not known here. The `Vec<T>` will be
/// constructed once the `T` is deduced by converting the marker to the `Vec<T>`
/// type.
///
/// Passing no arguments will produce an empty `Vec<T>`.
//
// Note: A marker type is used instead of explicitly constructing a vec
// immediately in order to avoid redundantly having to specify `T` when using
// the result of `sus::vec()` as a function argument or return value.
template <class... Ts>
[[nodiscard]] inline constexpr auto vec(Ts&&... vs sus_lifetimebound) noexcept {
  if constexpr (sizeof...(Ts) > 0) {
    return __private::VecMarker<Ts...>(
        ::sus::tuple_type::Tuple<Ts&&...>::with(::sus::forward<Ts>(vs)...));
  } else {
    return __private::VecEmptyMarker();
  }
}

}  // namespace sus::collections

// sus::iter::FromIterator trait for Vec.
template <class T>
struct sus::iter::FromIteratorImpl<::sus::collections::Vec<T>> {
  /// Constructs a vector by taking all the elements from the iterator.
  static constexpr ::sus::collections::Vec<T> from_iter(
      ::sus::iter::IntoIterator<T> auto&& into_iter) noexcept
    requires(::sus::mem::Move<T>)
  {
    auto&& iter = sus::move(into_iter).into_iter();
    auto [lower, upper] = iter.size_hint();
    auto v = ::sus::collections::Vec<T>::with_capacity(
        ::sus::move(upper).unwrap_or(lower));
    for (T t : iter) v.push(::sus::move(t));
    return v;
  }
};

// fmt support.
template <class T, class Char>
struct fmt::formatter<::sus::collections::Vec<T>, Char> {
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return underlying_.parse(ctx);
  }

  template <class FormatContext>
  constexpr auto format(const ::sus::collections::Vec<T>& vec,
                        FormatContext& ctx) const {
    auto out = ctx.out();
    out = fmt::format_to(out, "[");
    for (::sus::num::usize i; i < vec.len(); i += 1u) {
      if (i > 0u) out = fmt::format_to(out, ", ");
      ctx.advance_to(out);
      out = underlying_.format(vec[i], ctx);
    }
    return fmt::format_to(out, "]");
  }

 private:
  ::sus::string::__private::AnyFormatter<T, Char> underlying_;
};

// Stream support.
sus__format_to_stream(sus::collections, Vec, T);

// Promote Vec into the `sus` namespace.
namespace sus {
using ::sus::collections::Vec;
using ::sus::collections::vec;
}  // namespace sus