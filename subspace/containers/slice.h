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

#include <algorithm>  // Replace std::sort.

#include "subspace/assertions/check.h"
#include "subspace/construct/default.h"
#include "subspace/containers/concat.h"
#include "subspace/containers/iterators/chunks.h"
#include "subspace/containers/iterators/slice_iter.h"
#include "subspace/containers/join.h"
#include "subspace/fn/fn_concepts.h"
#include "subspace/fn/fn_ref.h"
#include "subspace/iter/iterator_defn.h"
#include "subspace/macros/assume.h"
#include "subspace/macros/lifetimebound.h"
#include "subspace/macros/pure.h"
#include "subspace/marker/unsafe.h"
#include "subspace/mem/clone.h"
#include "subspace/mem/move.h"
#include "subspace/mem/swap.h"
#include "subspace/num/unsigned_integer.h"
#include "subspace/ops/eq.h"
#include "subspace/ops/ord.h"
#include "subspace/ops/range.h"
#include "subspace/option/option.h"
#include "subspace/ptr/copy.h"
#include "subspace/result/result.h"
#include "subspace/tuple/tuple.h"

// TODO: sort_by_key()
// TODO: sort_by_cached_key()
// TODO: sort_unstable_by_key()

namespace sus::containers {

template <class T>
class Vec;

/// A dynamically-sized const view into a contiguous sequence of objects of type
/// `const T`.
///
/// Contiguous here means that elements are laid out so that every element is
/// the same distance from its neighbors, where there are
/// `sus::mem::size_of<T>()` many bytes between the start of each element.
///
/// Slices are a view into a block of memory represented as a pointer and a
/// length.
template <class T>
class [[sus_trivial_abi]] Slice final {
 public:
  static_assert(!std::is_reference_v<T>,
                "Slice holds references, so the type parameter can not also be "
                "a reference");
  static_assert(!std::is_const_v<T>,
                "Slice inner type should not be const, it provides const "
                "access to the slice regardless.");

  /// Constructs an empty Slice, which has no elements.
  constexpr Slice() : Slice(nullptr, 0_usize) {}

  /// Constructs a slice from its raw parts.
  ///
  /// # Safety
  /// The following must be upheld or Undefined Behaviour may result:
  /// * The `len` must be no more than the number of elements in the allocation
  ///   at and after the position of `data`.
  /// * The pointer `data` must be a valid pointer to an allocation, not a
  ///   dangling pointer, at any point during the Slice's lifetime. This must
  ///   be true even if `len` is 0.
  ///
  /// In some other langages such as Rust, the slice may hold an invalid pointer
  /// when the length is zero. But `Slice` must not hold a dangling pointer.
  /// This avoids addition on a dangling pointer, which is Undefined Behaviour
  /// in C++, and avoids runtime checks for `length == 0`. Care must be applied
  /// when converting slices between languages as a result.
  sus_pure static constexpr inline Slice from_raw_parts(
      ::sus::marker::UnsafeFnMarker, const T* data sus_lifetimebound,
      ::sus::usize len) noexcept {
    ::sus::check(size_t{len} <= size_t{isize::MAX_PRIMITIVE});
    // We strip the `const` off `data`, however only const access is provided
    // through this class. This is done so that mutable types can compose Slice
    // and store a mutable pointer.
    return Slice(const_cast<T*>(data), len);
  }

  /// sus::construct::From<T[N]> trait.
  ///
  /// Returns a Slice that refers to all elements of the `data` array.
  ///
  /// #[doc.overloads=from.array]
  template <size_t N>
    requires(N <= size_t{isize::MAX_PRIMITIVE})
  sus_pure static constexpr inline Slice from(
      const T (&data)[N] sus_lifetimebound) {
    // We strip the `const` off `data`, however only const access is provided
    // through this class. This is done so that mutable types can compose Slice
    // and store a mutable pointer.
    return Slice(const_cast<T*>(data), N);
  }

  /** Converts the slice into an iterator that consumes the slice and returns \
   * each element in the same order they appear in the slice.                 \
   */
  sus_pure constexpr SliceIter<const T&> into_iter() && noexcept {
    return SliceIter<const T&>::with(data_, len_);
  }

  /// sus::ops::Eq<Slice<U>> trait.
  template <class U>
    requires(::sus::ops::Eq<T, U>)
  friend constexpr inline bool operator==(const Slice<T>& l,
                                          const Slice<U>& r) noexcept {
    if (l.len() != r.len()) return false;
    const T* lp = l.data_;
    const U* rp = r.data_;
    const T* const endp = lp + l.len();
    while (lp != endp) {
      if (!(*lp == *rp)) return false;
      lp += 1u;
      rp += 1u;
    }
    return true;
  }

  template <class U>
    requires(!::sus::ops::Eq<T, U>)
  friend constexpr inline bool operator==(const Slice<T>& l,
                                          const Slice<U>& r) = delete;

#define _ptr_expr data_
#define _len_expr len_
#define _delete_rvalue 0
#include "__private/slice_methods.inc"
#undef _ptr_expr
#undef _len_expr
#undef _delete_rvalue

 private:
  constexpr Slice(T* data sus_lifetimebound, usize len) noexcept
      : data_(data), len_(len) {}

  friend class SliceMut<T>;
  friend class Vec<T>;

  T* data_;
  ::sus::usize len_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(data_),
                                  decltype(len_));
  // Slice does not satisfy NeverValueField because it requires that the default
  // constructor is trivial, but Slice's default constructor needs to initialize
  // its fields.
};

