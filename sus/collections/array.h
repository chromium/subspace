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

#include <stddef.h>
#include <stdint.h>

#include <concepts>
#include <type_traits>
#include <utility>  // TODO: replace std::make_index_sequence.

#include "fmt/core.h"
#include "sus/assertions/check.h"
#include "sus/cmp/eq.h"
#include "sus/cmp/ord.h"
#include "sus/collections/collections.h"
#include "sus/collections/iterators/array_iter.h"
#include "sus/collections/iterators/slice_iter.h"
#include "sus/collections/slice.h"
#include "sus/construct/default.h"
#include "sus/fn/fn_concepts.h"
#include "sus/iter/iterator_loop.h"
#include "sus/macros/__private/compiler_bugs.h"
#include "sus/macros/lifetimebound.h"
#include "sus/macros/no_unique_address.h"
#include "sus/marker/unsafe.h"
#include "sus/mem/clone.h"
#include "sus/mem/copy.h"
#include "sus/mem/forward.h"
#include "sus/mem/move.h"
#include "sus/mem/relocate.h"
#include "sus/num/num_concepts.h"
#include "sus/num/signed_integer.h"
#include "sus/num/unsigned_integer.h"
#include "sus/string/__private/any_formatter.h"
#include "sus/string/__private/format_to_stream.h"
#include "sus/tuple/tuple.h"

namespace sus::collections {

namespace __private {

template <class T, size_t N>
struct Storage final {
  [[_sus_no_unique_address]] sus::iter::IterRefCounter iter_refs_;
  [[_sus_no_unique_address]] T data_[N];
};

template <class T>
struct Storage<T, 0> final {};

}  // namespace __private

/// A collection of objects of type `T`, with a fixed size `N`.
///
/// An Array can not be larger than [`isize::MAX`]($sus::num::isize::MAX), as
/// subtracting a pointer at a greater distance results in Undefined Behaviour.
template <class T, size_t N>
class Array final {
  static_assert(N <= ::sus::cast<usize>(isize::MAX));
  static_assert(!std::is_reference_v<T>,
                "Array<T&, N> is invalid as Array must hold value types. Use "
                "Array<T*, N> instead.");
  static_assert(!std::is_const_v<T>,
                "`Array<const T, N>` should be written `const Array<T, N>`, as "
                "const applies transitively.");

 public:
  /// Default constructor of `Array` which default-constructs each object `T` in
  /// the array.
  ///
  /// This satisifies [`Default`]($sus::construct::Default) for the
  /// `Array<T, N>` if [`Default`]($sus::construct::Default) is satisifed for
  /// `T`.
  /// #[doc.overloads=ctor.default]
  explicit constexpr Array() noexcept
    requires(::sus::construct::Default<T>)
      : Array(WITH_DEFAULT, std::make_index_sequence<N>()) {}

  /// Constructs an `Array` with `N` elements from the `N` arguments given to
  /// the constructor.
  /// #[doc.overloads=ctor.values]
  template <std::convertible_to<T>... Ts>
    requires(sizeof...(Ts) == N &&  //
             N > 0u)
  explicit constexpr Array(Ts&&... ts) noexcept
      : storage_(::sus::iter::IterRefCounter::for_owner(),
                 {::sus::forward<Ts>(ts)...}) {}

  /// Constructs an `Array` with `N` elements from a single argument, repeatedly
  /// using it to construct each element. The given argument must be `Copy` in
  /// order to do this.
  ///
  /// To construct an Array from a single value that is `Clone` but not `Copy`,
  /// use `with_initializer([&x]() -> T { return sus::clone(x); })`;
  template <std::convertible_to<T> U>
    requires(::sus::mem::Copy<T>)
  constexpr static Array with_value(const U& t) noexcept {
    return [&t]<size_t... Is>(std::index_sequence<Is...>) {
      return Array(((void)Is, t)...);
    }(std::make_index_sequence<N>());
  }

  /// Constructs an `Array` with `N` elements, where each element is constructed
  /// from the given closure.
  constexpr static Array with_initializer(
      ::sus::fn::FnMut<T()> auto f) noexcept {
    if constexpr (N == 0u)
      return Array();
    else
      return Array(WITH_INITIALIZER, std::make_index_sequence<N>(), f);
  }

  Array(Array&&)
    requires(N == 0 && ::sus::mem::Move<T>)
  = default;
  Array& operator=(Array&&)
    requires(N == 0 && ::sus::mem::Move<T>)
  = default;

