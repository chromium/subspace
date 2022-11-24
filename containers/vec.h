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

#include "assertions/check.h"
#include "containers/__private/vec_iter.h"
#include "containers/slice.h"
#include "mem/never_value.h"
#include "mem/relocate.h"
#include "mem/replace.h"
#include "mem/size_of.h"
#include "num/unsigned_integer.h"
#include "option/option.h"

namespace sus::containers {

// TODO: Invalidate/drain iterators in every mutable method.

template <::sus::mem::Moveable T>
class Vec {
  static_assert(!std::is_const_v<T>);

  // The documentation in this class assumes isize::MAX == PTRDIFF_MAX.
  static_assert(isize::MAX() == isize(PTRDIFF_MAX));

 public:
  static inline Vec with_capacity(usize cap) noexcept { return Vec(cap); }

  // sus::construct::MakeDefault trait.
  static inline constexpr Vec with_default() noexcept { return Vec(); }

  ~Vec() {
    if (is_alloced()) free(storage_);
  }

  Vec(Vec&& o) noexcept
    requires(sus::mem::Moveable<T>)
  : storage_(::sus::mem::replace_ptr(mref(o.storage_), nullish())),
    len_(::sus::mem::replace(mref(o.len_), 0_usize)),
    capacity_(::sus::mem::replace(mref(o.capacity_), 0_usize)) {}
  Vec& operator=(Vec&& o) noexcept
    requires(sus::mem::MoveableForAssign<T>)
  {
    if (is_alloced()) free(storage_);
    storage_ = ::sus::mem::replace_ptr(mref(o.storage_), nullish());
    len_ = ::sus::mem::replace(mref(o.len_), 0_usize);
    capacity_ = ::sus::mem::replace(mref(o.capacity_), 0_usize);
    return *this;
  }

