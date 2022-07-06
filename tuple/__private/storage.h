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

#include "mem/forward.h"

namespace sus::tuple::__private {

template <size_t N, class... T>
struct TupleStorage;

template <>
struct TupleStorage<0> {};

template <class T>
struct TupleStorage<1, T> {
  template <size_t I>
  using Type = T;
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
struct TupleAccess {
  static inline constexpr const auto& get(const TupleStorage& tuple) noexcept {
    return TupleAccess<typename TupleStorage::Super, I - 1>::get(tuple);
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
struct TupleAccess<TupleStorage, 0> {
  static inline constexpr const auto& get(const TupleStorage& tuple) noexcept {
    return tuple.t;
  }

  static inline constexpr auto& get_mut(TupleStorage& tuple) noexcept {
    return tuple.t;
  }

  static inline constexpr auto&& unwrap(TupleStorage&& tuple) noexcept {
    return static_cast<decltype(tuple.t)&&>(tuple.t);
  }
};

}  // namespace sus::tuple::__private