  constexpr Array(Array&& o) noexcept
    requires(N > 0 && ::sus::mem::Move<T>)
      : Array(WITH_MOVE_FROM_POINTER, o.storage_.data_,
              o.storage_.iter_refs_.take_for_owner(),
              std::make_index_sequence<N>()) {
    sus_check(!has_iterators());
  }
  constexpr Array& operator=(Array&& o) noexcept
    requires(::sus::mem::Move<T>)
  {
    sus_check(!has_iterators());
    sus_check(!o.has_iterators());
    for (usize i; i < N; i += 1u)
      *(storage_.data_ + i) = ::sus::mem::take(*(o.storage_.data_ + i));
    storage_.iter_refs_ = o.storage_.iter_refs_.take_for_owner();
    return *this;
  }

  Array(Array&&)
    requires(!::sus::mem::Move<T>)
  = delete;
  Array& operator=(Array&&)
    requires(!::sus::mem::Move<T>)
  = delete;

  // sus::mem::Clone trait.
  constexpr Array clone() const& noexcept
    requires(::sus::mem::Clone<T>)
  {
    if constexpr (N == 0) {
      return Array();
    } else {
      return Array(WITH_CLONE_FROM_POINTER, storage_.data_,
                   std::make_index_sequence<N>());
    }
  }

  constexpr void clone_from(const Array& source) & noexcept
    requires(::sus::mem::Clone<T>)
  {
    if constexpr (N > 0) {
      if (&source == this) [[unlikely]]
        return;
      for (usize i; i < N; i += 1u) {
        ::sus::clone_into(get_unchecked_mut(::sus::marker::unsafe_fn, i),
                          source.get_unchecked(::sus::marker::unsafe_fn, i));
      }
    }
  }

  ~Array()
    requires(std::is_trivially_destructible_v<T>)
  = default;
  constexpr ~Array()
    requires(!std::is_trivially_destructible_v<T>)
  {
    for (usize i; i < N; i += 1u) (storage_.data_ + i)->~T();
  }

  /// Returns the number of elements in the array.
  constexpr usize len() const& noexcept { return N; }

  /// Returns a const reference to the element at index `i`.
  constexpr Option<const T&> get(usize i) const& noexcept sus_lifetimebound
    requires(N > 0)
  {
    if (i >= N) [[unlikely]]
      return Option<const T&>();
    return Option<const T&>(*(storage_.data_ + i));
  }
  constexpr Option<const T&> get(usize i) && = delete;

  /// Returns a mutable reference to the element at index `i`.
  constexpr Option<T&> get_mut(usize i) & noexcept sus_lifetimebound
    requires(N > 0)
  {
    if (i >= N) [[unlikely]]
      return Option<T&>();
    return Option<T&>(*(storage_.data_ + i));
  }

  /// Returns a const reference to the element at index `i`.
  ///
  /// # Safety
  /// The index `i` must be inside the bounds of the array or Undefined
  /// Behaviour results.
  constexpr inline const T& get_unchecked(
      ::sus::marker::UnsafeFnMarker, usize i) const& noexcept sus_lifetimebound
    requires(N > 0)
  {
    return *(storage_.data_ + i);
  }
  constexpr inline const T& get_unchecked(::sus::marker::UnsafeFnMarker,
                                          usize i) && = delete;

  /// Returns a mutable reference to the element at index `i`.
  ///
  /// # Safety
  /// The index `i` must be inside the bounds of the array or Undefined
  /// Behaviour results.
  constexpr inline T& get_unchecked_mut(::sus::marker::UnsafeFnMarker,
                                        usize i) & noexcept sus_lifetimebound
    requires(N > 0)
  {
    return *(storage_.data_ + i);
  }

  constexpr inline const T& operator[](usize i) const& noexcept
      sus_lifetimebound {
    sus_check(i < N);
    return *(storage_.data_ + i);
  }
  constexpr inline const T& operator[](usize i) && = delete;

  constexpr inline T& operator[](usize i) & noexcept sus_lifetimebound {
    sus_check(i < N);
    return *(storage_.data_ + i);
  }

  /// Returns a const pointer to the first element in the array.
  constexpr inline const T* as_ptr() const& noexcept sus_lifetimebound
    requires(N > 0)
  {
    return storage_.data_;
  }
  const T* as_ptr() && = delete;

