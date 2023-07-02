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

#include "fmt/core.h"
#include "subspace/assertions/check.h"
#include "subspace/assertions/debug_check.h"
#include "subspace/construct/default.h"
#include "subspace/containers/__private/sort.h"
#include "subspace/containers/concat.h"
#include "subspace/containers/iterators/chunks.h"
#include "subspace/containers/iterators/slice_iter.h"
#include "subspace/containers/iterators/split.h"
#include "subspace/containers/iterators/windows.h"
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
#include "subspace/num/signed_integer.h"
#include "subspace/num/unsigned_integer.h"
#include "subspace/ops/eq.h"
#include "subspace/ops/ord.h"
#include "subspace/ops/range.h"
#include "subspace/option/option.h"
#include "subspace/ptr/copy.h"
#include "subspace/ptr/swap.h"
#include "subspace/result/result.h"
#include "subspace/string/__private/any_formatter.h"
#include "subspace/string/__private/format_to_stream.h"
#include "subspace/tuple/tuple.h"

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
///
/// # Slices as function parameters
///
/// Receiving Slice as a const reference `const Slice<T>&` allows receiving a
/// `Slice`, `SliceMut`, or `Vec`, without any constructor or destructor being
/// generated at all. This is preferable to receiving them by value which would
/// force a constructor.
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
    ::sus::check(size_t{len} <= size_t{isize::MAX});
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
    requires(N <= size_t{isize::MAX})
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
  ///
  /// #[doc.overloads=slice.eq]
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

  /// Returns a reference to the element at position `i` in the Slice.
  ///
  /// # Panics
  /// If the index `i` is beyond the end of the slice, the function will panic.
  /// #[doc.overloads=slice.index.usize]
  sus_pure constexpr inline const T& operator[](
      ::sus::num::usize i) const& noexcept {
    ::sus::check(i < len());
    return *(as_ptr() + i);
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
  /// #[doc.overloads=slice.index.range]
  sus_pure constexpr inline Slice<T> operator[](
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
    return Slice<T>::from_raw_parts(::sus::marker::unsafe_fn, as_ptr() + rstart,
                                    rlen);
  }

#define _ptr_expr data_
#define _len_expr len_
#define _delete_rvalue false
#include "__private/slice_methods.inc"

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
#define _delete_rvalue false
#define _self Slice
#include "__private/slice_methods_impl.inc"

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
/// A `SliceMut<T>` can be implicitly converted to a `Slice<T>`.
///
/// # Slices as function parameters
///
/// Receiving SliceMut as a const reference `const SliceMut<T>&` allows
/// receiving a `Slice`, `SliceMut`, or `Vec`, without any constructor or
/// destructor being generated at all. This is preferable to receiving them by
/// value which would force a constructor.
///
/// `SliceMut<T>` encodes mutability of the underlying `T` objects in its type,
/// and it is not an owner of those `T` objects, so its constness is not
/// important as a correctness signal, and all its operations are also available
/// when const. This makes receiving it in function parameters as a `const
/// SliceMut<T>&` plausible.
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
    ::sus::check(len <= usize::from(isize::MAX));
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
  ///
  /// #[doc.overloads=slicemut.eq]
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

  /// Returns a reference to the element at position `i` in the Slice.
  ///
  /// # Panics
  /// If the index `i` is beyond the end of the slice, the function will panic.
  /// #[doc.overloads=slicemut.index.usize]
  sus_pure constexpr inline T& operator[](::sus::num::usize i) const& noexcept {
    ::sus::check(i < len());
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
  /// #[doc.overloads=slice.index.range]
  sus_pure constexpr inline SliceMut<T> operator[](
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
    return SliceMut<T>::from_raw_parts_mut(::sus::marker::unsafe_fn,
                                           as_mut_ptr() + rstart, rlen);
  }

  // TODO: Impl AsRef -> Slice<T>.
  constexpr Slice<T> as_slice() const& noexcept {
    return *this;
  }

  // SliceMut can be used as a Slice.
  sus_pure constexpr operator const Slice<T>&() const& { return slice_; }
  sus_pure constexpr operator Slice<T>&() & { return slice_; }
  sus_pure constexpr operator Slice<T>() && { return ::sus::move(slice_); }

#define _ptr_expr slice_.data_
#define _len_expr slice_.len_
#define _delete_rvalue false
#include "__private/slice_methods.inc"
#define _ptr_expr slice_.data_
#define _len_expr slice_.len_
#define _delete_rvalue false
#include "__private/slice_mut_methods.inc"

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
#define _delete_rvalue false
#define _self SliceMut
#include "__private/slice_methods_impl.inc"

// Implicit for-ranged loop iteration via `Slice::iter()` and
// `SliceMut::iter()`.
using ::sus::iter::__private::begin;
using ::sus::iter::__private::end;

}  // namespace sus::containers

// fmt support.
template <class T, class Char>
struct fmt::formatter<::sus::containers::Slice<T>, Char> {
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return underlying_.parse(ctx);
  }

  template <class FormatContext>
  constexpr auto format(const ::sus::containers::Slice<T>& slice,
                        FormatContext& ctx) const {
    auto out = ctx.out();
    out = fmt::format_to(out, "[");
    for (::sus::num::usize i; i < slice.len(); i += 1u) {
      if (i > 0u) out = fmt::format_to(out, ", ");
      ctx.advance_to(out);
      out = underlying_.format(slice[i], ctx);
    }
    return fmt::format_to(out, "]");
  }

 private:
  ::sus::string::__private::AnyFormatter<T, Char> underlying_;
};

// Stream support.
sus__format_to_stream(sus::containers, Slice, T);

// fmt support.
template <class T, class Char>
struct fmt::formatter<::sus::containers::SliceMut<T>, Char> {
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return underlying_.parse(ctx);
  }

  template <class FormatContext>
  constexpr auto format(const ::sus::containers::SliceMut<T>& slice,
                        FormatContext& ctx) const {
    auto out = ctx.out();
    out = fmt::format_to(out, "[");
    for (::sus::num::usize i; i < slice.len(); i += 1u) {
      if (i > 0u) out = fmt::format_to(out, ", ");
      ctx.advance_to(out);
      out = underlying_.format(slice[i], ctx);
    }
    return fmt::format_to(out, "]");
  }

 private:
  ::sus::string::__private::AnyFormatter<T, Char> underlying_;
};

// Stream support.
sus__format_to_stream(sus::containers, SliceMut, T);

// Promote Slice into the `sus` namespace.
namespace sus {
using ::sus::containers::Slice;
using ::sus::containers::SliceMut;
}  // namespace sus
