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
#include "sus/assertions/check.h"
#include "sus/assertions/debug_check.h"
#include "sus/cmp/eq.h"
#include "sus/cmp/ord.h"
#include "sus/collections/__private/sort.h"
#include "sus/collections/concat.h"
#include "sus/collections/iterators/chunks.h"
#include "sus/collections/iterators/slice_iter.h"
#include "sus/collections/iterators/split.h"
#include "sus/collections/iterators/windows.h"
#include "sus/collections/join.h"
#include "sus/construct/default.h"
#include "sus/fn/fn_concepts.h"
#include "sus/iter/iterator_defn.h"
#include "sus/iter/iterator_ref.h"
#include "sus/lib/__private/forward_decl.h"
#include "sus/macros/assume.h"
#include "sus/macros/lifetimebound.h"
#include "sus/macros/no_unique_address.h"
#include "sus/macros/pure.h"
#include "sus/marker/empty.h"
#include "sus/marker/unsafe.h"
#include "sus/mem/clone.h"
#include "sus/mem/move.h"
#include "sus/mem/swap.h"
#include "sus/num/cast.h"
#include "sus/num/signed_integer.h"
#include "sus/num/unsigned_integer.h"
#include "sus/ops/range.h"
#include "sus/option/option.h"
#include "sus/ptr/copy.h"
#include "sus/ptr/swap.h"
#include "sus/result/result.h"
#include "sus/string/__private/any_formatter.h"
#include "sus/string/__private/format_to_stream.h"
#include "sus/tuple/tuple.h"

namespace sus::collections {

/// A dynamically-sized const view into a contiguous sequence of objects of type
/// `const T`.
///
/// Contiguous here means that elements are laid out so that every element is
/// the same distance from its neighbors, where there are
/// [`sus::mem::size_of<T>()`]($sus::mem::size_of) many bytes between the start
/// of each element.
///
/// Slices are a view into a block of memory represented as a pointer and a
/// length.
template <class T>
class [[_sus_trivial_abi]] Slice final {
 public:
  static_assert(!std::is_reference_v<T>,
                "Slice holds references, so the type parameter can not also be "
                "a reference");
  static_assert(!std::is_const_v<T>,
                "Slice inner type should not be const, it provides const "
                "access to the slice regardless.");

  /// Constructs an empty `Slice`.
  ///
  /// This constructor is implicit so that using the [`EmptyMarker`](
  /// $sus::marker::EmptyMarker) allows the caller to avoid spelling out the
  /// full `Slice` type.
  /// #[doc.overloads=empty]
  constexpr Slice(::sus::marker::EmptyMarker) : Slice() {}

  /// Constructs an empty Slice, which has no elements.
  explicit constexpr Slice()
      : Slice(::sus::iter::IterRefCounter::empty_for_view(), nullptr, 0_usize) {
  }

  /// Constructs a slice from its raw parts.
  ///
  /// For building a Slice from a collection, use `from_raw_collection()`
  /// in order to participate in iterator invalidation tracking.
  ///
  /// # Safety
  /// The following must be upheld or Undefined Behaviour may result:
  /// * The `len` must be no more than the number of elements in the allocation
  ///   at and after the position of `data`.
  /// * The pointer `data` must be a valid pointer to an allocation, not a
  ///   dangling pointer, at any point during the Slice's lifetime. This must
  ///   be true even if `len` is 0.
  /// * The `refs` will be `sus::iter::IterRefCounter::empty_for_view()` unless
  ///   the `Slice` is being constructed from a context that owns an
  ///   IterRefCounter and wants to be able to observe when it invalidates the
  ///   `Slice` by tracking its lifetime.
  ///
  /// In some other langages such as Rust, the slice may hold an invalid pointer
  /// when the length is zero. But `Slice` must not hold a dangling pointer.
  /// Otherwise addition on the dangling pointer may happen in Slice methods,
  /// which is Undefined Behaviour in C++. To support dangling pointers, those
  /// methods would need `length == 0` branches. Care must be applied when
  /// converting slices between languages as a result.
  _sus_pure static constexpr Slice from_raw_parts(
      ::sus::marker::UnsafeFnMarker, const T* data sus_lifetimebound,
      usize len) noexcept {
    sus_check(len <= ::sus::cast<usize>(isize::MAX));
    // We strip the `const` off `data`, however only const access is provided
    // through this class. This is done so that mutable types can compose Slice
    // and store a mutable pointer.
    return Slice(::sus::iter::IterRefCounter::empty_for_view(),
                 const_cast<T*>(data), len);
  }