  /// Returns a mutable pointer to the first element in the array.
  constexpr inline T* as_mut_ptr() & noexcept sus_lifetimebound
    requires(N > 0)
  {
    return storage_.data_;
  }

  // Returns a slice that references all the elements of the array as const
  // references.
  constexpr Slice<T> as_slice() const& noexcept sus_lifetimebound {
    return Slice<T>::from(storage_.data_);
  }
  constexpr Slice<T> as_slice() && = delete;

  // Returns a slice that references all the elements of the array as mutable
  // references.
  constexpr SliceMut<T> as_mut_slice() & noexcept sus_lifetimebound {
    return SliceMut<T>::from(storage_.data_);
  }

  /// Returns an iterator over all the elements in the array, visited in the
  /// same order they appear in the array. The iterator gives const access to
  /// each element.
  constexpr SliceIter<const T&> iter() const& noexcept sus_lifetimebound {
    if constexpr (N == 0) {
      return SliceIter<const T&>(
          sus::iter::IterRefCounter::empty_for_view().to_iter_from_view(),
          nullptr, N);
    } else {
      // TODO: Add invalidation refcounts and check them on move. Would be part
      // of composing Array from SliceMut.
      return SliceIter<const T&>(storage_.iter_refs_.to_iter_from_owner(),
                                 storage_.data_, N);
    }
  }
  constexpr SliceIter<const T&> iter() && = delete;

  /// Returns an iterator over all the elements in the array, visited in the
  /// same order they appear in the array. The iterator gives mutable access to
  /// each element.
  constexpr SliceIterMut<T&> iter_mut() & noexcept sus_lifetimebound {
    if constexpr (N == 0) {
      return SliceIterMut<T&>(
          sus::iter::IterRefCounter::empty_for_view().to_iter_from_view(),
          nullptr, N);
    } else {
      return SliceIterMut<T&>(storage_.iter_refs_.to_iter_from_owner(),
                              storage_.data_, N);
    }
  }

  /// Converts the array into an iterator that consumes the array and returns
  /// each element in the same order they appear in the array.
  template <int&..., class U = T>
  constexpr ArrayIntoIter<U, N> into_iter() && noexcept
    requires(::sus::mem::Move<T>)
  {
    return ArrayIntoIter<T, N>(::sus::move(*this));
  }

  /// Consumes the array, and returns a new array, mapping each element of the
  /// array to a new type with the given function.
  ///
  /// To just walk each element and map them, consider using `iter()` and
  /// `Iterator::map`. This does not require consuming the array.
  template <::sus::fn::FnMut<::sus::fn::NonVoid(T&&)> MapFn>
    requires(N > 0)
  Array<sus::fn::ReturnOnce<MapFn, T&&>, N> map(MapFn f) && noexcept {
    return Array<sus::fn::ReturnOnce<MapFn, T&&>, N>::with_initializer(
        [this, &f, i = 0_usize]() mutable {
          return f(
              ::sus::move(*(storage_.data_ + ::sus::mem::replace(i, i + 1u))));
        });
  }

  /// Satisfies the [`Eq<Array<T, N>, Array<U, N>>`]($sus::cmp::Eq) concept.
  template <class U>
    requires(::sus::cmp::Eq<T, U>)
  constexpr bool operator==(const Array<U, N>& r) const& noexcept
    requires(::sus::cmp::Eq<T>)
  {
    return eq_impl(r, std::make_index_sequence<N>());
  }

  /// Satisfies the [`Eq<Array<T, N>, Slice<U>>`]($sus::cmp::Eq) concept.
  ///
  /// #[doc.overloads=array.eq.slice]
  template <class U>
    requires(::sus::cmp::Eq<T, U>)
  friend constexpr bool operator==(const Array<T, N>& l,
                                   const Slice<U>& r) noexcept {
    return r.len() == N && l.eq_impl(r, std::make_index_sequence<N>());
  }

  /// Satisfies the [`Eq<Array<T, N>, SliceMut<U>>`]($sus::cmp::Eq) concept.
  ///
  /// #[doc.overloads=array.eq.slicemut]
  template <class U>
    requires(::sus::cmp::Eq<T, U>)
  friend constexpr bool operator==(const Array<T, N>& l,
                                   const SliceMut<U>& r) noexcept {
    return r.len() == N && l.eq_impl(r, std::make_index_sequence<N>());
  }

