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
#include "subspace/containers/__private/slice_macros.h"
#include "subspace/containers/iterators/chunks.h"
#include "subspace/containers/iterators/slice_iter.h"
#include "subspace/fn/fn_concepts.h"
#include "subspace/fn/fn_ref.h"
#include "subspace/iter/iterator_defn.h"
#include "subspace/macros/assume.h"
#include "subspace/macros/lifetimebound.h"
#include "subspace/marker/unsafe.h"
#include "subspace/mem/addressof.h"
#include "subspace/ops/ord.h"
#include "subspace/ops/range.h"
#include "subspace/option/option.h"
#include "subspace/result/result.h"

// TODO: sort_by_key()
// TODO: sort_by_cached_key()
// TODO: sort_unstable_by_key()

namespace sus::containers {

template <class T>
class Vec;

/// A dynamically-sized view into a contiguous sequence of objects of type `T`.
///
/// Contiguous here means that elements are laid out so that every element is
/// the same distance from its neighbors, where there are
/// `sus::mem::size_of<T>()` many bytes between the start of each element.
///
/// Slices are a view into a block of memory represented as a pointer and a
/// length.
template <class T>
class [[sus_trivial_abi]] Slice {
 public:
  template <class U>
  friend class Slice;

  static_assert(!std::is_reference_v<T>,
                "Slice holds references, so the type parameter can not also be "
                "a reference");

  /// Constructs an empty Slice, which has no elements.
  constexpr Slice() : Slice(nullptr, 0_usize) {}

  // Conversion from Slice<T> to Slice<const T>.
  template <class U>
    requires(!std::same_as<T, U> && std::same_as<T, const U>)
  constexpr Slice(const Slice<U>& o) : data_(o.data_), len_(o.len_) {}
  template <class U>
    requires(!std::same_as<T, U> && std::same_as<T, const U>)
  constexpr Slice& operator=(const Slice<U>& o) {
    data_ = o.data_;
    len_ = o.len_;
  }

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
  static constexpr inline Slice from_raw_parts(::sus::marker::UnsafeFnMarker,
                                               T* data sus_lifetimebound,
                                               ::sus::usize len) noexcept {
    ::sus::check(size_t{len} <= size_t{isize::MAX_PRIMITIVE});
    return Slice(data, len);
  }

  /// sus::construct::From<T[N]> trait.
  ///
  /// Returns a Slice that refers to all elements of the `data` array.
  ///
  /// #[doc.overloads=from.array]
  template <size_t N>
    requires(N <= size_t{isize::MAX_PRIMITIVE})
  static constexpr inline Slice from(T (&data)[N] sus_lifetimebound) {
    return Slice(data, N);
  }

  // Slice holds references, so returning a reference from an rvalue Slice is
  // fine. The qualifiers allow use on rvalues.
  _sus__slice_defns(Slice, data_, len_, const, );
  _sus__slice_mut_defns(Slice, data_, len_, const, );

 private:
  constexpr Slice(T* data sus_lifetimebound, usize len) noexcept
      : data_(data), len_(len) {}

  T* data_;
  ::sus::usize len_;

  sus_class_trivially_relocatable(::sus::marker::unsafe_fn, decltype(data_),
                                  decltype(len_));

  // Slice does not satisfy NeverValueField because it requires that the default
  // constructor is trivial, but Slice's default constructor needs to initialize
  // its fields.
};

// Slice holds references, so returning a reference from an rvalue Slice is
// fine. The qualifiers allow use on rvalues.
_sus__slice_decls(Slice, data_, len_, const, );
_sus__slice_mut_decls(Slice, data_, len_, const, );

// Implicit for-ranged loop iteration via `Slice::iter()`.
using ::sus::iter::__private::begin;
using ::sus::iter::__private::end;

template <class T>
using SliceMut = Slice<T>;

}  // namespace sus::containers

// Promote Slice into the `sus` namespace.
namespace sus {
using ::sus::containers::Slice;
using ::sus::containers::SliceMut;
}  // namespace sus
