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

template <size_t N, class... T>
struct TupleStorage;

template <>
struct TupleStorage<0> {};

template <class T>
struct TupleStorage<1, T> {
  template <size_t I>
  using Type = T;
  sus_clang_bug_54040(constexpr inline TupleStorage(T&& t)
                      : t(static_cast<T&&>(t)){})
  T t;
};

template <size_t N, class T, class... MoreT>
struct TupleStorage<N, T, MoreT...> : TupleStorage<N - 1, MoreT...> {
  using Super = TupleStorage<N - 1, MoreT...>;
  template <size_t I>
  using Type =
      std::conditional_t<I == 0, T, typename Super::template Type<I - 1>>;

  template <class U, class... MoreU>
  constexpr inline TupleStorage(U&& t, MoreU&&... more) noexcept
      : Super(forward<MoreU>(more)...), t(forward<U>(t)) {}
  T t;
};

template <class TupleStorage, size_t I>
struct TupleAccess final {
  static inline constexpr const auto& get_ref(const TupleStorage& tuple) noexcept {
    return TupleAccess<typename TupleStorage::Super, I - 1>::get_ref(tuple);
  }

  static inline constexpr auto& get_mut(TupleStorage& tuple) noexcept {
    return TupleAccess<typename TupleStorage::Super, I - 1>::get_mut(tuple);
  }

  static inline constexpr auto&& unwrap(TupleStorage&& tuple) noexcept {
    return TupleAccess<typename TupleStorage::Super, I - 1>::unwrap(
        static_cast<TupleStorage&&>(tuple));
  }
};

template <class TupleStorage>
struct TupleAccess<TupleStorage, 0> final {
  static inline constexpr const auto& get_ref(const TupleStorage& tuple) noexcept {
    return tuple.t;
  }

  static inline constexpr auto& get_mut(TupleStorage& tuple) noexcept {
    return tuple.t;
  }

  static inline constexpr auto&& unwrap(TupleStorage&& tuple) noexcept {
    return sus::move(tuple.t);
  }
};

template <size_t I, class S1, class S2>
constexpr inline auto storage_eq_impl(const S1& l, const S2& r) noexcept {
  return TupleAccess<S1, I + 1>::get_ref(l) == TupleAccess<S2, I + 1>::get_ref(r);
};

template <class S1, class S2, size_t... N>
constexpr inline auto storage_eq(const S1& l, const S2& r,
                                 std::index_sequence<N...>) noexcept {
  return (... && (storage_eq_impl<N>(l, r)));
};

template <size_t I, class O, class S1, class S2>
constexpr inline bool storage_cmp_impl(O& val, const S1& l,
                                       const S2& r) noexcept {
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