  // Const Array can be used as a Slice.
  _sus_pure constexpr operator Slice<T>() const& noexcept
    requires(N == 0u)
  {
    return Slice<T>();
  }
  _sus_pure constexpr operator Slice<T>() const& noexcept
    requires(N > 0u)
  {
    return Slice<T>::from_raw_collection(
        ::sus::marker::unsafe_fn,
        // TODO: Would Iterator invalidation on move be useful?
        sus::iter::IterRefCounter::empty_for_view(), storage_.data_, N);
  }
  _sus_pure constexpr operator Slice<T>() && = delete;
  _sus_pure constexpr operator Slice<T>() & noexcept
    requires(N == 0u)
  {
    return Slice<T>();
  }
  _sus_pure constexpr operator Slice<T>() & noexcept
    requires(N > 0u)
  {
    return Slice<T>::from_raw_collection(
        ::sus::marker::unsafe_fn,
        // TODO: Would Iterator invalidation on move be useful?
        sus::iter::IterRefCounter::empty_for_view(), storage_.data_, N);
  }

  // Mutable Array can be used as a SliceMut.
  _sus_pure constexpr operator SliceMut<T>() & noexcept
    requires(N == 0u)
  {
    return SliceMut<T>();
  }
  _sus_pure constexpr operator SliceMut<T>() & noexcept
    requires(N > 0u)
  {
    return SliceMut<T>::from_raw_collection_mut(
        ::sus::marker::unsafe_fn,
        // TODO: Would Iterator invalidation on move be useful?
        sus::iter::IterRefCounter::empty_for_view(), storage_.data_, N);
  }

 private:
  enum WithDefault { WITH_DEFAULT };
  template <size_t... Is>
    requires(N == 0u)
  constexpr Array(WithDefault, std::index_sequence<Is...>) noexcept {}
  template <size_t... Is>
    requires(N > 0u)
  constexpr Array(WithDefault, std::index_sequence<Is...>) noexcept
      : storage_(::sus::iter::IterRefCounter::for_owner(),
                 {((void)Is, T())...}) {}

  enum WithInitializer { WITH_INITIALIZER };
  template <size_t... Is>
    requires(N > 0u)
  constexpr Array(WithInitializer, std::index_sequence<Is...>,
                  ::sus::fn::FnMut<T()> auto& f) noexcept
      : storage_(::sus::iter::IterRefCounter::for_owner(),
                 // NOTE: The calls to `f` need to happen in the initialization
                 // of the array. If they occur in funtion parameters then they
                 // will have an arbitrary ordering.
                 {((void)Is, ::sus::fn::call_mut(f))...}) {}

  enum WithMoveFromPointer { WITH_MOVE_FROM_POINTER };
  template <size_t... Is>
    requires(N > 0u)
  constexpr Array(WithMoveFromPointer, T* t, ::sus::iter::IterRefCounter refs,
                  std::index_sequence<Is...>) noexcept
      : storage_(::sus::move(refs), {::sus::move(*(t + Is))...}) {}

  enum WithCloneFromPointer { WITH_CLONE_FROM_POINTER };
  template <size_t... Is>
    requires(N > 0u)
  constexpr Array(WithCloneFromPointer, const T* t,
                  std::index_sequence<Is...>) noexcept
      : storage_(::sus::iter::IterRefCounter::for_owner(),
                 {::sus::clone(*(t + Is))...}) {}

  constexpr inline bool has_iterators() const noexcept
    requires(N > 0u)
  {
    return storage_.iter_refs_.count_from_owner() != 0u;
  }

  template <size_t... Is>
  constexpr inline auto eq_impl(const auto& r,
                                std::index_sequence<Is...>) const& noexcept {
    return (... && (get_unchecked(::sus::marker::unsafe_fn, Is) ==
                    r.get_unchecked(::sus::marker::unsafe_fn, Is)));
  };

  ::sus::collections::__private::Storage<T, N> storage_;

