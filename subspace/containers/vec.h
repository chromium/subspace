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
#include <stdlib.h>

#include <concepts>

#include "subspace/assertions/check.h"
#include "subspace/containers/__private/vec_marker.h"
#include "subspace/containers/iterators/vec_iter.h"
#include "subspace/containers/slice.h"
#include "subspace/fn/fn_concepts.h"
#include "subspace/iter/from_iterator.h"
#include "subspace/iter/into_iterator.h"
#include "subspace/macros/lifetimebound.h"
#include "subspace/mem/move.h"
#include "subspace/mem/relocate.h"
#include "subspace/mem/replace.h"
#include "subspace/mem/size_of.h"
#include "subspace/num/integer_concepts.h"
#include "subspace/num/unsigned_integer.h"
#include "subspace/ops/ord.h"
#include "subspace/option/option.h"
#include "subspace/tuple/tuple.h"

// TODO: sort_by_key()
// TODO: sort_by_cached_key()
// TODO: sort_unstable_by_key()

namespace sus::containers {

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
template <class T>
class Vec {
  static_assert(!std::is_const_v<T>,
                "`Vec<const T>` should be written `const Vec<T>`, as const "
                "applies transitively.");

 public:
  // sus::construct::Default trait.
  inline constexpr Vec() noexcept : Vec(nullptr, 0_usize, 0_usize) {}

