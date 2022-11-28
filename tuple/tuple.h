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

#include <compare>

#include "assertions/check.h"
#include "mem/forward.h"
#include "mem/replace.h"
#include "num/num_concepts.h"
#include "ops/eq.h"
#include "ops/ord.h"
#include "tuple/__private/storage.h"

namespace sus::tuple {

/// A Tuple is a finate sequence of one or more heterogenous values.
template <class T, class... Ts>
class Tuple final {
 public:
  /// Construct a Tuple with the given values.
  constexpr inline static Tuple with(T first, Ts... more) noexcept {
    return Tuple(static_cast<T&&>(first), static_cast<Ts>(more)...);
  }

  /// Gets a const reference to the `I`th element in the tuple.
  template <size_t I>
    requires(I <= sizeof...(Ts))
  constexpr inline const auto& get_ref() const& noexcept {
    ::sus::check(!moved_from());
    return Access<I + 1u>::get(storage_);
  }

  /// Disallows getting a reference to temporary Tuple.
  template <size_t I>
  constexpr inline const auto& get_ref() && = delete;

  /// Gets a mutable reference to the `I`th element in the tuple.
  template <size_t I>
    requires(I <= sizeof...(Ts))
  inline auto& get_mut() & noexcept {
    ::sus::check(!moved_from());
    return Access<I + 1u>::get_mut(storage_);
  }

  /// Returns the `I`th element in the tuple.
  template <size_t I>
    requires(I <= sizeof...(Ts))
  constexpr inline auto unwrap() && noexcept {
    ::sus::check(!set_moved_from());
    return Access<I + 1u>::unwrap(static_cast<Storage&&>(storage_));
  }

  /// sus::ops::Eq<Tuple<U...>> trait.
  template <class U, class... Us>
    requires(sizeof...(Us) == sizeof...(Ts) &&
             (::sus::ops::Eq<T, U> && ... && ::sus::ops::Eq<Ts, Us>))
  constexpr bool operator==(const Tuple<U, Us...>& r) const& noexcept {
    ::sus::check(!moved_from());
    ::sus::check(!r.moved_from());
    return __private::storage_eq(storage_, r.storage_,
                                 std::make_index_sequence<1 + sizeof...(Ts)>());
  }

  /// sus::ops::Ord<Tuple<U...>> trait.
  template <class U, class... Us>
    requires(sizeof...(Us) == sizeof...(Ts) &&
             (::sus::ops::ExclusiveOrd<T, U> && ... &&
              ::sus::ops::ExclusiveOrd<Ts, Us>))
  constexpr auto operator<=>(const Tuple<U, Us...>& r) const& noexcept {
    ::sus::check(!moved_from());
    ::sus::check(!r.moved_from());
    return __private::storage_cmp(
        std::strong_ordering::equal, storage_, r.storage_,
        std::make_index_sequence<1u + sizeof...(Ts)>());
  }

  /// sus::ops::WeakOrd<Tuple<U...>> trait.
  template <class U, class... Us>
    requires(sizeof...(Us) == sizeof...(Ts) &&
             (::sus::ops::ExclusiveWeakOrd<T, U> && ... &&
              ::sus::ops::ExclusiveWeakOrd<Ts, Us>))
  constexpr auto operator<=>(const Tuple<U, Us...>& r) const& noexcept {
    ::sus::check(!moved_from());
    ::sus::check(!r.moved_from());
    return __private::storage_cmp(
        std::weak_ordering::equivalent, storage_, r.storage_,
        std::make_index_sequence<1u + sizeof...(Ts)>());
  }

  /// sus::ops::PartialOrd<Tuple<U...>> trait.
  template <class U, class... Us>
    requires(sizeof...(Us) == sizeof...(Ts) &&
             (::sus::ops::ExclusivePartialOrd<T, U> && ... &&
              ::sus::ops::ExclusivePartialOrd<Ts, Us>))
  constexpr auto operator<=>(const Tuple<U, Us...>& r) const& noexcept {
    ::sus::check(!moved_from());
    ::sus::check(!r.moved_from());
    return __private::storage_cmp(
        std::partial_ordering::equivalent, storage_, r.storage_,
        std::make_index_sequence<1u + sizeof...(Ts)>());
  }

 private:
  template <class U, class... Us>
  friend class Tuple;  // For access to moved_from();

  /// Storage for the tuple elements. The first element is the moved-from flag.
  using Storage = __private::TupleStorage<2 + sizeof...(Ts), bool, T, Ts...>;
  /// A helper type used for accessing the `Storage`.
  template <size_t I>
  using Access = __private::TupleAccess<Storage, I>;

  constexpr inline Tuple(T&& first, Ts&&... more) noexcept
      : storage_(false, forward<T>(first), forward<Ts>(more)...) {}

  // TODO: Provide a way to opt out of all moved-from checks?
  constexpr inline bool moved_from() const& noexcept {
    return Access<0u>::get(storage_);
  }
  constexpr inline bool set_moved_from() & noexcept {
    return ::sus::mem::replace(mref(Access<0u>::get_mut(storage_)), true);
  }

  Storage storage_;
};

// Support for structured binding.
template <size_t I, class... Ts>
const auto& get(const Tuple<Ts...>& t) noexcept {
  return t.get_ref<I>();
}
template <size_t I, class... Ts>
auto& get(Tuple<Ts...>& t) noexcept {
  return t.get_mut<I>();
}
template <size_t I, class... Ts>
auto get(Tuple<Ts...>&& t) noexcept {
  return ::sus::move(t).unwrap<I>();
}

}  // namespace sus::tuple

namespace std {
template <class... Types>
struct tuple_size<::sus::tuple::Tuple<Types...>>
    : std::integral_constant<std::size_t, sizeof...(Types)> {};

template <std::size_t I, class... Types>
struct tuple_element<I, ::sus::tuple::Tuple<Types...>> {
  using type =
      decltype(std::declval<::sus::tuple::Tuple<Types...>>().unwrap<I>());
};

}  // namespace std

// Promote Tuple into the `sus` namespace.
namespace sus {
using ::sus::tuple::Tuple;
}  // namespace sus