  /// Constructs a slice from its raw parts with iterator invalidation tracking.
  /// Iterators produced from this slice will interact with the collection to
  /// allow it to know when they are being invalidated by the collection.
  ///
  /// For building a Slice from primitive pointer, use `from_raw_parts()`.
  ///
  /// # Safety
  /// The following must be upheld or Undefined Behaviour may result:
  /// * The `len` must be no more than the number of elements in the allocation
  ///   at and after the position of `data`.
  /// * The pointer `data` must be a valid pointer to an allocation, not a
  ///   dangling pointer, at any point during the Slice's lifetime. This must
  ///   be true even if `len` is 0.
  /// * The `refs` will be `sus::iter::IterRefCounter::empty_for_view()` unless
  ///   the `Slice` is being constructed from a context that owns an
  ///   IterRefCounter and wants to be able to observe when it invalidates the
  ///   `Slice` by tracking its lifetime.
  ///
  /// In some other langages such as Rust, the slice may hold an invalid pointer
  /// when the length is zero. But `Slice` must not hold a dangling pointer.
  /// Otherwise addition on the dangling pointer may happen in Slice methods,
  /// which is Undefined Behaviour in C++. To support dangling pointers, those
  /// methods would need `length == 0` branches. Care must be applied when
  /// converting slices between languages as a result.
  _sus_pure static constexpr Slice from_raw_collection(
      ::sus::marker::UnsafeFnMarker, ::sus::iter::IterRefCounter refs,
      const T* data sus_lifetimebound, usize len) noexcept {
    sus_check(len <= ::sus::cast<usize>(isize::MAX));
    // We strip the `const` off `data`, however only const access is provided
    // through this class. This is done so that mutable types can compose Slice
    // and store a mutable pointer.
    return Slice(::sus::move(refs), const_cast<T*>(data), len);
  }

  /// sus::construct::From<T[N]> trait.
  ///
  /// Returns a Slice that refers to all elements of the `data` array.
  ///
  /// #[doc.overloads=from.array]
  template <size_t N>
    requires(N <= ::sus::cast<usize>(isize::MAX))
  _sus_pure static constexpr Slice from(const T (&data)[N] sus_lifetimebound) {
    // We strip the `const` off `data`, however only const access is provided
    // through this class. This is done so that mutable types can compose Slice
    // and store a mutable pointer.
    return Slice(sus::iter::IterRefCounter::empty_for_view(),
                 const_cast<T*>(data), N);
  }

  /// Converts the slice into an iterator that consumes the slice and returns
  /// each element in the same order they appear in the slice.
  _sus_pure constexpr SliceIter<const T&> into_iter() && noexcept {
    return SliceIter<const T&>(
        // This method is in Slice only, so it's a view type.
        iter_refs_.to_iter_from_view(), data_, len_);
  }

  /// Satisfies the [`Eq<Slice<T>, Slice<U>>`]($sus::cmp::Eq) concept.
  ///
  /// #[doc.overloads=slice.eq]
  template <class U>
    requires(::sus::cmp::Eq<T, U>)
  friend constexpr bool operator==(const Slice<T>& l,
                                   const Slice<U>& r) noexcept {
    if (l.len() != r.len()) return false;
    for (usize i = l.len(); i > 0u; i -= 1u) {
      if (!(l[i - 1u] == r[i - 1u])) return false;
    }
    return true;
  }

  template <class U>
    requires(!::sus::cmp::Eq<T, U>)
  friend constexpr bool operator==(const Slice<T>& l,
                                   const Slice<U>& r) = delete;

  /// Returns a reference to the element at position `i` in the Slice.
  ///
  /// # Panics
  /// If the index `i` is beyond the end of the slice, the function will panic.
  /// #[doc.overloads=slice.index.usize]
  _sus_pure constexpr const T& operator[](usize i) const& noexcept {
    sus_check(i < len());
    return *(as_ptr() + i);
  }