  /// Creates a Vec<T> with at least the specified capacity.
  ///
  /// The vector will be able to hold at least `capacity` elements without
  /// reallocating. This method is allowed to allocate for more elements than
  /// capacity. If capacity is 0, the vector will not allocate.
  ///
  /// It is important to note that although the returned vector has the minimum
  /// capacity specified, the vector will have a zero length.
  ///
  /// # Panics
  /// Panics if the capacity exceeds `isize::MAX` bytes.
  static inline Vec with_capacity(usize capacity) noexcept {
    check(::sus::mem::size_of<T>() * capacity <= size_t{isize::MAX_PRIMITIVE});
    auto v = Vec(nullptr, 0_usize, 0_usize);
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
  static Vec from_raw_parts(::sus::marker::UnsafeFnMarker, T* ptr, usize length,
                            usize capacity) noexcept {
    return Vec(ptr, length, capacity);
  }

  /// Constructs a vector by taking all the elements from the iterator.
  ///
  /// sus::iter::FromIterator trait.
  static constexpr Vec from_iter(
      ::sus::iter::IntoIterator<T> auto into_iter) noexcept
    requires(::sus::mem::Move<T> && !std::is_reference_v<T>)
  {
    auto&& iter = sus::move(into_iter).into_iter();
    auto [lower, upper] = iter.size_hint();
    auto v = Vec::with_capacity(::sus::move(upper).unwrap_or(lower));
    for (T t : iter) v.push(::sus::move(t));
    return v;
  }

  /// sus::construct::From<Slice<const T>> trait.
  ///
  /// #[doc.overloads=from.slice.const]
  static constexpr Vec from(::sus::Slice<const T> slice) noexcept
    requires(sus::mem::Clone<T>)
  {
    auto v = Vec::with_capacity(slice.len());
    for (const T& t : slice) v.push(::sus::clone(t));
    return v;
  }

  /// sus::construct::From<Slice<T>> trait.
  ///
  /// #[doc.overloads=from.slice.mut]
  static constexpr Vec from(::sus::Slice<T> slice) noexcept
    requires(sus::mem::Clone<T>)
  {
    auto v = Vec::with_capacity(slice.len());
    for (const T& t : slice) v.push(::sus::clone(t));
    return v;
  }

  ~Vec() {
    // `is_alloced()` is false when Vec is moved-from.
    if (is_alloced()) free_storage();
  }

  Vec(Vec&& o) noexcept
      : storage_(::sus::mem::replace(mref(o.storage_), nullptr)),
        len_(::sus::mem::replace(mref(o.len_), kMovedFromLen)),
        capacity_(::sus::mem::replace(mref(o.capacity_), kMovedFromCapacity)) {
    check(!is_moved_from());
  }
  Vec& operator=(Vec&& o) noexcept {
    check(!o.is_moved_from());
    if (is_alloced()) free_storage();
    storage_ = ::sus::mem::replace(mref(o.storage_), nullptr);
    len_ = ::sus::mem::replace(mref(o.len_), kMovedFromLen);
    capacity_ = ::sus::mem::replace(mref(o.capacity_), kMovedFromCapacity);
    return *this;
  }

  Vec clone() const& noexcept
    requires(::sus::mem::Clone<T>)
  {
    check(!is_moved_from());
    auto v = Vec::with_capacity(capacity_);
    for (auto i = size_t{0}; i < len_; ++i) {
      new (v.as_mut_ptr() + i)
          T(::sus::clone(get_unchecked(::sus::marker::unsafe_fn, i)));
    }
    v.len_ = len_;
    return v;
  }

  void clone_from(const Vec& source) & noexcept
    requires(::sus::mem::Clone<T>)
  {
    check(!is_moved_from());
    check(!source.is_moved_from());
    if (&source == this) [[unlikely]]
      return;
    if (source.capacity_ == 0_usize) {
      destroy_storage_objects();
      free_storage();
      storage_ = nullptr;
      len_ = 0_usize;
      capacity_ = 0_usize;
    } else {
      grow_to_exact(source.capacity_);
      const size_t in_place_count =
          sus::ops::min(len_, source.len_).primitive_value;
      for (auto i = size_t{0}; i < in_place_count; ++i) {
        ::sus::clone_into(mref(get_unchecked_mut(::sus::marker::unsafe_fn, i)),
                          source.get_unchecked(::sus::marker::unsafe_fn, i));
      }
      for (auto i = in_place_count; i < len_; ++i) {
        get_unchecked_mut(::sus::marker::unsafe_fn, i).~T();
      }
      for (auto i = in_place_count; i < source.len_; ++i) {
        new (as_mut_ptr() + i)
            T(::sus::clone(source.get_unchecked(::sus::marker::unsafe_fn, i)));
      }
      len_ = source.len_;
    }
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
  ::sus::Tuple<T*, usize, usize> into_raw_parts() && noexcept {
    check(!is_moved_from());
    return sus::tuple(::sus::mem::replace(mref(storage_), nullptr),
                      ::sus::mem::replace(mref(len_), kMovedFromLen),
                      ::sus::mem::replace(mref(capacity_), kMovedFromCapacity));
  }

  /// Clears the vector, removing all values.
  ///
  /// Note that this method has no effect on the allocated capacity of the
  /// vector.
  void clear() noexcept {
    check(!is_moved_from());
    destroy_storage_objects();
    len_ = 0_usize;
  }

  /// Extends the Vec by cloning the contents of a slice.
  ///
  /// # Panics
  /// If the Slice is non-empty and points into the Vec, the function will
  /// panic, as resizing the Vec would invalidate the Slice.
  inline void extend_from_slice(::sus::containers::Slice<const T> s) noexcept
    requires(sus::mem::Clone<T>)
  {
    if (s.is_empty()) [[unlikely]] {
      return;
    }
    check(!is_moved_from());
    const T* slice_ptr = s.as_ptr();
    if (is_alloced()) {
      const T* vec_ptr = as_ptr();
      // If this check fails, the Slice aliases with the Vec, and the reserve()
      // call below would invalidate the Slice.
      //
      // TODO: Should we handle aliasing with a temp buffer?
      ::sus::check(!(slice_ptr >= vec_ptr && slice_ptr <= vec_ptr + len_));
    }
    reserve(s.len());
    if constexpr (sus::mem::Copy<T> &&
                  std::is_trivially_copy_constructible_v<T>) {
      memcpy(as_mut_ptr() + len_, slice_ptr,
             size_t{s.len() * ::sus::mem::size_of<T>()});
      len_ += s.len();
    } else {
      for (const T& t : s) push(::sus::clone(t));
    }
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
  /// Panics if the new capacity exceeds isize::MAX() bytes.
  void reserve(usize additional) noexcept {
    check(!is_moved_from());
    if (len_ + additional <= capacity_) return;  // Nothing to do.
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
  /// Panics if the new capacity exceeds isize::MAX bytes.
  void reserve_exact(usize additional) noexcept {
    check(!is_moved_from());
    const usize cap = len_ + additional;
    if (cap <= capacity_) return;  // Nothing to do.
    grow_to_exact(cap);
  }

  /// Increase the capacity of the vector (the total number of elements that the
  /// vector can hold without requiring reallocation) to `cap`, if there is not
  /// already room. Does nothing if capacity is already sufficient.
  ///
  /// This is similar to std::vector::reserve().
  ///
  /// # Panics
  /// Panics if the new capacity exceeds isize::MAX() bytes.
  void grow_to_exact(usize cap) noexcept {
    check(!is_moved_from());
    if (cap <= capacity_) return;  // Nothing to do.
    const auto bytes = ::sus::mem::size_of<T>() * cap;
    check(bytes <= usize(size_t{PTRDIFF_MAX}));
    if (!is_alloced()) {
      storage_ = static_cast<T*>(malloc(bytes.primitive_value));
    } else {
      if constexpr (::sus::mem::relocate_by_memcpy<T>) {
        storage_ = static_cast<T*>(realloc(storage_, bytes.primitive_value));
      } else {
        auto* const new_storage =
            static_cast<T*>(malloc(bytes.primitive_value));
        T* old_t = storage_;
        T* new_t = new_storage;
        const size_t len = size_t{len_};
        for (auto i = size_t{0}; i < len; ++i) {
          new (new_t) T(::sus::move(*old_t));
          old_t->~T();
          ++old_t;
          ++new_t;
        }
        storage_ = new_storage;
      }
    }
    capacity_ = cap;
  }

  // TODO: Clone.

  /// Returns the number of elements in the vector.
  constexpr inline usize len() const& noexcept {
    check(!is_moved_from());
    return len_;
  }

  /// Returns true if the vector has a length of 0.
  constexpr inline bool is_empty() const& noexcept {
    check(!is_moved_from());
    return len_ == 0u;
  }

  /// Returns the number of elements there is space allocated for in the vector.
  ///
  /// This may be larger than the number of elements present, which is returned
  /// by `len()`.
  constexpr inline usize capacity() const& noexcept {
    check(!is_moved_from());
    return capacity_;
  }

  /// Removes the last element from a vector and returns it, or None if it is
  /// empty.
  Option<T> pop() noexcept {
    check(!is_moved_from());
    if (len_ > 0u) {
      auto o = Option<T>::some(
          sus::move(get_unchecked_mut(::sus::marker::unsafe_fn, len_ - 1u)));
      get_unchecked_mut(::sus::marker::unsafe_fn, len_ - 1u).~T();
      len_ -= 1u;
      return o;
    } else {
      return Option<T>::none();
    }
  }

  /// Appends an element to the back of the vector.
  ///
  /// # Panics
  ///
  /// Panics if the new capacity exceeds isize::MAX bytes.
  //
  // Avoids use of a reference, and receives by value, to sidestep the whole
  // issue of the reference being to something inside the vector which
  // reserve() then invalidates.
  void push(T t) noexcept
    requires(::sus::mem::Move<T> && !std::is_reference_v<T>)
  {
    check(!is_moved_from());
    reserve(1_usize);
    new (&storage_[size_t{len_}]) T(::sus::move(t));
    len_ += 1_usize;
  }

  /// Constructs and appends an element to the back of the vector.
  ///
  /// The parameters to `emplace()` are used to construct the element. This
  /// typically works best for aggregate types, rather than types with a named
  /// static method constructor (such as `T::with_foo(foo)`).
  ///
  /// Disallows construction from a reference to `T`, as `push()` should be
  /// used in that case to avoid invalidating the input reference while
  /// constructing from it.
  ///
  /// # Panics
  ///
  /// Panics if the new capacity exceeds isize::MAX bytes.
  template <class... Us>
  void emplace(Us&&... args) noexcept
    requires(::sus::mem::Move<T> && !std::is_reference_v<T> &&
             !(sizeof...(Us) == 1u &&
               (... && std::same_as<std::decay_t<T>, std::decay_t<Us>>)))
  {
    check(!is_moved_from());
    reserve(1_usize);
    new (&storage_[size_t{len_}]) T(::sus::forward<Us>(args)...);
    len_ += 1_usize;
  }

  /// Returns a const reference to the element at index `i`.
  constexpr Option<const T&> get(usize i) const& noexcept sus_lifetimebound {
    check(!is_moved_from());
    if (i >= len_) [[unlikely]]
      return Option<const T&>::none();
    return Option<const T&>::some(get_unchecked(::sus::marker::unsafe_fn, i));
  }
  constexpr Option<const T&> get(usize i) && = delete;

  /// Returns a mutable reference to the element at index `i`.
  constexpr Option<T&> get_mut(usize i) & noexcept sus_lifetimebound {
    check(!is_moved_from());
    if (i >= len_) [[unlikely]]
      return Option<T&>::none();
    return Option<T&>::some(
        mref(get_unchecked_mut(::sus::marker::unsafe_fn, i)));
  }

  /// Returns a const reference to the element at index `i`.
  ///
  /// # Safety
  ///
  /// The index `i` must be inside the bounds of the array or Undefined
  /// Behaviour results.
  ///
  /// Additionally, the Vec must not be in a moved-from state or the pointer
  /// will be invalid and Undefined Behaviour results.
  constexpr inline const T& get_unchecked(::sus::marker::UnsafeFnMarker,
                                          usize i) const& noexcept
      sus_lifetimebound {
    return storage_[i.primitive_value];
  }
  constexpr inline const T& get_unchecked(::sus::marker::UnsafeFnMarker,
                                          usize i) && = delete;

  /// Returns a mutable reference to the element at index `i`.
  ///
  /// # Safety
  /// The index `i` must be inside the bounds of the array or Undefined
  /// Behaviour results.
  constexpr inline T& get_unchecked_mut(::sus::marker::UnsafeFnMarker,
                                        usize i) & noexcept sus_lifetimebound {
    return storage_[i.primitive_value];
  }

  /// Present a nicer error when trying to use operator[] with an `int`,
  /// since it can't convert to `usize` implicitly.
  template <::sus::num::SignedPrimitiveInteger I>
  constexpr inline const T& operator[](I) const = delete;

  constexpr inline const T& operator[](usize i) const& noexcept
      sus_lifetimebound {
    check(!is_moved_from());
    check(i < len_);
    return get_unchecked(::sus::marker::unsafe_fn, i);
  }
  constexpr inline const T& operator[](usize i) && = delete;

  constexpr inline T& operator[](usize i) & noexcept sus_lifetimebound {
    check(!is_moved_from());
    check(i < len_);
    return get_unchecked_mut(::sus::marker::unsafe_fn, i);
  }

  /// #[doc.inherit=[n]sus::[n]containers::[r]Slice::[f]sort]
  void sort() { as_mut_slice().sort(); }

  /// #[doc.inherit=[n]sus::[n]containers::[r]Slice::[f]sort_by]
  template <::sus::fn::FnMut<::sus::fn::NonVoid(const T&, const T&)> F, int&...,
            class R = std::invoke_result_t<F, const T&, const T&>>
    requires(::sus::ops::Ordering<R>)
  void sort_by(F compare) {
    as_mut_slice().sort_by(sus::move(compare));
  }

  /// #[doc.inherit=[n]sus::[n]containers::[r]Slice::[f]sort_unstable]
  void sort_unstable() { as_mut_slice().sort(); }

  /// #[doc.inherit=[n]sus::[n]containers::[r]Slice::[f]sort_unstable_by]
  template <::sus::fn::FnMut<::sus::fn::NonVoid(const T&, const T&)> F, int&...,
            class R = std::invoke_result_t<F, const T&, const T&>>
    requires(::sus::ops::Ordering<R>)
  void sort_unstable_by(F compare) {
    as_mut_slice().sort_by(sus::move(compare));
  }

  /// Returns a const pointer to the first element in the vector.
  ///
  /// # Panics
  /// Panics if the vector's capacity is zero.
  inline const T* as_ptr() const& noexcept sus_lifetimebound {
    check(!is_moved_from());
    check(is_alloced());
    return storage_;
  }
  const T* as_ptr() && = delete;

  /// Returns a mutable pointer to the first element in the vector.
  ///
  /// # Panics
  /// Panics if the vector's capacity is zero.
  inline T* as_mut_ptr() & noexcept sus_lifetimebound {
    check(!is_moved_from());
    check(is_alloced());
    return storage_;
  }

  // Returns a slice that references all the elements of the vector as const
  // references.
  constexpr Slice<const T> as_slice() const& noexcept sus_lifetimebound {
    check(!is_moved_from());
    // SAFETY: The `len_` is the number of elements in the Vec, and the pointer
    // is to the start of the Vec, so this Slice covers a valid range.
    return Slice<const T>::from_raw_parts(::sus::marker::unsafe_fn, storage_,
                                          len_);
  }
  constexpr Slice<const T> as_slice() && = delete;

  // Returns a slice that references all the elements of the vector as mutable
  // references.
  constexpr Slice<T> as_mut_slice() & noexcept sus_lifetimebound {
    check(!is_moved_from());
    // SAFETY: The `len_` is the number of elements in the Vec, and the pointer
    // is to the start of the Vec, so this Slice covers a valid range.
    return Slice<T>::from_raw_parts(::sus::marker::unsafe_fn, storage_, len_);
  }

  /// Returns an iterator over all the elements in the array, visited in the
  /// same order they appear in the array. The iterator gives const access to
  /// each element.
  constexpr SliceIter<const T&> iter() const& noexcept sus_lifetimebound {
    check(!is_moved_from());
    return SliceIter<const T&>::with(storage_, len_);
  }
  constexpr SliceIter<const T&> iter() && = delete;

  /// Returns an iterator over all the elements in the array, visited in the
  /// same order they appear in the array. The iterator gives mutable access to
  /// each element.
  constexpr SliceIterMut<T&> iter_mut() & noexcept sus_lifetimebound {
    check(!is_moved_from());
    return SliceIterMut<T&>::with(storage_, len_);
  }

  /// Converts the array into an iterator that consumes the array and returns
  /// each element in the same order they appear in the array.
  constexpr VecIntoIter<T> into_iter() && noexcept {
    check(!is_moved_from());
    return VecIntoIter<T>::with(::sus::move(*this));
  }

 private:
  Vec(T* ptr, usize len, usize cap)
      : storage_(ptr), len_(len), capacity_(cap) {}

  constexpr usize apply_growth_function(usize additional) const noexcept {
    usize goal = additional + len_;
    usize cap = capacity_;
    // TODO: What is a good growth function? Steal from WTF::Vector?
    while (cap < goal) {
      cap = (cap + 1_usize) * 3_usize;
      auto bytes = ::sus::mem::size_of<T>() * cap;
      check(bytes <= usize(size_t{PTRDIFF_MAX}));
    }
    return cap;
  }

  inline void destroy_storage_objects() {
    if constexpr (!std::is_trivially_destructible_v<T>) {
      const size_t len = size_t{len_};
      for (auto i = size_t{0}; i < len; ++i) storage_[i].~T();
    }
  }

  inline void free_storage() {
    destroy_storage_objects();
    free(storage_);
  }

  // Checks if Vec has storage allocated.
  constexpr inline bool is_alloced() const noexcept {
    return capacity_ > 0_usize;
  }

  // Checks if Vec has been moved from.
  constexpr inline bool is_moved_from() const noexcept {
    return len_ > capacity_;
  }

  static constexpr usize kMovedFromLen = 1_usize;
  static constexpr usize kMovedFromCapacity = 0_usize;

  T* storage_;
  usize len_;
  usize capacity_;

  sus_class_trivially_relocatable_if_types(::sus::marker::unsafe_fn,
                                           decltype(storage_), decltype(len_),
                                           decltype(capacity_));

  // Slice does not satisfy NeverValueField because it requires that the default
  // constructor is trivial, but Slice's default constructor needs to initialize
  // its fields.
};

// Implicit for-ranged loop iteration via `Vec::iter()`.
using ::sus::iter::__private::begin;
using ::sus::iter::__private::end;

/// Used to construct a Vec<T> with the parameters as its values.
///
/// Calling vec() produces a hint to make a Vec<T> but does not actually
/// construct Vec<T>, as the type `T` is not known here.
//
// Note: A marker type is used instead of explicitly constructing a vec
// immediately in order to avoid redundantly having to specify `T` when using
// the result of `sus::vec()` as a function argument or return value.
template <class... Ts>
  requires(sizeof...(Ts) > 0)
[[nodiscard]] inline constexpr auto vec(Ts&&... vs sus_lifetimebound) noexcept {
  return __private::VecMarker<Ts...>(
      ::sus::tuple_type::Tuple<Ts&&...>::with(::sus::forward<Ts>(vs)...));
}

}  // namespace sus::containers

// Promote Vec into the `sus` namespace.
namespace sus {
using ::sus::containers::Vec;
using ::sus::containers::vec;
}  // namespace sus