  sus_class_trivially_relocatable_if(::sus::marker::unsafe_fn,
                                     (N == 0 ||
                                      ::sus::mem::relocate_by_memcpy<T>));
};

template <class T, class... Ts>
  requires((... &&
            std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<Ts>>))
Array(T&&, Ts&&...) -> Array<std::remove_cvref_t<T>, 1u + sizeof...(Ts)>;

namespace __private {

template <size_t I>
constexpr inline bool array_cmp_impl(sus::cmp::Ordering auto& val,
                                     const auto& l, const auto& r) noexcept {
  auto cmp = l.get_unchecked(::sus::marker::unsafe_fn, I) <=>
             r.get_unchecked(::sus::marker::unsafe_fn, I);
  // Allow downgrading from equal to equivalent, but not the inverse.
  if (cmp != 0) val = cmp;
  // Short circuit by returning true when we find a difference.
  return val == 0;
};

template <size_t... Is>
constexpr inline auto array_cmp(sus::cmp::Ordering auto equal, const auto& l,
                                const auto& r,
                                std::index_sequence<Is...>) noexcept {
  auto val = equal;
  (true && ... && (array_cmp_impl<Is>(val, l, r)));
  return val;
};

}  // namespace __private

/// Compares two Arrays.
///
/// Satisfies sus::cmp::StrongOrd<Array<T, N>> if sus::cmp::StrongOrd<T>.
///
/// Satisfies sus::cmp::Ord<Array<T, N>> if sus::cmp::Ord<T>.
///
/// Satisfies sus::cmp::PartialOrd<Array<T, N>> if sus::cmp::PartialOrd<T>.
/// #[doc.overloads=array.cmp.array]
template <class T, class U, size_t N>
  requires(::sus::cmp::ExclusiveStrongOrd<T, U>)
constexpr inline std::strong_ordering operator<=>(
    const Array<T, N>& l, const Array<U, N>& r) noexcept {
  return __private::array_cmp(std::strong_ordering::equivalent, l, r,
                              std::make_index_sequence<N>());
}
/// #[doc.overloads=array.cmp.array]
template <class T, class U, size_t N>
  requires(::sus::cmp::ExclusiveOrd<T, U>)
constexpr inline std::weak_ordering operator<=>(const Array<T, N>& l,
                                                const Array<U, N>& r) noexcept {
  return __private::array_cmp(std::weak_ordering::equivalent, l, r,
                              std::make_index_sequence<N>());
}
/// #[doc.overloads=array.cmp.array]
template <class T, class U, size_t N>
  requires(::sus::cmp::ExclusivePartialOrd<T, U>)
constexpr inline std::partial_ordering operator<=>(
    const Array<T, N>& l, const Array<U, N>& r) noexcept {
  return __private::array_cmp(std::partial_ordering::equivalent, l, r,
                              std::make_index_sequence<N>());
}

/// Compares an Array and a Slice.
///
/// Satisfies sus::cmp::StrongOrd<Array<T, N>, Slice<T>> if
/// sus::cmp::StrongOrd<T>.
///
/// Satisfies sus::cmp::Ord<Array<T, N>, Slice<T>> if sus::cmp::Ord<T>.
///
/// Satisfies sus::cmp::PartialOrd<Array<T, N>, Slice<T>> if
/// sus::cmp::PartialOrd<T>.
/// #[doc.overloads=array.cmp.slice]
template <class T, class U, size_t N>
  requires(::sus::cmp::ExclusiveStrongOrd<T, U>)
constexpr inline std::strong_ordering operator<=>(const Array<T, N>& l,
                                                  const Slice<U>& r) noexcept {
  if (r.len() != N) return r.len() <=> N;
  return __private::array_cmp(std::strong_ordering::equivalent, l, r,
                              std::make_index_sequence<N>());
}
/// #[doc.overloads=array.cmp.slice]
template <class T, class U, size_t N>
  requires(::sus::cmp::ExclusiveOrd<T, U>)
constexpr inline std::weak_ordering operator<=>(const Array<T, N>& l,
                                                const Slice<U>& r) noexcept {
  if (r.len() != N) return r.len() <=> N;
  return __private::array_cmp(std::weak_ordering::equivalent, l, r,
                              std::make_index_sequence<N>());
}
/// #[doc.overloads=array.cmp.slice]
template <class T, class U, size_t N>
  requires(::sus::cmp::ExclusivePartialOrd<T, U>)
constexpr inline std::partial_ordering operator<=>(const Array<T, N>& l,
                                                   const Slice<U>& r) noexcept {
  if (r.len() != N) return r.len() <=> N;
  return __private::array_cmp(std::partial_ordering::equivalent, l, r,
                              std::make_index_sequence<N>());
}

/// Compares an Array and a SliceMut.
///
/// Satisfies sus::cmp::StrongOrd<Array<T, N>, SliceMut<T>> if
/// sus::cmp::StrongOrd<T>.
///
/// Satisfies sus::cmp::Ord<Array<T, N>, SliceMut<T>> if sus::cmp::Ord<T>.
///
/// Satisfies sus::cmp::PartialOrd<Array<T, N>, SliceMut<T>> if
/// sus::cmp::PartialOrd<T>.
/// #[doc.overloads=array.cmp.slicemut]
template <class T, class U, size_t N>
  requires(::sus::cmp::ExclusiveStrongOrd<T, U>)
constexpr inline std::strong_ordering operator<=>(
    const Array<T, N>& l, const SliceMut<U>& r) noexcept {
  if (r.len() != N) return r.len() <=> N;
  return __private::array_cmp(std::strong_ordering::equivalent, l, r,
                              std::make_index_sequence<N>());
}
/// #[doc.overloads=array.cmp.slicemut]
template <class T, class U, size_t N>
  requires(::sus::cmp::ExclusiveOrd<T, U>)
constexpr inline std::weak_ordering operator<=>(const Array<T, N>& l,
                                                const SliceMut<U>& r) noexcept {
  if (r.len() != N) return r.len() <=> N;
  return __private::array_cmp(std::weak_ordering::equivalent, l, r,
                              std::make_index_sequence<N>());
}
/// #[doc.overloads=array.cmp.slicemut]
template <class T, class U, size_t N>
  requires(::sus::cmp::ExclusivePartialOrd<T, U>)
constexpr inline std::partial_ordering operator<=>(
    const Array<T, N>& l, const SliceMut<U>& r) noexcept {
  if (r.len() != N) return r.len() <=> N;
  return __private::array_cmp(std::partial_ordering::equivalent, l, r,
                              std::make_index_sequence<N>());
}

/// Support for using structured bindings with `Array`.
/// #[doc.overloads=array.structured.bindings]
template <size_t I, class T, size_t N>
const auto& get(const Array<T, N>& a) noexcept {
  return a.get_unchecked(::sus::marker::unsafe_fn, I);
}
/// #[doc.overloads=array.structured.bindings]
template <size_t I, class T, size_t N>
auto& get(Array<T, N>& a) noexcept {
  return a.get_unchecked_mut(::sus::marker::unsafe_fn, I);
}
/// #[doc.overloads=array.structured.bindings]
template <size_t I, class T, size_t N>
auto get(Array<T, N>&& a) noexcept {
  return ::sus::move(a.get_unchecked_mut(::sus::marker::unsafe_fn, I));
}

}  // namespace sus::collections