  /// Returns a subslice which contains elements in `range`, which specifies a
  /// start and an end.
  ///
  /// The start is the index of the first element to be returned in the
  /// subslice, and the end one past the last element in the output slice.
  /// As such, `r.get_range(Range(0u, r.len()))` returns a slice over the
  /// full set of elements in `r`.
  ///
  /// # Panics
  /// If the Range would otherwise contain an element that is out of bounds,
  /// the function will panic.
  /// #[doc.overloads=slice.index.range]
  _sus_pure constexpr Slice<T> operator[](
      const ::sus::ops::RangeBounds<usize> auto range) const& noexcept {
    const usize length = len();
    const usize rstart = range.start_bound().unwrap_or(0u);
    const usize rend = range.end_bound().unwrap_or(length);
    return subrange(rstart, rend);
  }

  /// Stops tracking iterator invalidation.
  ///
  /// # Safety
  ///
  /// If the Slice points into a collection and that collection is invalidated,
  /// it will no longer be caught. The caller must provide conditions that can
  /// ensure the `Slice`'s pointer into the collection will remain valid.
  ///
  /// Iterator invalidation tracking also tracks the stability of the collection
  /// object itself, not just its contents, which can be overly strict.
  ///
  /// This function can be used when the collection's contents will remain
  /// valid, but the collection itself may be moved, which would invalidate the
  /// tracking and be treated as invalidating the iterator. There is no way to
  /// restore tracking.
  void drop_iterator_invalidation_tracking(::sus::marker::UnsafeFnMarker) {
    iter_refs_ = ::sus::iter::IterRefCounter::empty_for_view();
  }

#define _ptr_expr data_
#define _len_expr len_
#define _iter_refs_expr iter_refs_.to_iter_from_view()
#define _iter_refs_view_expr iter_refs_.to_view_from_view()
#define _delete_rvalue false
#include "__private/slice_methods.inc"

 private:
  constexpr Slice(::sus::iter::IterRefCounter refs, T* data sus_lifetimebound,
                  usize len) noexcept
      : iter_refs_(::sus::move(refs)), data_(data), len_(len) {}

  friend class SliceMut<T>;
  template <class VecT>
  friend class Vec;

  [[_sus_no_unique_address]] ::sus::iter::IterRefCounter iter_refs_;
  T* data_;
  usize len_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(data_),
                                  decltype(len_));
  // Slice does not satisfy NeverValueField because it requires that the default
  // constructor is trivial, but Slice's default constructor needs to initialize
  // its fields.
};

#define _ptr_expr data_
#define _len_expr len_
#define _iter_refs_expr iter_refs_.to_iter_from_view()
#define _iter_refs_view_expr iter_refs_.to_view_from_view()
#define _delete_rvalue false
#define _self_template class T
#define _self Slice<T>
#include "__private/slice_methods_impl.inc"

/// A dynamically-sized mutable view into a contiguous sequence of objects of
/// type `T`.
///
/// Contiguous here means that elements are laid out so that every element is
/// the same distance from its neighbors, where there are
/// [`sus::mem::size_of<T>()`]($sus::mem::size_of) many bytes between the start
/// of each element.
///
/// Slices are a view into a block of memory represented as a pointer and a
/// length.
///
/// A `SliceMut<T>` can be implicitly converted to a `Slice<T>`.
template <class T>
class [[_sus_trivial_abi]] SliceMut final {
 public:
  static_assert(
      !std::is_reference_v<T>,
      "SliceMut holds references, so the type parameter can not also be "
      "a reference");
  static_assert(!std::is_const_v<T>,
                "SliceMut inner type should not be const, it provides mutable "
                "access to the slice. Use Slice to represent const access.");

  /// Constructs an empty `Slice`.
  ///
  /// This constructor is implicit so that using the [`EmptyMarker`](
  /// $sus::marker::EmptyMarker) allows the caller to avoid spelling out the
  /// full `Slice` type.
  /// #[doc.overloads=empty]
  constexpr SliceMut(::sus::marker::EmptyMarker) : SliceMut() {}

  /// Constructs an empty SliceMut, which has no elements.
  explicit constexpr SliceMut() = default;

