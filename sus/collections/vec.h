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
#include "sus/collections/collections.h"
#include "sus/collections/concat.h"
#include "sus/collections/iterators/chunks.h"
#include "sus/collections/iterators/drain.h"
#include "sus/collections/iterators/slice_iter.h"
#include "sus/collections/iterators/vec_iter.h"
#include "sus/collections/slice.h"
#include "sus/fn/fn_concepts.h"
#include "sus/iter/adaptors/by_ref.h"
#include "sus/iter/adaptors/enumerate.h"
#include "sus/iter/adaptors/take.h"
#include "sus/iter/from_iterator.h"
#include "sus/iter/into_iterator.h"
#include "sus/iter/iterator_ref.h"
#include "sus/lib/__private/forward_decl.h"
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
#include "sus/cmp/ord.h"
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
class Vec final {
  static_assert(
      !std::is_reference_v<T>,
      "Vec<T&> is invalid as Vec must hold value types. Use Vec<T*> instead.");
  static_assert(!std::is_const_v<T>,
                "`Vec<const T>` should be written `const Vec<T>`, as const "
                "applies transitively.");
  // TODO: Make configurable.
  using A = std::allocator<T>;

  // TODO: Represent these allocator requirements as our own concept?
  // Required because otherwise move assignment is immensely complicated.
  // See
  // https://stackoverflow.com/questions/27471053/example-usage-of-propagate-on-container-move-assignment
  // for how to copy/move allocators.
  static_assert(
      std::allocator_traits<A>::propagate_on_container_move_assignment::value);
  // Required to have consistent behaviour between clone_from(Vec) and
  // clone_from_slice(Slice). The latter has no allocator visible to copy.
  static_assert(
      !std::allocator_traits<A>::propagate_on_container_copy_assignment::value);

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
      : Vec(FROM_PARTS, std::allocator<T>(), sizeof...(values), nullptr,
            0_usize) {
    if constexpr (sizeof...(values) > 0u) {
      data_ = std::allocator_traits<A>::allocate(allocator_, sizeof...(values));
    }
    (..., push_with_capacity_internal(::sus::forward<Ts>(values)));
  }

  /// Creates a `Vec` with at least the specified capacity.
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
  /// # Panics
  /// Panics if the capacity exceeds `isize::MAX` bytes.
  sus_pure static constexpr Vec with_capacity(usize capacity) noexcept {
    check(::sus::mem::size_of<T>() * capacity <= ::sus::mog<usize>(isize::MAX));
    return Vec(WITH_CAPACITY, std::allocator<T>(), capacity);
  }

