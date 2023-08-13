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
#include "sus/collections/__private/array_marker.h"
#include "sus/collections/collections.h"
#include "sus/collections/iterators/array_iter.h"
#include "sus/collections/iterators/slice_iter.h"
#include "sus/collections/slice.h"
#include "sus/construct/default.h"
#include "sus/fn/fn_concepts.h"
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
#include "sus/ops/eq.h"
#include "sus/ops/ord.h"
#include "sus/string/__private/any_formatter.h"
#include "sus/string/__private/format_to_stream.h"
#include "sus/tuple/tuple.h"

namespace sus::collections {

namespace __private {

template <class T, size_t N>
struct Storage final {
  sus_msvc_bug_10416202_else(
      [[sus_no_unique_address]]) sus::iter::IterRefCounter iter_refs_;
  T data_[N];
};

template <class T>
struct Storage<T, 0> final {};

}  // namespace __private

/// A collection of objects of type T, with a fixed size N.
///
/// An Array can not be larger than `isize::MAX`, as subtracting a pointer at a
/// greater distance results in Undefined Behaviour.
template <class T, size_t N>
class Array final {
  static_assert(N <= ::sus::mog<usize>(isize::MAX));
  static_assert(!std::is_reference_v<T>,
                "Array<T&, N> is invalid as Array must hold value types. Use "
                "Array<T*, N> instead.");
  static_assert(!std::is_const_v<T>,
                "`Array<const T, N>` should be written `const Array<T, N>`, as "
                "const applies transitively.");

 public:
  constexpr Array() noexcept
    requires(::sus::construct::Default<T>)
      : Array(kWithDefault, std::make_index_sequence<N>()) {}

  constexpr static Array with_initializer(
      ::sus::fn::FnMut<T()> auto&& f) noexcept {
    if constexpr (N == 0)
      return Array();
    else
      return Array(kWithInitializer, f, std::make_index_sequence<N>());
  }

  // Uses convertible_to<T> to accept `sus::into()` values. But doesn't use
  // sus::construct::Into<T> to avoid implicit conversions.
  template <std::convertible_to<T> U>
    requires(::sus::mem::Copy<T>)
  constexpr static Array with_value(U&& t) noexcept {
    if constexpr (N == 0)
      return Array();
    else
      return Array(kWithValue, t, std::make_index_sequence<N>());
  }

  // Uses convertible_to<T> instead of same_as<T> to accept `sus::into()`
  // values. But doesn't use sus::construct::Into<T> to avoid implicit
  // conversions.
  template <std::convertible_to<T>... Ts>
    requires(sizeof...(Ts) == N)
  constexpr static Array with(Ts&&... ts) noexcept {
    if constexpr (N == 0)
      return Array();
    else
      return Array(kWithValues, ::sus::forward<Ts>(ts)...);
  }

  Array(Array&&)
    requires(N == 0 && ::sus::mem::Move<T>)
  = default;
  Array& operator=(Array&&)
    requires(N == 0 && ::sus::mem::Move<T>)
  = default;

  constexpr Array(Array&& o) noexcept
    requires(N > 0 && ::sus::mem::Move<T>)
      : Array(kWithMoveFromPointer, o.storage_.data_,
              o.storage_.iter_refs_.take_for_owner(),
              std::make_index_sequence<N>()) {
    ::sus::check(!has_iterators());
  }
  constexpr Array& operator=(Array&& o) noexcept
    requires(::sus::mem::Move<T>)
  {
    ::sus::check(!has_iterators());
    ::sus::check(!o.has_iterators());
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
      return Array(kWithCloneFromPointer, storage_.data_,
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
    return Option<const T&>::with(*(storage_.data_ + i));
  }
  constexpr Option<const T&> get(usize i) && = delete;

  /// Returns a mutable reference to the element at index `i`.
  constexpr Option<T&> get_mut(usize i) & noexcept sus_lifetimebound
    requires(N > 0)
  {
    if (i >= N) [[unlikely]]
      return Option<T&>();
    return Option<T&>::with(*(storage_.data_ + i));
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
    check(i < N);
    return *(storage_.data_ + i);
  }
  constexpr inline const T& operator[](usize i) && = delete;

  constexpr inline T& operator[](usize i) & noexcept sus_lifetimebound {
    check(i < N);
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
      return SliceIter<const T&>::with(
          sus::iter::IterRefCounter::empty_for_view().to_iter_from_view(),
          nullptr, N);
    } else {
      // TODO: Add invalidation refcounts and check them on move. Would be part
      // of composing Array from SliceMut.
      return SliceIter<const T&>::with(storage_.iter_refs_.to_iter_from_owner(),
                                       storage_.data_, N);
    }
  }
  constexpr SliceIter<const T&> iter() && = delete;

