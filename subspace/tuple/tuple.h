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
#include "construct/default.h"
#include "macros/no_unique_address.h"
#include "mem/clone.h"
#include "mem/copy.h"
#include "mem/forward.h"
#include "mem/replace.h"
#include "num/num_concepts.h"
#include "ops/eq.h"
#include "ops/ord.h"
#include "tuple/__private/storage.h"

#define SUS_CONFIG_TUPLE_USE_AFTER_MOVE true

namespace sus::tuple_type {

/// A Tuple is a finate sequence of one or more heterogeneous values.
///
/// # Tail padding
/// The Tuple's tail padding may be reused when the Tuple is marked as
/// `[[no_unique_address]]`. The Tuple will have tail padding if the first
/// type has a size that is not a multiple of the Tuple's alignment. For
/// example if it's smaller than the alignment, such as `Tuple<u8, u64>` which
/// has `(alignof(u64) == sizeof(u64)) - sizeof(u8)` or 7 bytes of tail padding.
///
/// ```
/// struct S {
///   [[no_unique_address]] Tuple<u32, u64> tuple;  // 16 bytes.
///   u32 val;  // 4 bytes.
/// };  // 16 bytes, since `val` is stored inside `tuple`.
/// ```
///
/// However note that this behaviour is compiler-dependent, and MSVC does not
/// use the `[[no_unique_address]]` hint.
///
/// Use `sus::data_size_of<T>()` to determine the size of T excluding its tail
/// padding (so `sus::size_of<T>() - sus::data_size_of<T>()` is the tail
/// padding), which can be useful to ensure you have the expected behaviour from
/// your types.
///
/// Additionally types within the tuple may be placed inside the tail padding of
/// other types in the tuple, should such padding exist.
///
/// Generally, but not always, use of tail padding in Tuple is optimized by
/// ordering types (left-to-right in the template variables) from smallest-to-
/// largest for simple types such as integers (which have no tail padding
/// themselves), or in least-to-most tail-padding for more complex types.
/// Elements in a Tuple are stored internally in reverse of the order they are
/// specified, which is why the size of the *first* element matters for the
/// Tuple's externally usable tail padding.
///
/// # Use after move
/// Tuples optionally compile in checks against use-after-move to protect
/// against using their internal values after they are moved. It is on by
/// default and behind the `SUS_CONFIG_TUPLE_USE_AFTER_MOVE` define which can be
/// true or false. When on, all Tuples are an extra 8 bytes larger.
template <class T, class... Ts>
class Tuple final {
 public:
  static constexpr auto protects_uam = SUS_CONFIG_TUPLE_USE_AFTER_MOVE;

  /// Construct a Tuple with the default value for the types it contains.
  ///
  /// The Tuple's contained types must all be #Default, and will be
  /// constructed through that trait.
  inline constexpr Tuple() noexcept
    requires((::sus::construct::Default<T> && ... &&
              ::sus::construct::Default<Ts>))
      : Tuple(T(), Ts()...) {}

  /// Construct a Tuple with the given values.
  template <std::convertible_to<T> U, std::convertible_to<Ts>... Us>
    requires(sizeof...(Us) == sizeof...(Ts))
  constexpr inline static Tuple with(U&& first, Us&&... more) noexcept {
    return Tuple(::sus::forward<U>(first), ::sus::forward<Us>(more)...);
  }

  /// sus::mem::Clone trait.
  constexpr Tuple clone() const& noexcept
    requires((::sus::mem::CloneOrRef<T> && ... && ::sus::mem::CloneOrRef<Ts>) &&
             !(::sus::mem::CopyOrRef<T> && ... && ::sus::mem::CopyOrRef<Ts>))
  {
    ::sus::check(!any_moved_from());
    auto f = [this]<size_t... Is>(std::index_sequence<Is...>) {
      return Tuple::with(::sus::mem::clone_or_forward<T>(
          __private::find_storage<Is>(storage_).get_ref())...);
    };
    return f(std::make_index_sequence<1u + sizeof...(Ts)>());
  }

  /// Gets a const reference to the `I`th element in the tuple.
  template <size_t I>
    requires(I <= sizeof...(Ts))
  constexpr inline const auto& get_ref() const& noexcept {
    ::sus::check(!moved_from(I));
    return __private::find_storage<I>(storage_).get_ref();
  }

  /// Disallows getting a reference to temporary Tuple.
  template <size_t I>
  constexpr inline const auto& get_ref() && = delete;

  /// Gets a mutable reference to the `I`th element in the tuple.
  template <size_t I>
    requires(I <= sizeof...(Ts))
  constexpr inline auto& get_mut() & noexcept {
    ::sus::check(!moved_from(I));
    return __private::find_storage_mut<I>(storage_).get_mut();
  }