  /// Creates a `Vec` directly from a pointer, a capacity, and a length.
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
    return Vec(FROM_PARTS, std::allocator<T>(), capacity, ptr, length);
  }

  /// Constructs a Vec by cloning elements out of a slice.
  ///
  /// Satisfies `sus::construct::From<Slice<T>>`
  /// and `sus::construct::From<SliceMut<T>>`.
  ///
  /// #[doc.overloads=from.slice]
  static constexpr Vec from(::sus::Slice<T> slice) noexcept
    requires(sus::mem::Clone<T>)
  {
    auto v = Vec::with_capacity(slice.len());
    for (const T& t : slice) v.push_with_capacity_internal(::sus::clone(t));
    return v;
  }
  /// #[doc.overloads=from.slice]
  static constexpr Vec from(::sus::SliceMut<T> slice) noexcept
    requires(sus::mem::Clone<T>)
  {
    auto v = Vec::with_capacity(slice.len());
    for (const T& t : slice) v.push_with_capacity_internal(::sus::clone(t));
    return v;
  }

  /// Allocate a [`Vec<u8>`]($sus::collections::Vec) and fill it with a string
  /// from a `char` array.
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
      v.push_with_capacity_internal(sus::mog<uint8_t>(c));
    ::sus::check(s[N - 1] == 0);  // Null terminated.
    return v;
  }

  constexpr ~Vec() {
    // `is_alloced()` is false when Vec is moved-from.
    if (is_alloced()) free_storage();
  }

  /// Satisifes the [`Move`]($sus::mem::Move) concept.
  /// #[doc.overloads=vec.move]
  constexpr Vec(Vec&& o) noexcept
      : allocator_(::sus::move(o).allocator_),
        capacity_(::sus::mem::replace(o.capacity_, kMovedFromCapacity)),
        iter_refs_(o.iter_refs_.take_for_owner()),
        data_(::sus::mem::replace(o.data_, nullptr)),
        len_(::sus::mem::replace(o.len_, kMovedFromLen)) {
    check(!is_moved_from() && !has_iterators());
  }
  /// Satisifes the [`Move`]($sus::mem::Move) concept.
  /// #[doc.overloads=vec.move]
  constexpr Vec& operator=(Vec&& o) noexcept {
    check(!o.is_moved_from());
    check(!has_iterators());
    check(!o.has_iterators());
    if (is_alloced()) free_storage();
    allocator_ = ::sus::move(o).allocator_;
    capacity_ = ::sus::mem::replace(o.capacity_, kMovedFromCapacity);
    iter_refs_ = o.iter_refs_.take_for_owner();
    data_ = ::sus::mem::replace(o.data_, nullptr);
    len_ = ::sus::mem::replace(o.len_, kMovedFromLen);
    return *this;
  }

  /// Satisfies the [`Clone`]($sus::mem::Clone) concept.
  constexpr Vec clone() const& noexcept
    requires(::sus::mem::Clone<T>)
  {
    check(!is_moved_from());
    auto v = Vec(
        WITH_CAPACITY,
        // TODO: Why do we need to do select_on_container_copy_construction()
        // instead of just default-constructing a new allocator?
        std::allocator_traits<A>::select_on_container_copy_construction(
            allocator_),
        capacity_);
    const auto self_len = len_;
    for (usize i = self_len; i > 0u; i -= 1u) {
      std::construct_at(v.data_ + i - 1u, ::sus::clone(*(data_ + i - 1u)));
    }
    v.len_ = self_len;
    return v;
  }

  /// An optimization to reuse the existing storage for
  /// [`Clone`]($sus::mem::Clone).
  ///
  // When `propagate_on_container_copy_assignment` is true, the allocation may
  // not be able to be reused when the allocators are not equal.
  constexpr void clone_from(const Vec& source) noexcept
    requires(!std::allocator_traits<
             A>::propagate_on_container_copy_assignment::value)
  {
    check(!is_moved_from() && !has_iterators());

    // Drop anything in `this` that will not be overwritten.
    truncate(source.len());

    // len() <= source.len() due to the truncate above, so the
    // slices here are always in-bounds.
    auto [init, tail] = source.split_at(len_);

    // Reuse the contained values' allocations/resources.
    clone_from_slice(init);
    extend_from_slice(tail);
  }

  /// Removes the specified range from the vector in bulk, returning all
  /// removed elements as an iterator. If the iterator is dropped before
  /// being fully consumed, it drops the remaining removed elements.
  ///
  /// The `Vec` becomes moved-from and will panic on use while the
  /// [`Drain`]($sus::collections::Drain)
  /// iterator is in use, and will be usable again once
  /// [`Drain`]($sus::collections::Drain) is destroyed or
  /// [`Drain::keep_rest`]($sus::collections::Drain::keep_rest) is called.
  ///
  /// # Panics
  ///
  /// Panics if the starting point is greater than the end point or if
  /// the end point is greater than the length of the vector.
  constexpr Drain<T> drain(::sus::ops::RangeBounds<usize> auto range) noexcept {
    check(!is_moved_from() && !has_iterators());
    ::sus::ops::Range<usize> bounded_range =
        range.start_at(range.start_bound().unwrap_or(0u))
            .end_at(range.end_bound().unwrap_or(len_));
    return Drain<T>(::sus::move(*this), bounded_range);
  }

  /// Decomposes a `Vec` into its raw components.
  ///
  /// Returns the raw pointer to the underlying data, the length of the vector
  /// (in elements), and the allocated capacity of the data (in elements). These
  /// are the same arguments in the same order as the arguments to
  /// [`from_raw_parts`]($sus::collections::Vec::from_raw_parts).
  ///
  /// After calling this function, the caller is responsible for the memory
  /// previously managed by the `Vec`. The only way to do this is to convert the
  /// raw pointer, length, and capacity back into a `Vec` with the
  /// [`from_raw_parts`]($sus::collections::Vec::from_raw_parts) function,
  /// allowing the destructor to perform the cleanup.
  constexpr ::sus::Tuple<T*, usize, usize> into_raw_parts() && noexcept {
    check(!is_moved_from() && !has_iterators());
    return sus::tuple(::sus::mem::replace(data_, nullptr),
                      ::sus::mem::replace(len_, kMovedFromLen),
                      ::sus::mem::replace(capacity_, kMovedFromCapacity));
  }

  /// Returns the number of elements there is space allocated for in the vector.
  ///
  /// This may be larger than the number of elements present, which is returned
  /// by [`len`]($sus::collections::Vec::len).
  sus_pure constexpr inline usize capacity() const& noexcept {
    check(!is_moved_from());
    return capacity_;
  }

  /// Clears the vector, removing all values.
  ///
  /// Note that this method has no effect on the allocated capacity of the
  /// vector.
  constexpr void clear() noexcept {
    check(!is_moved_from() && !has_iterators());
    destroy_storage_objects();
    len_ = 0u;
  }

  /// Extends the `Vec` with the contents of an iterator, copying from the
  /// elements.
  ///
  /// Satisfies the [`Extend<const T&>`]($sus::iter::Extend) concept for
  /// `Vec<T>`.
  ///
  /// If `T` is [`Clone`]($sus::mem::Clone) but not [`Copy`]($sus::mem::Copy),
  ///  then the elements should be cloned explicitly by the caller (possibly
  /// through [`Iterator::cloned`]($sus::iter::IteratorBase::cloned)). Then use
  /// the [`extend`]($sus::collections::Vec::extend!vec.extend.val) (non-copy)
  /// method instead, moving the elements into the `Vec`.
  ///
  /// #[doc.overloads=vec.extend.const]
  constexpr void extend(sus::iter::IntoIterator<const T&> auto&& ii) noexcept
    requires(sus::mem::Copy<T> &&  //
             ::sus::mem::IsMoveRef<decltype(ii)>)
  {
    check(!is_moved_from() && !has_iterators());

    // Prevent mutation from other callers inside this method.
    sus::iter::IterRef ref = iter_refs_.to_iter_from_owner();

    // TODO: There's some serious improvements we can do here when the iterator
    // is over contiguous elements. See
    // https://doc.rust-lang.org/src/alloc/vec/spec_extend.rs.html
    auto&& it = sus::move(ii).into_iter();
    const usize self_len = len_;
    if constexpr (sus::iter::TrustedLen<decltype(it)>) {
      const auto [lower, upper] = it.size_hint();
      // If this fails there are more than usize elements in the iterator, but
      // the max container size is isize::MAX. We can't reserve that many so
      // panic now.
      ::sus::check_with_message(upper.is_some(), "capacity overflow");
      sus_debug_check(lower == upper.as_value());
      {
        T* ptr = reserve_internal(lower) + self_len;
        for (const T& t : it) {
          std::construct_at(ptr, t);
          ptr += 1u;
        }
      }
      // Move `len_` last so the new elements are not visible before being
      // constructed.
      len_ = self_len + lower;
    } else {
      const usize lower = it.size_hint().lower;
      if (sus::Option<const T&> first = it.next(); first.is_some()) {
        const usize r = sus::cmp::max(lower, 1_usize);
        T* ptr = reserve_internal(r) + self_len;
        std::construct_at(ptr, sus::move(first).unwrap());
        // The `1u` accounts for `first` which was already appended.
        usize inserted = 1u;
        for (const T& t : it.by_ref().take(r - 1u)) {
          std::construct_at(ptr + inserted, t);
          inserted += 1u;
        }
        // Move `len_` last so the new elements are not visible before being
        // constructed. Note that `reserve_allocated_internal` below needs the
        // correct `len_` to reserve.
        len_ = self_len + inserted;
        for (const T& t : it) {
          reserve_allocated_internal(1u);
          push_with_capacity_internal(t);
        }
      }
    }
  }

  /// Extends the `Vec` with the contents of an iterator.
  ///
  /// Satisfies the [`Extend<T>`]($sus::iter::Extend) concept for `Vec<T>`.
  ///
  /// #[doc.overloads=vec.extend.val]
  constexpr void extend(sus::iter::IntoIterator<T> auto&& ii) noexcept
    requires(::sus::mem::IsMoveRef<decltype(ii)>)
  {
    check(!is_moved_from() && !has_iterators());

    // Prevent mutation from other callers inside this method.
    sus::iter::IterRef ref = iter_refs_.to_iter_from_owner();

    // TODO: There's some serious improvements we can do here when the iterator
    // is over contiguous elements. See
    // https://doc.rust-lang.org/src/alloc/vec/spec_extend.rs.html
    auto&& it = sus::move(ii).into_iter();
    const usize self_len = len_;
    if constexpr (sus::iter::TrustedLen<decltype(it)>) {
      const auto [lower, upper] = it.size_hint();
      // If this fails there are more than usize elements in the iterator, but
      // the max container size is isize::MAX. We can't reserve that many so
      // panic now.
      ::sus::check_with_message(upper.is_some(), "capacity overflow");
      sus_debug_check(lower == upper.as_value());
      {
        T* ptr = reserve_internal(lower) + self_len;
        for (T&& t : it) {
          std::construct_at(ptr, ::sus::move(t));
          ptr += 1u;
        }
      }
      // Move `len_` last so the new elements are not visible before being
      // constructed.
      len_ = self_len + lower;
    } else {
      const usize lower = it.size_hint().lower;
      if (sus::Option<T> first = it.next(); first.is_some()) {
        const usize r = sus::cmp::max(lower, 1_usize);
        T* ptr = reserve_internal(r) + self_len;
        std::construct_at(ptr, sus::move(first).unwrap());
        // The `1u` accounts for `first` which was already appended.
        usize inserted = 1u;
        for (T&& t : it.by_ref().take(r - 1u)) {
          std::construct_at(ptr + inserted, ::sus::move(t));
          inserted += 1u;
        }
        // Move `len_` last so the new elements are not visible before being
        // constructed. Note that `reserve_allocated_internal` below needs the
        // correct `len_` to reserve.
        len_ = self_len + inserted;
        for (T&& t : it) {
          reserve_allocated_internal(1u);
          push_with_capacity_internal(::sus::move(t));
        }
      }
    }
  }

  /// Extends the Vec by cloning the contents of a slice.
  ///
  /// If `T` is [`TrivialCopy`]($sus::mem::TrivialCopy), then the copy is done
  /// by `memcpy`.
  ///
  /// # Panics
  /// If the Slice is non-empty and points into the Vec, the function will
  /// panic, as resizing the Vec would invalidate the Slice.
  constexpr void extend_from_slice(::sus::collections::Slice<T> s) noexcept
    requires(sus::mem::Clone<T>)
  {
    check(!is_moved_from() && !has_iterators());
    if (s.is_empty()) {
      return;
    }
    const auto self_len = len_;
    const auto slice_len = s.len();
    const T* slice_ptr = s.as_ptr();
    if (is_alloced()) {
      // If this check fails, the Slice aliases with the Vec, and the
      // reserve() call below would invalidate the Slice.
      //
      // TODO: Should we handle aliasing with a temp buffer?
      ::sus::check(!(slice_ptr >= data_ && slice_ptr <= data_ + self_len));
      reserve_allocated_internal(slice_len);
    } else {
      reserve_internal(slice_len);
    }
    if constexpr (sus::mem::TrivialCopy<T>) {
      ::sus::ptr::copy_nonoverlapping(::sus::marker::unsafe_fn, slice_ptr,
                                      data_ + self_len, slice_len);
      len_ += slice_len;
    } else {
      for (const T& t : s) push_with_capacity_internal(::sus::clone(t));
    }
  }

  /// Increase the capacity of the vector (the total number of elements that the
  /// vector can hold without requiring reallocation) to `cap`, if there is not
  /// already room. Does nothing if capacity is already sufficient.
  ///
  /// This is similar to [`std::vector::reserve()`](
  /// https://en.cppreference.com/w/cpp/container/vector/reserve).
  ///
  /// # Panics
  /// Panics if the new capacity exceeds `isize::MAX()` bytes.
  constexpr void grow_to_exact(usize cap) noexcept {
    check(!is_moved_from() && !has_iterators());
    reserve_exact_internal(cap - len_);
  }

  /// Removes the last element from a vector and returns it, or None if it is
  /// empty.
  constexpr Option<T> pop() noexcept {
    check(!is_moved_from() && !has_iterators());
    const auto self_len = len_;
    if (self_len > 0u) {
      auto o = Option<T>(sus::move(
          get_unchecked_mut(::sus::marker::unsafe_fn, self_len - 1u)));
      if constexpr (!std::is_trivially_destructible_v<T>)
        std::destroy_at(as_mut_ptr() + self_len - 1u);
      len_ -= 1u;
      return o;
    } else {
      return Option<T>();
    }
  }

  /// Appends an element to the back of the vector.
  ///
  /// # Panics
  ///
  /// Panics if the new capacity exceeds [`isize::MAX`]($sus::num::isize::MAX)
  /// bytes.
  ///
  /// # Implementation note
  /// Avoids use of a reference, and receives by value, to sidestep the whole
  /// issue of the reference being to something inside the vector which
  /// `reserve` then invalidates.
  constexpr void push(T t) noexcept
    requires(::sus::mem::Move<T>)
  {
    check(!is_moved_from() && !has_iterators());
    reserve_internal(1_usize);
    push_with_capacity_internal(::sus::move(t));
  }

  /// Reserves capacity for at least `additional` more elements to be inserted
  /// in the given [`Vec<T>`]($sus::collections::Vec). The collection may
  /// reserve more space to
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
    check(!is_moved_from() && !has_iterators());
    reserve_internal(additional);
  }

  /// Reserves the minimum capacity for at least `additional` more elements to
  /// be inserted in the given [`Vec<T>`]($sus::collections::Vec). Unlike
  /// reserve, this will not
  /// deliberately over-allocate to speculatively avoid frequent allocations.
  /// After calling `reserve_exact`, capacity will be greater than or equal to
  /// `len() + additional`. Does nothing if the capacity is already sufficient.
  ///
  /// Note that the allocator may give the collection more space than it
  /// requests. Therefore, capacity can not be relied upon to be precisely
  /// minimal. Prefer reserve if future insertions are expected.
  ///
  /// # Panics
  /// Panics if the new capacity exceeds `isize::MAX` bytes.
  constexpr void reserve_exact(usize additional) noexcept {
    check(!is_moved_from() && !has_iterators());
    reserve_exact_internal(additional);
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
  constexpr void set_len(::sus::marker::UnsafeFnMarker,
                         usize new_len) noexcept {
    check(!is_moved_from());
    sus_debug_check(new_len <= capacity_);
    len_ = new_len;
  }

  /// Shortens the vector, keeping the first `len` elements and dropping the
  /// rest.
  ///
  /// If `len` is greater than the vector's current length, this has no effect.
  ///
  /// The [`drain`]($sus::collections::Vec::drain) method can emulate
  /// [`truncate`]($sus::collections::Vec::truncate), but causes the excess
  /// elements to be returned instead of dropped.
  ///
  /// Note that this method has no effect on the allocated capacity of the
  /// vector.
  constexpr void truncate(usize len) noexcept {
    if (len > len_) return;
    const auto remaining_len = len_ - len;
    if constexpr (!std::is_trivially_destructible_v<T>) {
      for (usize i = remaining_len; i > 0u; i -= 1u)
        std::destroy_at(data_ + len + i - 1u);
    }
    len_ = len;
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
    check(!is_moved_from() && !has_iterators());
    reserve_internal(1_usize);
    std::construct_at(data_ + len_, ::sus::forward<Us>(args)...);
    len_ += 1u;
  }

  /// Returns a [`Slice`]($sus::collections::Slice) that references all the
  /// elements of the vector as const references.
  sus_pure constexpr Slice<T> as_slice() const& noexcept sus_lifetimebound {
    return *this;
  }
  constexpr Slice<T> as_slice() && = delete;

  /// Returns a [`SliceMut`]($sus::collections::SliceMut) that references all
  /// the elements of the vector as mutable references.
  sus_pure constexpr SliceMut<T> as_mut_slice() & noexcept sus_lifetimebound {
    return *this;
  }

  /// Consumes the `Vec` into an [`Iterator`]($sus::iter::Iterator) that will
  /// return ownership of each element in the same order they appear in the
  /// `Vec`.
  constexpr VecIntoIter<T> into_iter() && noexcept
    requires(::sus::mem::Move<T>)
  {
    check(!is_moved_from());
    return VecIntoIter<T>(::sus::move(*this));
  }

  /// Satisfies the [`Eq<Vec<T>, Vec<U>>`]($sus::cmp::Eq) concept.
  ///
  /// #[doc.overloads=vec.eq.vec]
  template <class U>
    requires(::sus::cmp::Eq<T, U>)
  friend constexpr bool operator==(const Vec<T>& l, const Vec<U>& r) noexcept {
    return l.as_slice() == r.as_slice();
  }

  template <class U>
    requires(!::sus::cmp::Eq<T, U>)
  friend constexpr bool operator==(const Vec<T>& l, const Vec<U>& r) = delete;

  /// Satisfies the [`Eq<Vec<T>, Slice<U>>`]($sus::cmp::Eq) concept.
  ///
  /// #[doc.overloads=vec.eq.slice]
  template <class U>
    requires(::sus::cmp::Eq<T, U>)
  friend constexpr bool operator==(const Vec<T>& l,
                                   const Slice<U>& r) noexcept {
    return l.as_slice() == r;
  }

  /// Satisfies the [`Eq<Vec<T>, SliceMut<U>>`]($sus::cmp::Eq) concept.
  ///
  /// #[doc.overloads=vec.eq.slicemut]
  template <class U>
    requires(::sus::cmp::Eq<T, U>)
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
    ::sus::check(i < len_);
    return *(as_ptr() + i);
  }
  /// #[doc.overloads=vec.index.usize]
  constexpr const T& operator[](::sus::num::usize i) && = delete;

  /// Returns a mutable reference to the element at position `i` in the Vec.
  ///
  /// # Panics
  /// If the index `i` is beyond the end of the Vec, the function will panic.
  /// #[doc.overloads=vec.index_mut.usize]
  sus_pure constexpr T& operator[](::sus::num::usize i) & noexcept {
    check(i < len_);
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
    const ::sus::num::usize length = len_;
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
  /// #[doc.overloads=vec.index.range]
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
    const ::sus::num::usize length = len_;
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

  /// Converts to a [`Slice<T>`]($sus::collections::Slice). A `Vec` can be used
  /// anywhere a [`Slice`]($sus::collections::Slice) is wanted.
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

  /// Converts to a [`SliceMut<T>`]($sus::collections::SliceMut). A mutable
  /// `Vec` can be used anywhere a [`SliceMut`]($sus::collections::SliceMut) is
  /// wanted.
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
  friend sus::iter::FromIteratorImpl<Vec>;

  enum FromParts { FROM_PARTS };
  constexpr Vec(FromParts, std::allocator<T> alloc, usize cap, T* ptr,
                usize len)
      : allocator_(::sus::move(alloc)),
        capacity_(cap),
        iter_refs_(sus::iter::IterRefCounter::for_owner()),
        data_(ptr),
        len_(len) {}

  enum WithCapacity { WITH_CAPACITY };
  constexpr Vec(WithCapacity, std::allocator<T> alloc, usize cap)
      : allocator_(::sus::move(alloc)),
        capacity_(0u),
        iter_refs_(sus::iter::IterRefCounter::for_owner()),
        data_(nullptr),
        len_(0u) {
    // TODO: Consider rounding up to nearest 2^N for some N? A min capacity?
    if (cap > 0u) alloc_internal_check_cap(cap);
  }

  constexpr usize apply_growth_function(usize additional) const noexcept {
    usize goal = additional + len_;
    usize cap = capacity_;
    // TODO: What is a good growth function? Steal from WTF::Vector?
    while (cap < goal) {
      cap = (cap + 1u) * 3u;
    }
    return cap;
  }

  constexpr void destroy_storage_objects() {
    if constexpr (!std::is_trivially_destructible_v<T>) {
      for (usize i = len_; i > 0u; i -= 1u) std::destroy_at(data_ + i - 1u);
    }
  }

  constexpr void free_storage() {
    destroy_storage_objects();
    std::allocator_traits<std::allocator<T>>::deallocate(allocator_, data_,
                                                         capacity_);
  }

  /// Requires that there is capacity present for `t` already, and that
  /// Vec is in a valid state to mutate.
  constexpr void push_with_capacity_internal(const T& t) noexcept {
    std::construct_at(data_ + len_, t);
    len_ += 1u;
  }
  constexpr void push_with_capacity_internal(T&& t) noexcept {
    std::construct_at(data_ + len_, ::sus::move(t));
    len_ += 1u;
  }

  /// Requires that:
  /// * Vec is in a valid state to mutate
  constexpr T* reserve_internal(usize additional) noexcept {
    T* new_data;
    if (len_ + additional > capacity_) {
      if (!is_alloced()) {
        new_data = alloc_internal_check_cap(additional);
      } else {
        new_data =
            grow_to_internal_check_cap(apply_growth_function(additional));
      }
    } else {
      new_data = data_;
    }
    return new_data;
  }

  /// Requires that:
  /// * Vec is in a valid state to mutate
  constexpr T* reserve_exact_internal(usize additional) noexcept {
    const usize cap = len_ + additional;
    T* new_data;
    if (cap > capacity_) {
      if (!is_alloced())
        new_data = alloc_internal_check_cap(cap);
      else
        new_data = grow_to_internal_check_cap(cap);
    } else {
      new_data = data_;
    }
    return new_data;
  }

  /// Requires that:
  /// * Vec is already allocated.
  /// * Vec is in a valid state to mutate
  constexpr T* reserve_allocated_internal(usize additional) noexcept {
    sus_debug_check(is_alloced());
    T* new_data;
    if (len_ + additional > capacity_) {
      // TODO: Consider rounding up to nearest 2^N for some N?
      new_data = grow_to_internal_check_cap(apply_growth_function(additional));
    } else {
      new_data = data_;
    }
    return new_data;
  }

  /// Requires that:
  /// * Vec is NOT already allocated.
  /// * Vec is in a valid state to mutate
  constexpr T* alloc_internal_check_cap(usize cap) noexcept;

  /// Requires that:
  /// * `cap` > `capacity()`
  /// * Vec is already allocated
  /// * Vec is in a valid state to mutate
  constexpr T* grow_to_internal_check_cap(usize additional) noexcept;

  /// Checks if Vec has storage allocated.
  constexpr inline bool is_alloced() const noexcept {
    return capacity_ > 0_usize;
  }

  /// Checks if Vec has been moved from.
  constexpr inline bool is_moved_from() const noexcept {
    return len_ > capacity_;
  }

  constexpr inline bool has_iterators() const noexcept {
    return iter_refs_.count_from_owner() != 0u;
  }

  /// The length is set to this value when `Vec` is moved from. It is non-zero
  /// as `is_moved_from()` returns true when `length > capacity`.
  static constexpr usize kMovedFromLen = 1_usize;
  /// The capacity is set to this value when `Vec` is moved from. It is zero to
  /// signal that the Vec is unallocated, and it is less than kMovedFromLen to
  /// signal its moved-from state.
  static constexpr usize kMovedFromCapacity = 0_usize;

  [[sus_no_unique_address]] std::allocator<T> allocator_;
  usize capacity_;
  // These are in the same order as Slice/SliceMut, and come last to make it
  // easier to reuse the same stack space.
  [[sus_no_unique_address]] ::sus::iter::IterRefCounter iter_refs_;
  T* data_;
  usize len_;

  // The allocator is not always trivially relocatable (it's not in libstdc++).
  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(allocator_),
                                           decltype(iter_refs_),
                                           decltype(capacity_), decltype(data_),
                                           decltype(len_));
};

