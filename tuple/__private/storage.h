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

#include "mem/forward.h"
#include "mem/move.h"

namespace sus::tuple::__private {

struct UseAfterMoveMarker {
  constexpr inline UseAfterMoveMarker() noexcept : value(0u) {}
  constexpr inline UseAfterMoveMarker(const UseAfterMoveMarker& o) noexcept
      : value(o.value) {
    ::sus::check(!any_moved_from());
  }
  constexpr inline UseAfterMoveMarker& operator=(
      const UseAfterMoveMarker& o) noexcept {
    // Verify that the other Tuple has no moved-from items.
    value = o.value;
    ::sus::check(!any_moved_from());
    return *this;
  }
  constexpr inline UseAfterMoveMarker(UseAfterMoveMarker&& o) noexcept
      : value(o.set_all_moved_from()) {
    ::sus::check(!any_moved_from());
  }
  constexpr inline UseAfterMoveMarker& operator=(
      UseAfterMoveMarker&& o) noexcept {
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
    return (::sus::mem::replace(mref(value), value | uint64_t{1} << i) &
            (uint64_t{1} << i)) != 0u;
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
template <class... T>
struct TupleStorage;

template <class T>
struct TupleStorage<T> {
  sus_clang_bug_54040(template <std::convertible_to<T> U>
                      constexpr inline TupleStorage(U&& value)
                      : value(::sus::forward<U>(value)){});
  T value;
};

template <class T, class... Ts>
  requires(sizeof...(Ts) > 0)
struct TupleStorage<T, Ts...> : TupleStorage<Ts...> {
  using Super = TupleStorage<Ts...>;

  template <class U, class... Us>
  constexpr inline TupleStorage(U&& value, Us&&... more) noexcept
      : Super(::sus::forward<Us>(more)...),
        value(::sus::forward<U>(value)) {}

  T value;
};

template <size_t I, class S>
static constexpr const auto& find_storage(const S& storage) {
  return find_storage(storage, std::integral_constant<size_t, I>());
}

template <size_t I, class S>
static constexpr const auto& find_storage(const S& storage,
                                          std::integral_constant<size_t, I>) {
  return find_storage(static_cast<const S::Super&>(storage),
                      std::integral_constant<size_t, I - 1>());
}

template <class S>
static constexpr const S& find_storage(const S& storage,
                                       std::integral_constant<size_t, 0>) {
  return storage;
}

template <size_t I, class S>
static constexpr auto& find_storage_mut(S& storage) {
  return find_storage_mut(storage, std::integral_constant<size_t, I>());
}

template <size_t I, class S>
static constexpr auto& find_storage_mut(S& storage,
                                        std::integral_constant<size_t, I>) {
  return find_storage_mut(static_cast<S::Super&>(storage),
                          std::integral_constant<size_t, I - 1>());
}

template <class S>
static constexpr S& find_storage_mut(S& storage,
                                     std::integral_constant<size_t, 0>) {
  return storage;
}

template <size_t I, class S1, class S2>
constexpr inline auto storage_eq_impl(const S1& l, const S2& r) noexcept {
  return find_storage<I>(l).value == find_storage<I>(r).value;
};

template <class S1, class S2, size_t... N>
constexpr inline auto storage_eq(const S1& l, const S2& r,
                                 std::index_sequence<N...>) noexcept {
  return (... && (storage_eq_impl<N>(l, r)));
};

template <size_t I, class O, class S1, class S2>
constexpr inline bool storage_cmp_impl(O& val, const S1& l,
                                       const S2& r) noexcept {
  auto cmp = find_storage<I>(l).value <=> find_storage<I>(r).value;
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