  /// Constructs a slice from its raw parts.
  ///
  /// For building a SliceMut from a collection, use `from_raw_collection_mut()`
  /// in order to participate in iterator invalidation tracking.
  ///
  /// # Safety
  /// The following must be upheld or Undefined Behaviour may result:
  ///   IterRefCounter and wants to be able to observe when it invalidates the
  ///   `SliceMut` by tracking its lifetime.
  /// * The `len` must be no more than the number of elements in the allocation
  ///   at and after the position of `data`.
  /// * The pointer `data` must be a valid pointer to an allocation, not a
  ///   dangling pointer, at any point during the SliceMut's lifetime. This must
  ///   be true even if `len` is 0.
  ///
  /// In some other langages such as Rust, the slice may hold an invalid pointer
  /// when the length is zero. But `SliceMut` must not hold a dangling pointer.
  /// Otherwise addition on the dangling pointer may happen in SliceMut methods,
  /// which is Undefined Behaviour in C++. To support dangling pointers, those
  /// methods would need `length == 0` branches. Care must be applied when
  /// converting slices between languages as a result.
  _sus_pure static constexpr SliceMut from_raw_parts_mut(
      ::sus::marker::UnsafeFnMarker, T* data sus_lifetimebound,
      usize len) noexcept {
    sus_check(len <= ::sus::cast<usize>(isize::MAX));
    return SliceMut(::sus::iter::IterRefCounter::empty_for_view(), data, len);
  }

  /// Constructs a slice from its raw parts with iterator invalidation tracking.
  /// Iterators produced from this slice will interact with the collection to
  /// allow it to know when they are being invalidated by the collection.
  ///
  /// For building a SliceMut from primitive pointer, use
  /// `from_raw_parts_mut()`.
  ///
  /// # Safety
  /// The following must be upheld or Undefined Behaviour may result:
  /// * The `refs` should be constructed from an `IterRefCounter` in the
  ///   collection with `IterRefCounter::to_view_from_owner()`.
  /// * The `len` must be no more than the number of elements in the allocation
  ///   at and after the position of `data`.
  /// * The pointer `data` must be a valid pointer to an allocation, not a
  ///   dangling pointer, at any point during the SliceMut's lifetime. This must
  ///   be true even if `len` is 0.
  ///
  /// In some other langages such as Rust, the slice may hold an invalid pointer
  /// when the length is zero. But `SliceMut` must not hold a dangling pointer.
  /// Otherwise addition on the dangling pointer may happen in SliceMut methods,
  /// which is Undefined Behaviour in C++. To support dangling pointers, those
  /// methods would need `length == 0` branches. Care must be applied when
  /// converting slices between languages as a result.
  _sus_pure static constexpr SliceMut from_raw_collection_mut(
      ::sus::marker::UnsafeFnMarker, ::sus::iter::IterRefCounter refs,
      T* data sus_lifetimebound, usize len) noexcept {
    sus_check(len <= ::sus::cast<usize>(isize::MAX));
    return SliceMut(::sus::move(refs), data, len);
  }

  /// sus::construct::From<T[N]> trait.
  ///
  /// Returns a SliceMut that refers to all elements of the `data` array.
  ///
  /// #[doc.overloads=from.array]
  template <size_t N>
    requires(N <= ::sus::cast<usize>(isize::MAX_PRIMITIVE))
  _sus_pure static constexpr SliceMut from(T (&data)[N] sus_lifetimebound) {
    return SliceMut(::sus::iter::IterRefCounter::empty_for_view(), data, N);
  }

  /// Converts the slice into an iterator that consumes the slice and returns
  /// each element in the same order they appear in the slice.
  _sus_pure constexpr SliceIterMut<T&> into_iter() && noexcept {
    return SliceIterMut<T&>(
        // This method is in SliceMut only, so it's a view type.
        slice_.iter_refs_.to_iter_from_view(), slice_.data_, slice_.len_);
  }

  /// Satisfies the [`Eq<SliceMut<T>, SliceMut<U>>`]($sus::cmp::Eq) concept.
  ///
  /// #[doc.overloads=slicemut.eq]
  template <class U>
    requires(::sus::cmp::Eq<T, U>)
  friend constexpr bool operator==(const SliceMut<T>& l,
                                   const SliceMut<U>& r) noexcept {
    return l.as_slice() == r.as_slice();
  }

  template <class U>
    requires(!::sus::cmp::Eq<T, U>)
  friend constexpr bool operator==(const SliceMut<T>& l,
                                   const SliceMut<U>& r) = delete;

  /// Returns a reference to the element at position `i` in the Slice.
  ///
  /// # Panics
  /// If the index `i` is beyond the end of the slice, the function will panic.
  /// #[doc.overloads=slicemut.index.usize]
  _sus_pure constexpr T& operator[](usize i) const& noexcept {
    sus_check(i < len());
    return *(as_mut_ptr() + i);
  }