#define _ptr_expr data_
#define _len_expr len_
#define _delete_rvalue 0
#define Self Slice
#include "__private/slice_methods_out_of_line.inc"
#undef Self
#undef _ptr_expr
#undef _len_expr
#undef _delete_rvalue

/// A dynamically-sized mutable view into a contiguous sequence of objects of
/// type `T`.
///
/// Contiguous here means that elements are laid out so that every element is
/// the same distance from its neighbors, where there are
/// `sus::mem::size_of<T>()` many bytes between the start of each element.
///
/// Slices are a view into a block of memory represented as a pointer and a
/// length.
///
/// SliceMut can be converted to or referenced as a Slice.
template <class T>
class [[sus_trivial_abi]] SliceMut final {
 public:
  static_assert(
      !std::is_reference_v<T>,
      "SliceMut holds references, so the type parameter can not also be "
      "a reference");
  static_assert(!std::is_const_v<T>,
                "SliceMut inner type should not be const, it provides mutable "
                "access to the slice. Use Slice to represent const access.");

  /// Constructs an empty SliceMut, which has no elements.
  constexpr SliceMut() = default;

  /// Constructs a slice from its raw parts.
  ///
  /// # Safety
  /// The following must be upheld or Undefined Behaviour may result:
  /// * The `len` must be no more than the number of elements in the allocation
  ///   at and after the position of `data`.
  /// * The pointer `data` must be a valid pointer to an allocation, not a
  ///   dangling pointer, at any point during the SliceMut's lifetime. This must
  ///   be true even if `len` is 0.
  ///
  /// In some other langages such as Rust, the slice may hold an invalid pointer
  /// when the length is zero. But `SliceMut` must not hold a dangling pointer.
  /// This avoids addition on a dangling pointer, which is Undefined Behaviour
  /// in C++, and avoids runtime checks for `length == 0`. Care must be applied
  /// when converting slices between languages as a result.
  sus_pure static constexpr inline SliceMut from_raw_parts_mut(
      ::sus::marker::UnsafeFnMarker, T* data sus_lifetimebound,
      ::sus::usize len) noexcept {
    ::sus::check(size_t{len} <= size_t{isize::MAX_PRIMITIVE});
    return SliceMut(data, len);
  }

  /// sus::construct::From<T[N]> trait.
  ///
  /// Returns a SliceMut that refers to all elements of the `data` array.
  ///
  /// #[doc.overloads=from.array]
  template <size_t N>
    requires(N <= size_t{isize::MAX_PRIMITIVE})
  sus_pure static constexpr inline SliceMut from(
      T (&data)[N] sus_lifetimebound) {
    return SliceMut(data, N);
  }

  /** Converts the slice into an iterator that consumes the slice and returns \
   * each element in the same order they appear in the slice.                 \
   */
  sus_pure constexpr SliceIterMut<T&> into_iter() && noexcept {
    return SliceIterMut<T&>::with(slice_.data_, slice_.len_);
  }

  /// sus::ops::Eq<SliceMut<U>> trait.
  template <class U>
    requires(::sus::ops::Eq<T, U>)
  friend constexpr inline bool operator==(const SliceMut<T>& l,
                                          const SliceMut<U>& r) noexcept {
    return l.as_slice() == r.as_slice();
  }

  template <class U>
    requires(!::sus::ops::Eq<T, U>)
  friend constexpr inline bool operator==(const SliceMut<T>& l,
                                          const SliceMut<U>& r) = delete;

  // TODO: Impl AsRef -> Slice<T>.
  constexpr Slice<T> as_slice() const& noexcept {
    // SAFETY: The `raw_len()` is the number of elements in the Vec, and the
    // pointer is to the start of the Vec, so this Slice covers a valid range.
    return *this;
  }

  // SliceMut can be used as a Slice.
  sus_pure constexpr operator const Slice<T>&() const& { return slice_; }
  sus_pure constexpr operator Slice<T>&() & { return slice_; }
  sus_pure constexpr operator Slice<T>() && { return ::sus::move(slice_); }

#define _ptr_expr slice_.data_
#define _len_expr slice_.len_
#define _delete_rvalue 0
#include "__private/slice_methods.inc"
#include "__private/slice_mut_methods.inc"
#undef _ptr_expr
#undef _len_expr
#undef _delete_rvalue

 private:
  constexpr SliceMut(T* data, ::sus::num::usize len) : slice_(data, len) {}

  friend class Vec<T>;

  Slice<T> slice_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(slice_));
  // SliceMut does not satisfy NeverValueField because it requires that the
  // default constructor is trivial, but SliceMut's default constructor needs to
  // initialize its fields.
};

#define _ptr_expr slice_.data_
#define _len_expr slice_.len_
#define _delete_rvalue 0
#define Self SliceMut
#include "__private/slice_methods_out_of_line.inc"
#undef _ptr_expr
#undef _len_expr
#undef _delete_rvalue
#undef Self

// Implicit for-ranged loop iteration via `Slice::iter()` and
// `SliceMut::iter()`.
using ::sus::iter::__private::begin;
using ::sus::iter::__private::end;

}  // namespace sus::containers

// Promote Slice into the `sus` namespace.
namespace sus {
using ::sus::containers::Slice;
using ::sus::containers::SliceMut;
}  // namespace sus