  /// Removes the `I`th element from the tuple, leaving the Tuple in a
  /// moved-from state where it should no longer be used.
  template <size_t I>
    requires(I <= sizeof...(Ts))
  constexpr inline decltype(auto) into_inner() && noexcept {
    ::sus::check(!moved_from(I));
    set_all_moved_from();
    return ::sus::move(__private::find_storage_mut<I>(storage_)).into_inner();
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
  constexpr inline bool any_moved_from() const noexcept {
#if SUS_CONFIG_TUPLE_USE_AFTER_MOVE
    return marker.any_moved_from();
#else
    return false;
#endif
  }
  constexpr inline bool moved_from(size_t i) const noexcept {
#if SUS_CONFIG_TUPLE_USE_AFTER_MOVE
    return marker.moved_from(i);
#else
    (void)i;
    return false;
#endif
  }
  // TODO: Provide a way to opt out of all moved-from checks?
  constexpr inline void set_all_moved_from() noexcept {
#if SUS_CONFIG_TUPLE_USE_AFTER_MOVE
    marker.set_all_moved_from();
#endif
  }

#if SUS_CONFIG_TUPLE_USE_AFTER_MOVE
  __private::UseAfterMoveMarker marker;
#endif
  // The use of `[[no_unique_address]]` allows the tail padding of of the
  // `storage_` to be used in structs that request to do so by putting
  // `[[no_unique_address]]` on the Tuple. For example, Union does this with its
  // internal Tuples to put its tag inside the Tuples' storage when possible.
  [[sus_no_unique_address]] Storage storage_;
};

// Support for structured binding.
template <size_t I, class... Ts>
decltype(auto) get(const Tuple<Ts...>& t) noexcept {
  return t.template get_ref<I>();
}
template <size_t I, class... Ts>
decltype(auto) get(Tuple<Ts...>& t) noexcept {
  return t.template get_mut<I>();
}
template <size_t I, class... Ts>
decltype(auto) get(Tuple<Ts...>&& t) noexcept {
  // We explicitly don't move-from `t` to call `t.into_inner()` as this `get()`
  // function will be called for each member of `t` when making structured
  // bindings from an rvalue Tuple.
  return static_cast<decltype(::sus::move(t).template into_inner<I>())>(
      t.template get_mut<I>());
}

namespace __private {
template <class... Ts>
struct TupleMarker {
  Tuple<Ts&&...> values;

  template <class... Us>
  inline constexpr operator Tuple<Us...>() && noexcept {
    auto make_tuple =
        [this]<size_t... Is>(std::integer_sequence<size_t, Is...>) {
          return Tuple<Us...>::with(
              ::sus::forward<Ts>(values.template get_mut<Is>())...);
        };
    return make_tuple(std::make_integer_sequence<size_t, sizeof...(Ts)>());
  }
};

}  // namespace __private

/// Used to construct a Tuple<Ts...> with the parameters as its values.
///
/// Calling tuple() produces a hint to make a Tuple<Ts...> but does not actually
/// construct Tuple<Ts...>, as the types in `Ts...` are not known here.
//
// Note: A marker type is used instead of explicitly constructing a tuple
// immediately in order to avoid redundantly having to specify `Ts...` when
// using the result of `sus::tuple()` as a function argument or return value.
template <class... Ts>
  requires(sizeof...(Ts) > 0)
[[nodiscard]] inline constexpr auto tuple(
    Ts&&... vs sus_if_clang([[clang::lifetimebound]])) noexcept {
  return __private::TupleMarker<Ts...>(
      ::sus::tuple_type::Tuple<Ts&&...>::with(::sus::forward<Ts>(vs)...));
}

}  // namespace sus::tuple_type

namespace std {
template <class... Types>
struct tuple_size<::sus::tuple_type::Tuple<Types...>> {
  static constexpr size_t value = sizeof...(Types);
};

template <size_t I, class T, class... Types>
struct tuple_element<I, ::sus::tuple_type::Tuple<T, Types...>> {
  using type = tuple_element<I - 1, ::sus::tuple_type::Tuple<Types...>>::type;
};

template <class T, class... Types>
struct tuple_element<0, ::sus::tuple_type::Tuple<T, Types...>> {
  using type = T;
};

}  // namespace std

// Promote Tuple into the `sus` namespace.
namespace sus {
using ::sus::tuple_type::Tuple;
using ::sus::tuple_type::tuple;
}  // namespace sus

#undef SUS_CONFIG_TUPLE_USE_AFTER_MOVE