  /// Returns an iterator over all the elements in the array, visited in the
  /// same order they appear in the array. The iterator gives mutable access to
  /// each element.
  constexpr SliceIterMut<T&> iter_mut() & noexcept sus_lifetimebound {
    if constexpr (N == 0) {
      return SliceIterMut<T&>::with(
          sus::iter::IterRefCounter::empty_for_view().to_iter_from_view(),
          nullptr, N);
    } else {
      return SliceIterMut<T&>::with(storage_.iter_refs_.to_iter_from_owner(),
                                    storage_.data_, N);
    }
  }

  /// Converts the array into an iterator that consumes the array and returns
  /// each element in the same order they appear in the array.
  template <int&..., class U = T>
  constexpr ArrayIntoIter<U, N> into_iter() && noexcept
    requires(::sus::mem::Move<T>)
  {
    return ArrayIntoIter<T, N>::with(::sus::move(*this));
  }

  /// Consumes the array, and returns a new array, mapping each element of the
  /// array to a new type with the given function.
  ///
  /// To just walk each element and map them, consider using `iter()` and
  /// `Iterator::map`. This does not require consuming the array.
  template <::sus::fn::FnMut<::sus::fn::NonVoid(T&&)> MapFn, int&...,
            class R = std::invoke_result_t<MapFn&&, T&&>>
    requires(N > 0)
  Array<R, N> map(MapFn f) && noexcept {
    return Array<R, N>::with_initializer([this, &f, i = 0_usize]() mutable {
      return f(::sus::move(*(storage_.data_ + ::sus::mem::replace(i, i + 1u))));
    });
  }

  /// sus::ops::Eq<Array<T, N>, Array<U, N>> trait.
  template <class U>
    requires(::sus::ops::Eq<T, U>)
  constexpr bool operator==(const Array<U, N>& r) const& noexcept
    requires(::sus::ops::Eq<T>)
  {
    return eq_impl(r, std::make_index_sequence<N>());
  }

  // TODO: Array is a slice.
  // Const Array can be used as a Slice.
  // constexpr operator const Slice<T>&() const& { return slice_mut_; }
  // constexpr operator Slice<T>&() & { return slice_mut_; }
  // constexpr operator Slice<T>() && { return ::sus::move(slice_mut_); }

  // TODO: Array is a slice.
  // Mutable Array can be used as a SliceMut.
  // constexpr operator SliceMut<T>&() & { return slice_mut_; }
  // constexpr operator SliceMut<T>&&() && { return ::sus::move(slice_mut_); }

 private:
  enum WithDefault { kWithDefault };
  template <size_t... Is>
    requires(N == 0)
  constexpr Array(WithDefault, std::index_sequence<Is...>) noexcept {}
  template <size_t... Is>
    requires(N > 0)
  constexpr Array(WithDefault, std::index_sequence<Is...>) noexcept
      : storage_(::sus::iter::IterRefCounter::for_owner(),
                 {((void)Is, T())...}) {}

  enum WithInitializer { kWithInitializer };
  template <size_t... Is>
    requires(N > 0)
  constexpr Array(WithInitializer, ::sus::fn::FnMut<T()> auto&& f,
                  std::index_sequence<Is...>) noexcept
      : storage_(::sus::iter::IterRefCounter::for_owner(),
                 {((void)Is, ::sus::fn::call_mut(f))...}) {}

  enum WithValue { kWithValue };
  template <size_t... Is>
    requires(N > 0)
  constexpr Array(WithValue, const T& t, std::index_sequence<Is...>) noexcept
      : storage_(::sus::iter::IterRefCounter::for_owner(), {((void)Is, t)...}) {
  }

  enum WithValues { kWithValues };
  template <class... Ts>
    requires(N > 0)
  constexpr Array(WithValues, Ts&&... ts) noexcept
      : storage_(::sus::iter::IterRefCounter::for_owner(),
                 {::sus::forward<Ts>(ts)...}) {}

  enum WithMoveFromPointer { kWithMoveFromPointer };
  template <size_t... Is>
    requires(N > 0)
  constexpr Array(WithMoveFromPointer, T* t, ::sus::iter::IterRefCounter refs,
                  std::index_sequence<Is...>) noexcept
      : storage_(::sus::move(refs), {::sus::move(*(t + Is))...}) {}