  void clear() noexcept {
    if (is_alloced()) {
      free(storage_);
      len_ = 0_usize;
      capacity_ = 0_usize;
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
    if (len_ + additional <= capacity_) return;  // Nothing to do.
    grow_to_exact(apply_growth_function(additional));
  }

  /// Reserves the minimum capacity for at least additional more elements to be
  /// inserted in the given Vec<T>. Unlike reserve, this will not deliberately
  /// over-allocate to speculatively avoid frequent allocations. After calling
  /// reserve_exact, capacity will be greater than or equal to self.len() +
  /// additional. Does nothing if the capacity is already sufficient.
  ///
  /// Note that the allocator may give the collection more space than it
  /// requests. Therefore, capacity can not be relied upon to be precisely
  /// minimal. Prefer reserve if future insertions are expected.
  ///
  /// # Panics
  /// Panics if the new capacity exceeds isize::MAX bytes.
  void reserve_exact(usize additional) noexcept {
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
    if (cap <= capacity_) return;  // Nothing to do.
    const auto bytes = ::sus::mem::size_of<T>() * cap;
    check(bytes <= usize(size_t{PTRDIFF_MAX}));
    static_assert(sizeof(size_t) == sizeof(usize));
    if (!is_alloced())
      storage_ = static_cast<char*>(malloc(bytes.primitive_value));
    else
      storage_ = static_cast<char*>(realloc(storage_, bytes.primitive_value));
    capacity_ = cap;
  }

  // TODO: Clone.

  /// Returns the number of elements in the vector.
  constexpr inline usize len() const& noexcept { return len_; }

  /// Returns the number of elements there is space allocated for in the vector.
  ///
  /// This may be larger than the number of elements present, which is returned
  /// by `len()`.
  constexpr inline usize capacity() const& noexcept { return capacity_; }

  /// Appends an element to the back of the vector.
  ///
  /// # Panics
  /// Panics if the new capacity exceeds isize::MAX bytes.
  ///
  // Templated with a univeral reference to avoid temporaries. Uses
  // convertible_to<T> to accept `sus::into()` values. But doesn't use
  // sus::construct::Into<T> to avoid implicit conversions.
  template <std::convertible_to<T> U>
  void push(U&& t) noexcept {
    reserve(1_usize);
    new (as_mut_ptr() + len_.primitive_value) T(static_cast<T&&>(t));
    len_ += 1_usize;
  }

  /// Returns a const reference to the element at index `i`.
  constexpr Option<const T&> get(usize i) const& noexcept {
    if (i >= len_) [[unlikely]]
      return Option<const T&>::none();
    return Option<const T&>::some(get_unchecked(unsafe_fn, i));
  }
  constexpr Option<const T&> get(usize i) && = delete;

  /// Returns a mutable reference to the element at index `i`.
  constexpr Option<T&> get_mut(usize i) & noexcept {
    if (i >= len_) [[unlikely]]
      return Option<T&>::none();
    return Option<T&>::some(mref(get_unchecked_mut(unsafe_fn, i)));
  }

  /// Returns a const reference to the element at index `i`.
  ///
  /// # Safety
  /// The index `i` must be inside the bounds of the array or Undefined
  /// Behaviour results.
  constexpr inline const T& get_unchecked(::sus::marker::UnsafeFnMarker,
                                          usize i) const& noexcept {
    return reinterpret_cast<T*>(storage_)[i.primitive_value];
  }
  constexpr inline const T& get_unchecked(::sus::marker::UnsafeFnMarker,
                                          usize i) && = delete;

  /// Returns a mutable reference to the element at index `i`.
  ///
  /// # Safety
  /// The index `i` must be inside the bounds of the array or Undefined
  /// Behaviour results.
  constexpr inline T& get_unchecked_mut(::sus::marker::UnsafeFnMarker,
                                        usize i) & noexcept {
    return reinterpret_cast<T*>(storage_)[i.primitive_value];
  }

  constexpr inline const T& operator[](usize i) const& noexcept {
    check(i < len_);
    return get_unchecked(unsafe_fn, i);
  }
  constexpr inline const T& operator[](usize i) && = delete;

  constexpr inline T& operator[](usize i) & noexcept {
    check(i < len_);
    return get_unchecked_mut(unsafe_fn, i);
  }

  /// Returns a const pointer to the first element in the vector.
  ///
  /// # Panics
  /// Panics if the vector's capacity is zero.
  inline const T* as_ptr() const& noexcept {
    check(is_alloced());
    return reinterpret_cast<T*>(storage_);
  }
  const T* as_ptr() && = delete;

  /// Returns a mutable pointer to the first element in the vector.
  ///
  /// # Panics
  /// Panics if the vector's capacity is zero.
  inline T* as_mut_ptr() & noexcept {
    check(is_alloced());
    return reinterpret_cast<T*>(storage_);
  }

  // Returns a slice that references all the elements of the vector as const
  // references.
  constexpr Slice<const T> as_ref() const& noexcept {
    return Slice<const T>::from_raw_parts(reinterpret_cast<const T*>(storage_),
                                          len_);
  }
  constexpr Slice<const T> as_ref() && = delete;

  // Returns a slice that references all the elements of the vector as mutable
  // references.
  constexpr Slice<T> as_mut() & noexcept {
    return Slice<T>::from_raw_parts(reinterpret_cast<T*>(storage_), len_);
  }

  /// Returns an iterator over all the elements in the array, visited in the
  /// same order they appear in the array. The iterator gives const access to
  /// each element.
  constexpr ::sus::iter::Iterator<SliceIter<T>> iter() const& noexcept {
    return SliceIter<T>::with(reinterpret_cast<const T*>(storage_), len_);
  }
  constexpr ::sus::iter::Iterator<SliceIter<T>> iter() && = delete;

  /// Returns an iterator over all the elements in the array, visited in the
  /// same order they appear in the array. The iterator gives mutable access to
  /// each element.
  constexpr ::sus::iter::Iterator<SliceIterMut<T>> iter_mut() & noexcept {
    return SliceIterMut<T>::with(reinterpret_cast<T*>(storage_), len_);
  }

  /// Converts the array into an iterator that consumes the array and returns
  /// each element in the same order they appear in the array.
  constexpr ::sus::iter::Iterator<VecIntoIter<T>> into_iter() && noexcept {
    return VecIntoIter<T>::with(static_cast<Vec&&>(*this));
  }

 private:
  Vec() : storage_(nullish()), len_(0_usize), capacity_(0_usize) {}

  Vec(usize cap)
      : storage_(cap > 0_usize ? static_cast<char*>(malloc(
                                     size_t{::sus::mem::size_of<T>() * cap}))
                               : nullish()),
        len_(0_usize),
        capacity_(cap) {
    check(::sus::mem::size_of<T>() * cap <= usize(size_t{PTRDIFF_MAX}));
  }

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

  // The fields are used to encode extra data as follows:
  //
  // * `storage_` == 0: Never occurs while inside the Vec's lifetime. Instead, a
  //   nullish value that is well-aligned for T is used, but is never read
  //   outside of UB.
  static inline char* nullish() noexcept {
    return reinterpret_cast<char*>(alignof(T));
  }

  // Checks if Vec has storage allocated.
  constexpr inline bool is_alloced() const noexcept {
    return capacity_ > 0_usize;
  }

  alignas(T*) char* storage_;
  usize len_;
  usize capacity_;

  sus_class_never_value_field(unsafe_fn, Vec, storage_, nullptr);
  sus_class_trivial_relocatable_value(unsafe_fn,
                                      sus::mem::relocate_array_by_memcpy<T>);
};

}  // namespace sus::containers

// Promote Vec into the `sus` namespace.
namespace sus {
using ::sus::containers::Vec;
}

// Promote Vec into the top-level namespace.
// TODO: Make a compile-time option for this.
using ::sus::containers::Vec;
