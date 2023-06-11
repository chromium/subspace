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

#include <concepts>
#include <type_traits>
#include <utility>  // TODO: replace std::make_index_sequence.

#include "fmt/core.h"
#include "subspace/assertions/check.h"
#include "subspace/construct/default.h"
#include "subspace/containers/__private/array_marker.h"
#include "subspace/containers/iterators/array_iter.h"
#include "subspace/containers/iterators/slice_iter.h"
#include "subspace/containers/slice.h"
#include "subspace/fn/fn_concepts.h"
#include "subspace/macros/lifetimebound.h"
#include "subspace/marker/unsafe.h"
#include "subspace/mem/clone.h"
#include "subspace/mem/copy.h"
#include "subspace/mem/forward.h"
#include "subspace/mem/move.h"
#include "subspace/mem/relocate.h"
#include "subspace/num/num_concepts.h"
#include "subspace/num/unsigned_integer.h"
#include "subspace/ops/eq.h"
#include "subspace/ops/ord.h"
#include "subspace/string/__private/any_formatter.h"
#include "subspace/string/__private/format_to_stream.h"
#include "subspace/tuple/tuple.h"

namespace sus::containers {

namespace __private {

template <class T, size_t N>
struct Storage final {
  T data_[N];
};

template <class T>
struct Storage<T, 0> final {};

}  // namespace __private

/// A container of objects of type T, with a fixed size N.
///
/// An Array can not be larger than `isize::MAX`, as subtracting a pointer at a
/// greater distance results in Undefined Behaviour.
template <class T, size_t N>
  requires(N <= size_t{PTRDIFF_MAX})
class Array final {
  static_assert(!std::is_const_v<T>,
                "`Array<const T, N>` should be written `const Array<T, N>`, as "
                "const applies transitively.");

 public:
  constexpr Array() noexcept
    requires(::sus::construct::Default<T>)
      : Array(kWithDefault, std::make_index_sequence<N>()) {}

  constexpr static Array with_initializer(
      ::sus::fn::FnMut<T()> auto&& f) noexcept
    requires(::sus::mem::Move<T>)
  {
    return Array(kWithInitializer, f, std::make_index_sequence<N>());
  }

  // Uses convertible_to<T> to accept `sus::into()` values. But doesn't use
  // sus::construct::Into<T> to avoid implicit conversions.
  template <std::convertible_to<T> U>
    requires(::sus::mem::Copy<T>)
  constexpr static Array with_value(U&& t) noexcept {
    return Array(kWithValue, t, std::make_index_sequence<N>());
  }

  // Uses convertible_to<T> instead of same_as<T> to accept `sus::into()`
  // values. But doesn't use sus::construct::Into<T> to avoid implicit
  // conversions.
  template <std::convertible_to<T>... Ts>
    requires(sizeof...(Ts) == N && ::sus::mem::Move<T>)
  constexpr static Array with_values(Ts&&... ts) noexcept {
    return Array(kWithValues, ::sus::forward<Ts>(ts)...);
  }

  Array(Array&&)
      // TODO: Make a TrivialMove<T>.
    requires(::sus::mem::Move<T> && std::is_trivially_move_constructible_v<T>)
  = default;
  Array(Array&& o)
      // TODO: Make a NonTrivialMove<T>.
    requires(::sus::mem::Move<T> && !std::is_trivially_move_constructible_v<T>)
      : Array(kWithMoveFromPointer, o.storage_.data_,
              std::make_index_sequence<N>()) {}
  Array(Array&&)
    requires(!::sus::mem::Move<T>)
  = delete;

  Array& operator=(Array&&)
    requires(::sus::mem::Move<T> && std::is_trivially_move_assignable_v<T>)
  = default;
  Array& operator=(Array&& o)
    requires(::sus::mem::Move<T> && !std::is_trivially_move_assignable_v<T>)
  {
    for (auto i = size_t{0}; i < N; ++i)
      storage_.data_[i] = ::sus::mem::take(mref(o.storage_.data_[i]));
  }
  Array& operator=(Array&&)
    requires(!::sus::mem::Move<T>)
  = delete;

  // sus::mem::Clone trait.
  constexpr Array clone() const& noexcept
    requires(::sus::mem::Clone<T>)
  {
    auto ar = Array(kWithCloneFromPointer, storage_.data_,
                    std::make_index_sequence<N>());
    return ar;
  }