  enum WithCloneFromPointer { kWithCloneFromPointer };
  template <size_t... Is>
    requires(N > 0)
  constexpr Array(WithCloneFromPointer, const T* t,
                  std::index_sequence<Is...>) noexcept
      : storage_(::sus::iter::IterRefCounter::for_owner(),
                 {::sus::clone(*(t + Is))...}) {}

  constexpr inline bool has_iterators() const noexcept
    requires(N > 0)
  {
    return storage_.iter_refs_.count_from_owner() != 0u;
  }

  template <class U, size_t... Is>
  constexpr inline auto eq_impl(const Array<U, N>& r,
                                std::index_sequence<Is...>) const& noexcept {
    return (... && (get_unchecked(::sus::marker::unsafe_fn, Is) ==
                    r.get_unchecked(::sus::marker::unsafe_fn, Is)));
  };

  ::sus::collections::__private::Storage<T, N> storage_;

  sus_class_trivially_relocatable_if(::sus::marker::unsafe_fn,
                                     (N == 0 ||
                                      ::sus::mem::relocate_by_memcpy<T>));
};

namespace __private {

template <size_t I, class O, class T, class U, size_t N>
constexpr inline bool array_cmp_impl(O& val, const Array<T, N>& l,
                                     const Array<U, N>& r) noexcept {
  auto cmp = l.get_unchecked(::sus::marker::unsafe_fn, I) <=>
             r.get_unchecked(::sus::marker::unsafe_fn, I);
  // Allow downgrading from equal to equivalent, but not the inverse.
  if (cmp != 0) val = cmp;
  // Short circuit by returning true when we find a difference.
  return val == 0;
};

template <class T, class U, size_t N, size_t... Is>
constexpr inline auto array_cmp(auto equal, const Array<T, N>& l,
                                const Array<U, N>& r,
                                std::index_sequence<Is...>) noexcept {
  auto val = equal;
  (true && ... && (array_cmp_impl<Is>(val, l, r)));
  return val;
};

}  // namespace __private

/// Compares two Arrays.
///
/// Satisfies sus::ops::StrongOrd<Array<T, N>> if sus::ops::StrongOrd<T>.
///
/// Satisfies sus::ops::Ord<Array<T, N>> if sus::ops::Ord<T>.
///
/// Satisfies sus::ops::PartialOrd<Array<T, N>> if sus::ops::PartialOrd<T>.
//
// sus::ops::StrongOrd<Array<T, N>> trait.
// sus::ops::Ord<Array<T, N>> trait.
// sus::ops::PartialOrd<Array<T, N>> trait.
template <class T, class U, size_t N>
  requires(::sus::ops::ExclusiveStrongOrd<T, U>)
constexpr inline auto operator<=>(const Array<T, N>& l,
                                  const Array<U, N>& r) noexcept {
  return __private::array_cmp(std::strong_ordering::equivalent, l, r,
                              std::make_index_sequence<N>());
}

template <class T, class U, size_t N>
  requires(::sus::ops::ExclusiveOrd<T, U>)
constexpr inline auto operator<=>(const Array<T, N>& l,
                                  const Array<U, N>& r) noexcept {
  return __private::array_cmp(std::weak_ordering::equivalent, l, r,
                              std::make_index_sequence<N>());
}

template <class T, class U, size_t N>
  requires(::sus::ops::ExclusivePartialOrd<T, U>)
constexpr inline auto operator<=>(const Array<T, N>& l,
                                  const Array<U, N>& r) noexcept {
  return __private::array_cmp(std::partial_ordering::equivalent, l, r,
                              std::make_index_sequence<N>());
}

/// Implicit for-ranged loop iteration via `Array::iter()`.
using ::sus::iter::__private::begin;
/// Implicit for-ranged loop iteration via `Array::iter()`.
using ::sus::iter::__private::end;

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

/// Used to construct an Array<T, N> with the parameters as its values.
///
/// Calling array() produces a hint to make an Array<T, N> but does not actually
/// construct Array<T, N>, as the type `T` is not known here.
//
// Note: A marker type is used instead of explicitly constructing an array
// immediately in order to avoid redundantly having to specify `T` when using
// the result of `sus::array()` as a function argument or return value.
template <class... Ts>
  requires(sizeof...(Ts) > 0)
[[nodiscard]] inline constexpr auto array(
    Ts&&... vs sus_lifetimebound) noexcept {
  return __private::ArrayMarker<Ts...>(
      ::sus::tuple_type::Tuple<Ts&&...>::with(::sus::forward<Ts>(vs)...));
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
                                                    fmt::format("{}", value));
}

}  // namespace sus::collections

// Promote Array into the `sus` namespace.
namespace sus {
using ::sus::collections::array;
using ::sus::collections::Array;
}  // namespace sus
