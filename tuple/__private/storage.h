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

#include <type_traits>
#include <utility>  // TODO: Replace std::index_sequence to remove this header.

#include "macros/compiler.h"
#include "mem/forward.h"
#include "mem/move.h"

namespace sus::tuple::__private {

struct UseAfterMoveMarker {
  constexpr inline UseAfterMoveMarker() noexcept : value(0u) {}
  constexpr inline UseAfterMoveMarker(const UseAfterMoveMarker& o) noexcept : value(o.value) {
    ::sus::check(!any_moved_from());
  }
  constexpr inline UseAfterMoveMarker& operator=(const UseAfterMoveMarker& o) noexcept {
    // Verify that the other Tuple has no moved-from items.
    value = o.value;
    ::sus::check(!any_moved_from());
    return *this;
  }
  constexpr inline UseAfterMoveMarker(UseAfterMoveMarker&& o) noexcept
      : value(o.set_all_moved_from()) {
    ::sus::check(!any_moved_from());
  }
  constexpr inline UseAfterMoveMarker& operator=(UseAfterMoveMarker&& o) noexcept {
    value = o.set_all_moved_from();
    ::sus::check(!any_moved_from());
    return *this;
  }

  constexpr inline bool any_moved_from() const noexcept { return value != 0u; }
  constexpr inline bool moved_from(size_t i) const noexcept {
    return (value & (uint64_t{1} << i)) != 0u;
  }
  // Sets one element as moved from and returns it was already moved from.
  constexpr inline bool set_moved_from(size_t i) noexcept {
    return (::sus::mem::replace(mref(value), value | uint64_t{1} << i) & (uint64_t{1} << i)) != 0u;
  }
  // Sets all elements as moved from and returns the old value.
  constexpr inline bool set_all_moved_from() noexcept {
    return ::sus::mem::replace(mref(value), uint64_t{0xffffffffffffffff});
  }

  uint64_t value;
};

// TODO: Consider having TupleStorage inherit from each type (if it's final then
// it has to compose it), or just inherit from N things that each hold a T, as
// libc++ does for std::tuple, using `:public Holds<Index..., T...>`. Then to
// get from the Storage to the base class you can static_cast the Storage
// pointer to it.
template <size_t N, class... T>
struct TupleStorage;

template <>
struct TupleStorage<0> {};

template <class T>
struct TupleStorage<1, T> {
  template <size_t I>
  using Type = T;
  sus_clang_bug_54040(template <std::convertible_to<T> U> constexpr inline TupleStorage(U&& t)
                      : t(::sus::forward<U>(t)){});
  T t;
};

template <size_t N, class T, class... Ts>
struct TupleStorage<N, T, Ts...> {
  using Next = TupleStorage<N - 1, Ts...>;
  template <size_t I>
  using Type = std::conditional_t<I == 0, T, typename Next::template Type<I - 1>>;

  template <class U, class... Us>
  constexpr inline TupleStorage(U&& t, Us&&... more) noexcept
      : t(forward<U>(t)), next(forward<Us>(more)...) {}
  [[sus_if_msvc(msvc::) no_unique_address]] T t;
  Next next;
};

template <class S, size_t I>
struct TupleAccess final {
  static inline constexpr const auto& get_ref(const S& tuple) noexcept {
    return TupleAccess<typename S::Next, I - 1>::get_ref(tuple.next);
  }

  static inline constexpr auto& get_mut(S& tuple) noexcept {
    return TupleAccess<typename S::Next, I - 1>::get_mut(tuple.next);
  }

  static inline constexpr decltype(auto) unwrap(S&& tuple) noexcept {
    return TupleAccess<typename S::Next, I - 1>::unwrap(::sus::move(tuple.next));
  }
};

template <class S>
struct TupleAccess<S, 0> final {
  static inline constexpr const auto& get_ref(const S& tuple) noexcept { return tuple.t; }

  static inline constexpr auto& get_mut(S& tuple) noexcept { return tuple.t; }

  static inline constexpr decltype(auto) unwrap(S&& tuple) noexcept {
    return ::sus::forward<decltype(tuple.t)>(tuple.t);
  }
};

template <size_t I, class S1, class S2>
constexpr inline auto storage_eq_impl(const S1& l, const S2& r) noexcept {
  return TupleAccess<S1, I + 1>::get_ref(l) == TupleAccess<S2, I + 1>::get_ref(r);
};

template <class S1, class S2, size_t... N>
constexpr inline auto storage_eq(const S1& l, const S2& r, std::index_sequence<N...>) noexcept {
  return (... && (storage_eq_impl<N>(l, r)));
};

template <size_t I, class O, class S1, class S2>
constexpr inline bool storage_cmp_impl(O& val, const S1& l, const S2& r) noexcept {
  auto cmp = TupleAccess<S1, I + 1>::get_ref(l) <=> TupleAccess<S2, I + 1>::get_ref(r);
  // Allow downgrading from equal to equivalent, but not the inverse.
  if (cmp != 0) val = cmp;
  // Short circuit by returning true when we find a difference.
  return val == 0;
};

template <class S1, class S2, size_t... N>
constexpr inline auto storage_cmp(auto equal, const S1& l, const S2& r,
                                  std::index_sequence<N...>) noexcept {
  auto val = equal;
  (... && (storage_cmp_impl<N>(val, l, r)));
  return val;
};

}  // namespace sus::tuple::__private