  constexpr void clone_from(const Array& source) & noexcept
    requires(::sus::mem::Clone<T>)
  {
    if (&source == this) [[unlikely]]
      return;
    for (auto i = size_t{0}; i < N; ++i) {
      ::sus::clone_into(mref(get_unchecked_mut(::sus::marker::unsafe_fn, i)),
                        source.get_unchecked(::sus::marker::unsafe_fn, i));
    }
  }

  ~Array()
    requires(std::is_trivially_destructible_v<T>)
  = default;
  ~Array()
    requires(!std::is_trivially_destructible_v<T>)
  {
    for (auto i = size_t{0}; i < N; ++i) storage_.data_[i].~T();
  }

  /// Returns the number of elements in the array.
  constexpr usize len() const& noexcept { return N; }

  /// Returns a const reference to the element at index `i`.
  constexpr Option<const T&> get(usize i) const& noexcept sus_lifetimebound
    requires(N > 0)
  {
    if (i.primitive_value >= N) [[unlikely]]
      return Option<const T&>();
    return Option<const T&>::with(storage_.data_[i.primitive_value]);
  }
  constexpr Option<const T&> get(usize i) && = delete;

  /// Returns a mutable reference to the element at index `i`.
  constexpr Option<T&> get_mut(usize i) & noexcept sus_lifetimebound
    requires(N > 0)
  {
    if (i.primitive_value >= N) [[unlikely]]
      return Option<T&>();
    return Option<T&>::with(mref(storage_.data_[i.primitive_value]));
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
    return storage_.data_[i.primitive_value];
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
    return storage_.data_[i.primitive_value];
  }

  constexpr inline const T& operator[](usize i) const& noexcept
      sus_lifetimebound {
    check(i.primitive_value < N);
    return storage_.data_[i.primitive_value];
  }
  constexpr inline const T& operator[](usize i) && = delete;

  constexpr inline T& operator[](usize i) & noexcept sus_lifetimebound {
    check(i.primitive_value < N);
    return storage_.data_[i.primitive_value];
  }

  /// Returns a const pointer to the first element in the array.
  inline const T* as_ptr() const& noexcept sus_lifetimebound
    requires(N > 0)
  {
    return storage_.data_;
  }
  const T* as_ptr() && = delete;

  /// Returns a mutable pointer to the first element in the array.
  inline T* as_mut_ptr() & noexcept sus_lifetimebound
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
    return SliceIter<const T&>::with(storage_.data_, N);
  }
  constexpr SliceIter<const T&> iter() && = delete;