  /// Returns a subslice which contains elements in `range`, which specifies a
  /// start and an end.
  ///
  /// The start is the index of the first element to be returned in the
  /// subslice, and the end one past the last element in the output slice.
  /// As such, `r.get_range(Range(0u, r.len()))` returns a slice over the
  /// full set of elements in `r`.
  ///
  /// # Panics
  /// If the Range would otherwise contain an element that is out of bounds,
  /// the function will panic.
  /// #[doc.overloads=slicemut.index.range]
  _sus_pure constexpr SliceMut<T> operator[](
      const ::sus::ops::RangeBounds<usize> auto range) const& noexcept {
    const usize length = len();
    const usize rstart = range.start_bound().unwrap_or(0u);
    const usize rend = range.end_bound().unwrap_or(length);
    return subrange_mut(rstart, rend);
  }

  // TODO: Impl AsRef -> Slice<T>.
  constexpr Slice<T> as_slice() const& noexcept { return *this; }

  // SliceMut can be used as a Slice.
  _sus_pure constexpr operator const Slice<T>&() const& noexcept {
    return slice_;
  }
  _sus_pure constexpr operator Slice<T>&() & noexcept { return slice_; }
  _sus_pure constexpr operator Slice<T>() && noexcept {
    return ::sus::move(slice_);
  }

  /// Stops tracking iterator invalidation.
  ///
  /// # Safety
  ///
  /// If the Slice points into a collection and that collection is invalidated,
  /// it will no longer be caught. The caller must provide conditions that can
  /// ensure the `SliceMut`'s pointer into the collection will remain valid.
  ///
  /// Iterator invalidation tracking also tracks the stability of the collection
  /// object itself, not just its contents, which can be overly strict.
  ///
  /// This function can be used when the collection's contents will remain
  /// valid, but the collection itself may be moved, which would invalidate the
  /// tracking and be treated as invalidating the iterator. There is no way to
  /// restore tracking.
  void drop_iterator_invalidation_tracking(::sus::marker::UnsafeFnMarker) {
    slice_.iter_refs_ = ::sus::iter::IterRefCounter::empty_for_view();
  }

#define _ptr_expr slice_.data_
#define _len_expr slice_.len_
#define _iter_refs_expr slice_.iter_refs_.to_iter_from_view()
#define _iter_refs_view_expr slice_.iter_refs_.to_view_from_view()
#define _delete_rvalue false
#include "__private/slice_methods.inc"
#define _ptr_expr slice_.data_
#define _len_expr slice_.len_
#define _iter_refs_expr slice_.iter_refs_.to_iter_from_view()
#define _iter_refs_view_expr slice_.iter_refs_.to_view_from_view()
#define _delete_rvalue false
#include "__private/slice_mut_methods.inc"

 private:
  constexpr SliceMut(sus::iter::IterRefCounter refs, T* data, usize len)
      : slice_(::sus::move(refs), data, len) {}

  template <class VecT>
  friend class Vec;

  Slice<T> slice_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(slice_));
  // SliceMut does not satisfy NeverValueField because it requires that the
  // default constructor is trivial, but SliceMut's default constructor needs to
  // initialize its fields.
};

#define _ptr_expr slice_.data_
#define _len_expr slice_.len_
#define _iter_refs_expr slice_.iter_refs_.to_iter_from_view()
#define _iter_refs_view_expr slice_.iter_refs_.to_view_from_view()
#define _delete_rvalue false
#define _self_template class T
#define _self SliceMut<T>
#include "__private/slice_methods_impl.inc"

// Documented in vec.h
using ::sus::iter::begin;
// Documented in vec.h
using ::sus::iter::end;

}  // namespace sus::collections

// fmt support.
template <class T, class Char>
struct fmt::formatter<::sus::collections::Slice<T>, Char> {
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return underlying_.parse(ctx);
  }

  template <class FormatContext>
  constexpr auto format(const ::sus::collections::Slice<T>& slice,
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
_sus_format_to_stream(sus::collections, Slice, T);

// fmt support.
template <class T, class Char>
struct fmt::formatter<::sus::collections::SliceMut<T>, Char> {
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return underlying_.parse(ctx);
  }

  template <class FormatContext>
  constexpr auto format(const ::sus::collections::SliceMut<T>& slice,
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
_sus_format_to_stream(sus::collections, SliceMut, T);

// Promote Slice into the `sus` namespace.
namespace sus {
using ::sus::collections::Slice;
using ::sus::collections::SliceMut;
}  // namespace sus
