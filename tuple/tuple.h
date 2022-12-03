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
#include <concepts>

#include "assertions/check.h"
#include "construct/make_default.h"
#include "mem/forward.h"
#include "mem/replace.h"
#include "num/num_concepts.h"
#include "ops/eq.h"
#include "ops/ord.h"
#include "tuple/__private/storage.h"

#define SUS_CONFIG_TUPLE_USE_AFTER_MOVE true

namespace sus::tuple {

/// A Tuple is a finate sequence of one or more heterogeneous values.
template <class T, class... Ts>
class Tuple final {
  template <size_t I>
  static constexpr bool IsConst =
      std::is_const_v<std::remove_reference_t<std::tuple_element_t<I, Tuple>>>;

 public:
  static constexpr auto protects_uam = SUS_CONFIG_TUPLE_USE_AFTER_MOVE;

  /// Construct a Tuple with the given values.
  template <std::convertible_to<T> U, std::convertible_to<Ts>... Us>
    requires(sizeof...(Us) == sizeof...(Ts))
  constexpr inline static Tuple with(U&& first, Us&&... more) noexcept {
    return Tuple(::sus::forward<U>(first), ::sus::forward<Us>(more)...);
  }

  /// Construct a Tuple with the default value for the types it contains.
  ///
  /// The Tuple's contained types must all be #MakeDefault, and will be
  /// constructed through that trait.
  static inline constexpr Tuple with_default() noexcept
    requires((::sus::construct::MakeDefault<T> && ... &&
              ::sus::construct::MakeDefault<Ts>))
  {
    return Tuple(::sus::construct::make_default<T>(),
                 ::sus::construct::make_default<Ts>()...);
  }

  /// Gets a const reference to the `I`th element in the tuple.
  template <size_t I>
    requires(I <= sizeof...(Ts))
  constexpr inline const auto& get_ref() const& noexcept {
    ::sus::check(!moved_from(I));
    return __private::find_storage<I>(storage_).value;
  }

  /// Disallows getting a reference to temporary Tuple.
  template <size_t I>
  constexpr inline const auto& get_ref() && = delete;

  /// Gets a mutable reference to the `I`th element in the tuple.
  template <size_t I>
    requires(I <= sizeof...(Ts) && !IsConst<I>)
  inline auto& get_mut() & noexcept {
    ::sus::check(!moved_from(I));
    return __private::find_storage_mut<I>(storage_).value;
  }

  /// sus::ops::Eq<Tuple<U...>> trait.
  template <class U, class... Us>
    requires(sizeof...(Us) == sizeof...(Ts) &&
             (::sus::ops::Eq<T, U> && ... && ::sus::ops::Eq<Ts, Us>))
  constexpr bool operator==(const Tuple<U, Us...>& r) const& noexcept {
    ::sus::check(!any_moved_from());
    ::sus::check(!r.any_moved_from());
    return __private::storage_eq(
        storage_, r.storage_, std::make_index_sequence<1u + sizeof...(Ts)>());
  }

  /// sus::ops::Ord<Tuple<U...>> trait.
  template <class U, class... Us>
    requires(sizeof...(Us) == sizeof...(Ts) &&
             (::sus::ops::ExclusiveOrd<T, U> && ... &&
              ::sus::ops::ExclusiveOrd<Ts, Us>))
  constexpr auto operator<=>(const Tuple<U, Us...>& r) const& noexcept {
    ::sus::check(!any_moved_from());
    ::sus::check(!r.any_moved_from());
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
    ::sus::check(!any_moved_from());
    ::sus::check(!r.any_moved_from());
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
    ::sus::check(!any_moved_from());
    ::sus::check(!r.any_moved_from());
    return __private::storage_cmp(
        std::partial_ordering::equivalent, storage_, r.storage_,
        std::make_index_sequence<1u + sizeof...(Ts)>());
  }

 private:
  template <class U, class... Us>
  friend class Tuple;  // For access to moved_from();

  /// Storage for the tuple elements. The first element is the moved-from flag.
  using Storage = __private::TupleStorage<T, Ts...>;

  template <std::convertible_to<T> U, std::convertible_to<Ts>... Us>
  constexpr inline Tuple(U&& first, Us&&... more) noexcept
      : storage_(::sus::forward<U>(first), ::sus::forward<Us>(more)...) {}

  // TODO: Provide a way to opt out of all moved-from checks?
  constexpr inline bool any_moved_from() const& noexcept {
#if SUS_CONFIG_TUPLE_USE_AFTER_MOVE
    return marker.any_moved_from();
#else
    return false;
#endif
  }
  constexpr inline bool moved_from(size_t i) const& noexcept {
#if SUS_CONFIG_TUPLE_USE_AFTER_MOVE
    return marker.moved_from(i);
#else
    (void)i;
    return false;
#endif
  }
  // Sets one element as moved from and returns it was already moved from.
  constexpr inline bool set_moved_from(size_t i) & noexcept {
#if SUS_CONFIG_TUPLE_USE_AFTER_MOVE
    return marker.set_moved_from(i);
#else
    (void)i;
    return false;
#endif
  }
  // Sets all elements as moved from and returns if any were already moved from.
  constexpr inline bool set_all_moved_from() & noexcept {
#if SUS_CONFIG_TUPLE_USE_AFTER_MOVE
    return marker.set_all_moved_from();
#else
    return false;
#endif
  }

#if SUS_CONFIG_TUPLE_USE_AFTER_MOVE
  __private::UseAfterMoveMarker marker;
#endif
  Storage storage_;
};

// Support for structured binding.
template <size_t I, class... Ts>
const auto& get(const Tuple<Ts...>& t) noexcept {
  return t.template get_ref<I>();
}
template <size_t I, class... Ts>
auto& get(Tuple<Ts...>& t) noexcept {
  return t.template get_mut<I>();
}
template <size_t I, class... Ts>
decltype(auto) get(Tuple<Ts...>&& t) noexcept {
  return ::sus::mem::move_or_copy_ref(t.template get_mut<I>());
}

}  // namespace sus::tuple

namespace std {
template <class... Types>
struct tuple_size<::sus::tuple::Tuple<Types...>>
    : std::integral_constant<std::size_t, sizeof...(Types)> {};

template <std::size_t I, class T, class... Types>
struct tuple_element<I, ::sus::tuple::Tuple<T, Types...>> {
  using type = tuple_element<I - 1, ::sus::tuple::Tuple<Types...>>::type;
};

template <class T, class... Types>
struct tuple_element<0, ::sus::tuple::Tuple<T, Types...>> {
  using type = T;
};

}  // namespace std

// Promote Tuple into the `sus` namespace.
namespace sus {
using ::sus::tuple::Tuple;
}  // namespace sus

#undef SUS_CONFIG_TUPLE_USE_AFTER_MOVE