// Structured bindings support.
namespace std {
template <class T, size_t N>
struct tuple_size<::sus::collections::Array<T, N>> {
  static constexpr size_t value = N;
};

template <size_t I, class T, size_t N>
struct tuple_element<I, ::sus::collections::Array<T, N>> {
  using type = T;
};
}  // namespace std

// fmt support.
template <class T, size_t N, class Char>
struct fmt::formatter<::sus::collections::Array<T, N>, Char> {
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return underlying_.parse(ctx);
  }

  template <class FormatContext>
  constexpr auto format(const ::sus::collections::Array<T, N>& array,
                        FormatContext& ctx) const
    requires(N > 0)
  {
    auto out = ctx.out();
    out = fmt::format_to(out, "[");
    for (::sus::num::usize i; i < array.len(); i += 1u) {
      if (i > 0u) out = fmt::format_to(out, ", ");
      ctx.advance_to(out);
      out = underlying_.format(array[i], ctx);
    }
    return fmt::format_to(out, "]");
  }

  template <class FormatContext>
  constexpr auto format(const ::sus::collections::Array<T, N>&,
                        FormatContext& ctx) const
    requires(N == 0)
  {
    return fmt::format_to(ctx.out(), "[]");
  }

 private:
  ::sus::string::__private::AnyFormatter<T, Char> underlying_;
};

// Stream support (written out manually due to size_t template param).
namespace sus::collections {
template <class T, size_t N,
          ::sus::string::__private::StreamCanReceiveString<char> StreamType>
inline StreamType& operator<<(StreamType& stream, const Array<T, N>& value) {
  return ::sus::string::__private::format_to_stream(stream,
                                                    fmt::to_string(value));
}

}  // namespace sus::collections

namespace sus::collections {
// Documented in vec.h
using ::sus::iter::begin;
// Documented in vec.h
using ::sus::iter::end;
}  // namespace sus::collections

// Promote Array into the `sus` namespace.
namespace sus {
using ::sus::collections::Array;
}  // namespace sus