template <class T, std::same_as<T>... Ts>
Vec(const T&, const Ts&...) -> Vec<T>;
template <class T, std::same_as<T>... Ts>
Vec(T&, Ts&...) -> Vec<T>;
template <class T, std::same_as<T>... Ts>
Vec(T&&, Ts&&...) -> Vec<T>;

#define _ptr_expr data_
#define _len_expr len_
#define _iter_refs_expr iter_refs_.to_iter_from_owner()
#define _iter_refs_view_expr iter_refs_.to_view_from_owner()
#define _delete_rvalue true
#define _self_template class T
#define _self Vec<T>
#include "__private/slice_methods_impl.inc"

template <class T>
constexpr T* Vec<T>::alloc_internal_check_cap(usize cap) noexcept {
  sus_debug_check(!is_alloced());
  ::sus::check(cap <= ::sus::mog<usize>(isize::MAX));
  T* const new_data = std::allocator_traits<A>::allocate(allocator_, cap);
  data_ = new_data;
  capacity_ = cap;
  return new_data;
}

template <class T>
constexpr T* Vec<T>::grow_to_internal_check_cap(usize cap) noexcept {
  sus_debug_check(is_alloced());
  sus_debug_check(cap > capacity_);
  ::sus::check(cap <= ::sus::mog<usize>(isize::MAX));
  T* const new_data =
      std::allocator_traits<std::allocator<T>>::allocate(allocator_, cap);
  if constexpr (::sus::mem::relocate_by_memcpy<T>) {
    if (!std::is_constant_evaluated()) {
      // SAFETY: new_t was just allocated above, so does not alias
      // with `old_t` which was the previous allocation.
      ::sus::ptr::copy_nonoverlapping(::sus::marker::unsafe_fn, data_, new_data,
                                      capacity_);
      std::allocator_traits<std::allocator<T>>::deallocate(allocator_, data_,
                                                           capacity_);
      data_ = new_data;
      capacity_ = cap;
      return new_data;
    }
  }

  for (usize i = len_; i > 0u; i -= 1u) {
    std::construct_at(new_data + i - 1u, ::sus::move(*(data_ + i - 1u)));
    if constexpr (!std::is_trivially_destructible_v<T>)
      std::destroy_at(data_ + i - 1u);
  }
  std::allocator_traits<std::allocator<T>>::deallocate(allocator_, data_,
                                                       capacity_);
  data_ = new_data;
  capacity_ = cap;
  return new_data;
}

/// Implicit for-ranged loop iteration via
/// [`Vec::iter()`]($sus:collections::Vec::iter).
using ::sus::iter::__private::begin;
/// Implicit for-ranged loop iteration via
/// [`Vec::iter()`]($sus:collections::Vec::iter).
using ::sus::iter::__private::end;

}  // namespace sus::collections

// sus::iter::FromIterator trait for Vec.
template <class T>
struct sus::iter::FromIteratorImpl<::sus::collections::Vec<T>> {
  /// Constructs a vector by taking all the elements from the iterator.
  static constexpr ::sus::collections::Vec<T> from_iter(
      ::sus::iter::IntoIterator<T> auto&& ii) noexcept
    requires(::sus::mem::Move<T> &&  //
             ::sus::mem::IsMoveRef<decltype(ii)>)
  {
    auto v = ::sus::collections::Vec<T>();
    v.extend(::sus::move(ii));
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
}  // namespace sus