  /// Returns an iterator over all the elements in the array, visited in the
  /// same order they appear in the array. The iterator gives mutable access to
  /// each element.
  constexpr SliceIterMut<T&> iter_mut() & noexcept sus_lifetimebound {
    return SliceIterMut<T&>::with(storage_.data_, N);
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
    return Array<R, N>::with_initializer([this, &f, i = size_t{0}]() mutable {
      return f(::sus::move(storage_.data_[i++]));
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
  constexpr Array(WithDefault, std::index_sequence<Is...>) noexcept
      : storage_{((void)Is, T())...} {}

  enum WithInitializer { kWithInitializer };
  template <size_t... Is>
  constexpr Array(WithInitializer, ::sus::fn::FnMut<T()> auto&& f,
                  std::index_sequence<Is...>) noexcept
      : storage_{((void)Is, f())...} {}

  enum WithValue { kWithValue };
  template <size_t... Is>
  constexpr Array(WithValue, const T& t, std::index_sequence<Is...>) noexcept
      : storage_{((void)Is, t)...} {}

  enum WithValues { kWithValues };
  template <class... Ts>
  constexpr Array(WithValues, Ts&&... ts) noexcept
      : storage_{::sus::forward<Ts>(ts)...} {}

  enum WithMoveFromPointer { kWithMoveFromPointer };
  template <size_t... Is>
  constexpr Array(WithMoveFromPointer, T* t,
                  std::index_sequence<Is...>) noexcept
      : storage_{::sus::move(*(t + Is))...} {}

  enum WithCloneFromPointer { kWithCloneFromPointer };
  template <size_t... Is>
  constexpr Array(WithCloneFromPointer, const T* t,
                  std::index_sequence<Is...>) noexcept
      : storage_{::sus::clone(*(t + Is))...} {}

  template <std::convertible_to<T> T1, std::convertible_to<T>... Ts>
  static inline void init_values(T* a, size_t index, T1&& t1, Ts&&... ts) {
    new (a + index) T(::sus::forward<T1>(t1));
    init_values(a, index + 1, ::sus::forward<Ts>(ts)...);
  }
  template <std::convertible_to<T> T1>
  static inline void init_values(T* a, size_t index, T1&& t1) {
    new (a + index) T(::sus::forward<T1>(t1));
  }

  template <class U, size_t... Is>
  constexpr inline auto eq_impl(const Array<U, N>& r,
                                std::index_sequence<Is...>) const& noexcept {
    return (... && (get_unchecked(::sus::marker::unsafe_fn, Is) ==
                    r.get_unchecked(::sus::marker::unsafe_fn, Is)));
  };

  ::sus::containers::__private::Storage<T, N> storage_;

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
/// Satisfies sus::ops::Ord<Array<T, N>> if sus::ops::Ord<T>.
///
/// Satisfies sus::ops::WeakOrd<Array<T, N>> if sus::ops::WeakOrd<T>.
///
/// Satisfies sus::ops::PartialOrd<Array<T, N>> if sus::ops::PartialOrd<T>.
//
// sus::ops::Ord<Array<T, N>> trait.
// sus::ops::WeakOrd<Array<T, N>> trait.
// sus::ops::PartialOrd<Array<T, N>> trait.
template <class T, class U, size_t N>
  requires(::sus::ops::ExclusiveOrd<T, U>)
constexpr inline auto operator<=>(const Array<T, N>& l,
                                  const Array<U, N>& r) noexcept {
  return __private::array_cmp(std::strong_ordering::equivalent, l, r,
                              std::make_index_sequence<N>());
}

template <class T, class U, size_t N>
  requires(::sus::ops::ExclusiveWeakOrd<T, U>)
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

// Implicit for-ranged loop iteration via `Array::iter()`.
using ::sus::iter::__private::begin;
using ::sus::iter::__private::end;

// Support for structured binding.
template <size_t I, class T, size_t N>
const auto& get(const Array<T, N>& a) noexcept {
  return a.get_unchecked(::sus::marker::unsafe_fn, I);
}
template <size_t I, class T, size_t N>
auto& get(Array<T, N>& a) noexcept {
  return a.get_unchecked_mut(::sus::marker::unsafe_fn, I);
}
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

}  // namespace sus::containers

// Structured bindings support.
namespace std {
template <class T, size_t N>
struct tuple_size<::sus::containers::Array<T, N>> {
  static constexpr size_t value = N;
};

template <size_t I, class T, size_t N>
struct tuple_element<I, ::sus::containers::Array<T, N>> {
  using type = T;
};
}  // namespace std

// fmt support.
template <class T, size_t N, class Char>
struct fmt::formatter<::sus::containers::Array<T, N>, Char> {
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return underlying_.parse(ctx);
  }

  template <class FormatContext>
  constexpr auto format(const ::sus::containers::Array<T, N>& array,
                        FormatContext& ctx) const
    requires(N > 0)
  {
    auto out = ctx.out();
    out = format_to(out, "[");
    for (::sus::num::usize i; i < array.len(); i += 1) {
      if (i > 0) out = format_to(out, ", ");
      ctx.advance_to(out);
      out = underlying_.format(array[i], ctx);
    }
    return format_to(out, "]");
  }

  template <class FormatContext>
  constexpr auto format(const ::sus::containers::Array<T, N>&,
                        FormatContext& ctx) const
    requires(N == 0)
  {
    return format_to(ctx.out(), "[]");
  }

 private:
  ::sus::string::__private::AnyFormatter<T, Char> underlying_;
};

// Stream support (written out manually due to size_t template param).
namespace sus::containers {
template <class T, size_t N,
          ::sus::string::__private::StreamCanReceiveString<char> StreamType>
inline StreamType& operator<<(StreamType& stream, const Array<T, N>& value) {
  return ::sus::string::__private::format_to_stream(stream,
                                                    fmt::format("{}", value));
}
}  // namespace sus::containers

// Promote Array into the `sus` namespace.
namespace sus {
using ::sus::containers::array;
using ::sus::containers::Array;
}  // namespace sus
