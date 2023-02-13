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
#include <type_traits>

#include "subspace/choice/__private/nothing.h"
#include "subspace/choice/__private/pack_index.h"
#include "subspace/macros/no_unique_address.h"
#include "subspace/mem/move.h"
#include "subspace/tuple/tuple.h"

namespace sus::choice_type::__private {

template <class StorageType>
concept ValueIsVoid = std::same_as<Nothing, StorageType>;

template <class StorageType>
concept ValueIsNotVoid = !ValueIsVoid<StorageType>;

template <class... Ts>
struct MakeStorageType {
  using type = ::sus::Tuple<Ts...>;
};

template <>
struct MakeStorageType<void> {
  using type = Nothing;
};

template <class... Ts>
struct StorageTypeOfTagHelper;

template <>
struct StorageTypeOfTagHelper<Nothing> {
  using type = Nothing;
};

template <class T>
struct StorageTypeOfTagHelper<::sus::Tuple<T>> {
  using type = T;
};

template <class... Ts>
  requires(sizeof...(Ts) > 1)
struct StorageTypeOfTagHelper<::sus::Tuple<Ts...>> {
  using type = ::sus::Tuple<Ts...>;
};

/// The Storage class stores Tuples internally, but for Tuples of a single
/// object, they are stored and accessed as the interior object.
///
/// For example:
/// * Storage of `Tuple<i32, u32>` will be `Tuple<i32, u32>`.
/// * Storage of `Tuple<i32>` will be just `i32`.
template <size_t I, class... Ts>
using StorageTypeOfTag = StorageTypeOfTagHelper<PackIth<I, Ts...>>::type;

template <size_t I, class... Elements>
union Storage;

template <size_t I, class... Ts, class... Elements>
  requires(sizeof...(Ts) > 1 && sizeof...(Elements) > 0)
union Storage<I, ::sus::Tuple<Ts...>, Elements...> {
  Storage() {}
  ~Storage()
    requires(std::is_trivially_destructible_v<::sus::Tuple<Ts...>> && ... &&
             std::is_trivially_destructible_v<Elements>)
  = default;
  ~Storage()
    requires(!(std::is_trivially_destructible_v<::sus::Tuple<Ts...>> && ... &&
               std::is_trivially_destructible_v<Elements>))
  {}

  Storage(const Storage&)
    requires(std::is_trivially_copy_constructible_v<::sus::Tuple<Ts...>> &&
             ... && std::is_trivially_copy_constructible_v<Elements>)
  = default;
  Storage& operator=(const Storage&)
    requires(std::is_trivially_copy_assignable_v<::sus::Tuple<Ts...>> && ... &&
             std::is_trivially_copy_assignable_v<Elements>)
  = default;
  Storage(Storage&&)
    requires(std::is_trivially_move_constructible_v<::sus::Tuple<Ts...>> &&
             ... && std::is_trivially_move_constructible_v<Elements>)
  = default;
  Storage& operator=(Storage&&)
    requires(std::is_trivially_move_assignable_v<::sus::Tuple<Ts...>> && ... &&
             std::is_trivially_move_assignable_v<Elements>)
  = default;

  using Type = ::sus::Tuple<Ts...>;

  inline void construct(Type&& tuple) {
    new (&tuple_) Type(::sus::move(tuple));
  }
  inline constexpr void assign(Type&& tuple) { tuple_ = ::sus::move(tuple); }
  inline void move_construct(size_t index, Storage&& from) {
    if (index == I) {
      new (&tuple_) Type(::sus::move(from.tuple_));
    } else {
      new (&more_) decltype(more_)();  // Make the more_ member active.
      more_.move_construct(index, ::sus::move(from.more_));
    }
  }
  inline constexpr void move_assign(size_t index, Storage&& from) {
    if (index == I) {
      tuple_ = ::sus::move(from.tuple_);
    } else {
      more_.move_assign(index, ::sus::move(from.more_));
    }
  }
  inline void copy_construct(size_t index, const Storage& from) {
    if (index == I) {
      new (&tuple_) Type(from.tuple_);
    } else {
      new (&more_) decltype(more_)();  // Make the more_ member active.
      more_.copy_construct(index, from.more_);
    }
  }
  inline constexpr void copy_assign(size_t index, const Storage& from) {
    if (index == I) {
      tuple_ = from.tuple_;
    } else {
      more_.copy_assign(index, from.more_);
    }
  }
  inline void clone_construct(size_t index, const Storage& from) {
    if (index == I) {
      new (&tuple_) Type(::sus::clone(from.tuple_));
    } else {
      new (&more_) decltype(more_)();  // Make the more_ member active.
      more_.clone_construct(index, from.more_);
    }
  }
  inline constexpr void destroy(size_t index) {
    if (index == I) {
      tuple_.~Type();
    } else {
      more_.destroy(index);
      more_.~Storage<I + 1, Elements...>();
    }
  }
  inline constexpr bool eq(size_t index, const Storage& other) const& {
    if (index == I) {
      return tuple_ == other.tuple_;
    } else {
      return more_.eq(index, other.more_);
    }
  }
  inline constexpr std::strong_ordering ord(size_t index,
                                            const Storage& other) const& {
    if (index == I) {
      return std::strong_order(tuple_, other.tuple_);
    } else {
      return more_.ord(index, other.more_);
    }
  }
  inline constexpr std::weak_ordering weak_ord(size_t index,
                                               const Storage& other) const& {
    if (index == I) {
      return std::weak_order(tuple_, other.tuple_);
    } else {
      return more_.weak_ord(index, other.more_);
    }
  }
  inline constexpr std::partial_ordering partial_ord(
      size_t index, const Storage& other) const& {
    if (index == I) {
      return std::partial_order(tuple_, other.tuple_);
    } else {
      return more_.partial_ord(index, other.more_);
    }
  }

  constexpr auto as() const& {
    return [this]<size_t... Is>(std::index_sequence<Is...>) {
      return ::sus::Tuple<const std::remove_reference_t<Ts>&...>::with(
          tuple_.template at<Is>()...);
    }(std::make_index_sequence<sizeof...(Ts)>());
  }
  constexpr auto as_mut() & {
    return [this]<size_t... Is>(std::index_sequence<Is...>) {
      return ::sus::Tuple<Ts&...>::with(tuple_.template at_mut<Is>()...);
    }(std::make_index_sequence<sizeof...(Ts)>());
  }
  inline constexpr auto into_inner() && { return ::sus::move(tuple_); }

  [[sus_no_unique_address]] Type tuple_;
  [[sus_no_unique_address]] Storage<I + 1, Elements...> more_;
};

template <size_t I, class... Elements>
  requires(sizeof...(Elements) > 0)
union Storage<I, Nothing, Elements...> {
  Storage() {}
  ~Storage()
    requires(... && std::is_trivially_destructible_v<Elements>)
  = default;
  ~Storage()
    requires(!(... && std::is_trivially_destructible_v<Elements>))
  {}

  Storage(const Storage&)
    requires(... && std::is_trivially_copy_constructible_v<Elements>)
  = default;
  Storage& operator=(const Storage&)
    requires(... && std::is_trivially_copy_assignable_v<Elements>)
  = default;
  Storage(Storage&&)
    requires(... && std::is_trivially_move_constructible_v<Elements>)
  = default;
  Storage& operator=(Storage&&)
    requires(... && std::is_trivially_move_assignable_v<Elements>)
  = default;

  inline void move_construct(size_t index, Storage&& from) {
    if (index != I) {
      new (&more_) decltype(more_)();  // Make the more_ member active.
      more_.move_construct(index, ::sus::move(from.more_));
    }
  }
  inline constexpr void move_assign(size_t index, Storage&& from) {
    if (index != I) {
      more_.move_assign(index, ::sus::move(from.more_));
    }
  }
  inline void copy_construct(size_t index, const Storage& from) {
    if (index != I) {
      new (&more_) decltype(more_)();  // Make the more_ member active.
      more_.copy_construct(index, from.more_);
    }
  }
  inline constexpr void copy_assign(size_t index, const Storage& from) {
    if (index != I) {
      more_.copy_assign(index, from.more_);
    }
  }
  inline void clone_construct(size_t index, const Storage& from) {
    if (index != I) {
      more_.clone_construct(index, from.more_);
    }
  }
  inline constexpr void destroy(size_t index) {
    if (index != I) {
      more_.destroy(index);
      more_.~Storage<I + 1, Elements...>();
    }
  }
  inline constexpr bool eq(size_t index, const Storage& other) const& {
    if (index == I) {
      return true;
    } else {
      return more_.eq(index, other.more_);
    }
  }
  inline constexpr auto ord(size_t index, const Storage& other) const& {
    if (index == I) {
      return std::strong_ordering::equivalent;
    } else {
      return more_.ord(index, other.more_);
    }
  }
  inline constexpr auto weak_ord(size_t index, const Storage& other) const& {
    if (index == I) {
      return std::weak_ordering::equivalent;
    } else {
      return more_.weak_ord(index, other.more_);
    }
  }
  inline constexpr auto partial_ord(size_t index, const Storage& other) const& {
    if (index == I) {
      return std::partial_ordering::equivalent;
    } else {
      return more_.partial_ord(index, other.more_);
    }
  }

  [[sus_no_unique_address]] Storage<I + 1, Elements...> more_;
};

template <size_t I, class T, class... Elements>
  requires(sizeof...(Elements) > 0)
union Storage<I, ::sus::Tuple<T>, Elements...> {
  Storage() {}
  ~Storage()
    requires(std::is_trivially_destructible_v<::sus::Tuple<T>> && ... &&
             std::is_trivially_destructible_v<Elements>)
  = default;
  ~Storage()
    requires(!(std::is_trivially_destructible_v<::sus::Tuple<T>> && ... &&
               std::is_trivially_destructible_v<Elements>))
  {}

  Storage(const Storage&)
    requires(std::is_trivially_copy_constructible_v<::sus::Tuple<T>> && ... &&
             std::is_trivially_copy_constructible_v<Elements>)
  = default;
  Storage& operator=(const Storage&)
    requires(std::is_trivially_copy_assignable_v<::sus::Tuple<T>> && ... &&
             std::is_trivially_copy_assignable_v<Elements>)
  = default;
  Storage(Storage&&)
    requires(std::is_trivially_move_constructible_v<::sus::Tuple<T>> && ... &&
             std::is_trivially_move_constructible_v<Elements>)
  = default;
  Storage& operator=(Storage&&)
    requires(std::is_trivially_move_assignable_v<::sus::Tuple<T>> && ... &&
             std::is_trivially_move_assignable_v<Elements>)
  = default;

  using Type = ::sus::Tuple<T>;

  template <class U>
  inline void construct(U&& value) {
    new (&tuple_)::sus::Tuple<T>(
        ::sus::Tuple<T>::with(::sus::forward<U>(value)));
  }
  inline constexpr void assign(T&& value) {
    tuple_ = Type::with(::sus::move(value));
  }
  inline void move_construct(size_t index, Storage&& from) {
    if (index == I) {
      new (&tuple_) Type(::sus::move(from.tuple_));
    } else {
      new (&more_) decltype(more_)();  // Make the more_ member active.
      more_.move_construct(index, ::sus::move(from.more_));
    }
  }
  inline constexpr void move_assign(size_t index, Storage&& from) {
    if (index == I) {
      tuple_ = ::sus::move(from.tuple_);
    } else {
      more_.move_assign(index, ::sus::move(from.more_));
    }
  }
  inline void copy_construct(size_t index, const Storage& from) {
    if (index == I) {
      new (&tuple_) Type(from.tuple_);
    } else {
      new (&more_) decltype(more_)();  // Make the more_ member active.
      more_.copy_construct(index, from.more_);
    }
  }
  inline constexpr void copy_assign(size_t index, const Storage& from) {
    if (index == I) {
      tuple_ = from.tuple_;
    } else {
      more_.copy_assign(index, from.more_);
    }
  }
  inline void clone_construct(size_t index, const Storage& from) {
    if (index == I) {
      auto x = ::sus::clone(from.tuple_);
      new (&tuple_) Type(sus::move(x));
    } else {
      new (&more_) decltype(more_)();  // Make the more_ member active.
      more_.clone_construct(index, from.more_);
    }
  }
  inline constexpr void destroy(size_t index) {
    if (index == I) {
      tuple_.~Type();
    } else {
      more_.destroy(index);
      more_.~Storage<I + 1, Elements...>();
    }
  }
  inline constexpr bool eq(size_t index, const Storage& other) const& {
    if (index == I) {
      return tuple_ == other.tuple_;
    } else {
      return more_.eq(index, other.more_);
    }
  }
  inline constexpr auto ord(size_t index, const Storage& other) const& {
    if (index == I) {
      return std::strong_order(tuple_, other.tuple_);
    } else {
      return more_.ord(index, other.more_);
    }
  }
  inline constexpr auto weak_ord(size_t index, const Storage& other) const& {
    if (index == I) {
      return std::weak_order(tuple_, other.tuple_);
    } else {
      return more_.weak_ord(index, other.more_);
    }
  }
  inline constexpr auto partial_ord(size_t index, const Storage& other) const& {
    if (index == I) {
      return std::partial_order(tuple_, other.tuple_);
    } else {
      return more_.partial_ord(index, other.more_);
    }
  }

  inline constexpr decltype(auto) as() const& {
    return tuple_.template at<0>();
  }
  inline constexpr decltype(auto) as_mut() & {
    return tuple_.template at_mut<0>();
  }
  inline constexpr decltype(auto) into_inner() && {
    return ::sus::move(tuple_).template into_inner<0>();
  }

  // TODO: Switch away from Tuple for 1 object when we don't need to use the
  // use-after-move checks there.
  [[sus_no_unique_address]] Type tuple_;
  [[sus_no_unique_address]] Storage<I + 1, Elements...> more_;
};

template <size_t I, class... Ts>
  requires(sizeof...(Ts) > 1)
union Storage<I, ::sus::Tuple<Ts...>> {
  Storage() {}
  ~Storage()
    requires(std::is_trivially_destructible_v<::sus::Tuple<Ts...>>)
  = default;
  ~Storage()
    requires(!(std::is_trivially_destructible_v<::sus::Tuple<Ts...>>))
  {}

  Storage(const Storage&)
    requires(std::is_trivially_copy_constructible_v<::sus::Tuple<Ts...>>)
  = default;
  Storage& operator=(const Storage&)
    requires(std::is_trivially_copy_assignable_v<::sus::Tuple<Ts...>>)
  = default;
  Storage(Storage&&)
    requires(std::is_trivially_move_constructible_v<::sus::Tuple<Ts...>>)
  = default;
  Storage& operator=(Storage&&)
    requires(std::is_trivially_move_assignable_v<::sus::Tuple<Ts...>>)
  = default;

  using Type = ::sus::Tuple<Ts...>;

  inline void construct(Type&& tuple) {
    new (&tuple_) Type(::sus::move(tuple));
  }
  inline constexpr void assign(Type&& tuple) { tuple_ = ::sus::move(tuple); }
  inline void move_construct(size_t index, Storage&& from) {
    ::sus::check(index == I);
    new (&tuple_) Type(::sus::move(from.tuple_));
  }
  inline constexpr void move_assign(size_t index, Storage&& from) {
    ::sus::check(index == I);
    tuple_ = ::sus::move(from.tuple_);
  }
  inline void copy_construct(size_t index, const Storage& from) {
    ::sus::check(index == I);
    new (&tuple_) Type(from.tuple_);
  }
  inline constexpr void copy_assign(size_t index, const Storage& from) {
    ::sus::check(index == I);
    tuple_ = from.tuple_;
  }
  inline void clone_construct(size_t index, const Storage& from) {
    ::sus::check(index == I);
    new (&tuple_) Type(::sus::clone(from.tuple_));
  }
  inline constexpr void destroy(size_t index) {
    ::sus::check(index == I);
    tuple_.~Type();
  }
  inline constexpr bool eq(size_t index, const Storage& other) const& {
    ::sus::check(index == I);
    return tuple_ == other.tuple_;
  }
  inline constexpr auto ord(size_t index, const Storage& other) const& {
    ::sus::check(index == I);
    return std::strong_order(tuple_, other.tuple_);
  }
  inline constexpr auto weak_ord(size_t index, const Storage& other) const& {
    ::sus::check(index == I);
    return std::weak_order(tuple_, other.tuple_);
  }
  inline constexpr auto partial_ord(size_t index, const Storage& other) const& {
    ::sus::check(index == I);
    return std::partial_order(tuple_, other.tuple_);
  }

  constexpr auto as() const& {
    return [this]<size_t... Is>(std::index_sequence<Is...>) {
      return ::sus::Tuple<const std::remove_reference_t<Ts>&...>::with(
          tuple_.template at<Is>()...);
    }(std::make_index_sequence<sizeof...(Ts)>());
  }
  constexpr auto as_mut() & {
    return [this]<size_t... Is>(std::index_sequence<Is...>) {
      return ::sus::Tuple<Ts&...>::with(tuple_.template at_mut<Is>()...);
    }(std::make_index_sequence<sizeof...(Ts)>());
  }
  inline constexpr auto into_inner() && { return ::sus::move(tuple_); }

  [[sus_no_unique_address]] Type tuple_;
};

template <size_t I>
union Storage<I, Nothing> {
  Storage() {}

  inline void move_construct(size_t index, Storage&&) {
    ::sus::check(index == I);
  }
  inline constexpr void move_assign(size_t index, Storage&&) {
    ::sus::check(index == I);
  }
  inline void copy_construct(size_t index, const Storage&) {
    ::sus::check(index == I);
  }
  inline constexpr void copy_assign(size_t index, const Storage&) {
    ::sus::check(index == I);
  }
  inline void clone_construct(size_t index, const Storage&) {
    ::sus::check(index == I);
  }
  inline constexpr void destroy(size_t index) { ::sus::check(index == I); }
  inline constexpr bool eq(size_t index, const Storage&) const& {
    ::sus::check(index == I);
    return true;
  }
  inline constexpr auto ord(size_t index, const Storage&) const& {
    ::sus::check(index == I);
    return std::strong_ordering::equivalent;
  }
  inline constexpr auto weak_ord(size_t index, const Storage&) const& {
    ::sus::check(index == I);
    return std::weak_ordering::equivalent;
  }
  inline constexpr auto partial_ord(size_t index, const Storage&) const& {
    ::sus::check(index == I);
    return std::partial_ordering::equivalent;
  }
};

template <size_t I, class T>
union Storage<I, ::sus::Tuple<T>> {
  Storage() {}
  ~Storage()
    requires(std::is_trivially_destructible_v<::sus::Tuple<T>>)
  = default;
  ~Storage()
    requires(!(std::is_trivially_destructible_v<::sus::Tuple<T>>))
  {}

  Storage(const Storage&)
    requires(std::is_trivially_copy_constructible_v<::sus::Tuple<T>>)
  = default;
  Storage& operator=(const Storage&)
    requires(std::is_trivially_copy_assignable_v<::sus::Tuple<T>>)
  = default;
  Storage(Storage&&)
    requires(std::is_trivially_move_constructible_v<::sus::Tuple<T>>)
  = default;
  Storage& operator=(Storage&&)
    requires(std::is_trivially_move_assignable_v<::sus::Tuple<T>>)
  = default;

  using Type = ::sus::Tuple<T>;

  template <class U>
  inline void construct(U&& value) {
    new (&tuple_)::sus::Tuple<T>(
        ::sus::Tuple<T>::with(::sus::forward<U>(value)));
  }
  inline constexpr void assign(T&& value) {
    tuple_ = Type::with(::sus::move(value));
  }
  inline void move_construct(size_t index, Storage&& from) {
    ::sus::check(index == I);
    new (&tuple_) Type(::sus::move(from.tuple_));
  }
  inline constexpr void move_assign(size_t index, Storage&& from) {
    ::sus::check(index == I);
    tuple_ = ::sus::move(from.tuple_);
  }
  inline void copy_construct(size_t index, const Storage& from) {
    ::sus::check(index == I);
    new (&tuple_) Type(from.tuple_);
  }
  inline constexpr void copy_assign(size_t index, const Storage& from) {
    ::sus::check(index == I);
    tuple_ = from.tuple_;
  }
  inline void clone_construct(size_t index, const Storage& from) {
    ::sus::check(index == I);
    new (&tuple_) Type(::sus::clone(from.tuple_));
  }
  inline constexpr void destroy(size_t index) {
    ::sus::check(index == I);
    tuple_.~Type();
  }
  inline constexpr bool eq(size_t index, const Storage& other) const& {
    ::sus::check(index == I);
    return tuple_ == other.tuple_;
  }
  inline constexpr auto ord(size_t index, const Storage& other) const& {
    ::sus::check(index == I);
    return std::strong_order(tuple_, other.tuple_);
  }
  inline constexpr auto weak_ord(size_t index, const Storage& other) const& {
    ::sus::check(index == I);
    return std::weak_order(tuple_, other.tuple_);
  }
  inline constexpr auto partial_ord(size_t index, const Storage& other) const& {
    ::sus::check(index == I);
    return std::partial_order(tuple_, other.tuple_);
  }

  inline constexpr decltype(auto) as() const& {
    return tuple_.template at<0>();
  }
  inline constexpr decltype(auto) as_mut() & {
    return tuple_.template at_mut<0>();
  }
  inline constexpr decltype(auto) into_inner() && {
    return ::sus::move(tuple_).template into_inner<0>();
  }

  // TODO: Switch away from Tuple for 1 object when we don't need to use the
  // use-after-move checks there.
  [[sus_no_unique_address]] ::sus::Tuple<T> tuple_;
};

template <auto I, class S>
static constexpr const auto& find_choice_storage(const S& storage) {
  return find_choice_storage(storage,
                             std::integral_constant<size_t, size_t{I}>());
}

template <size_t I, class S>
static constexpr const auto& find_choice_storage(
    const S& storage, std::integral_constant<size_t, I>) {
  return find_choice_storage(storage.more_,
                             std::integral_constant<size_t, I - 1u>());
}

template <class S>
static constexpr const auto& find_choice_storage(
    const S& storage, std::integral_constant<size_t, 0>) {
  return storage;
}

template <auto I, class S>
static constexpr auto& find_choice_storage_mut(S& storage) {
  return find_choice_storage_mut(storage,
                                 std::integral_constant<size_t, size_t{I}>());
}

template <size_t I, class S>
static constexpr auto& find_choice_storage_mut(
    S& storage, std::integral_constant<size_t, I>) {
  return find_choice_storage_mut(storage.more_,
                                 std::integral_constant<size_t, I - 1u>());
}

template <class S>
static constexpr auto& find_choice_storage_mut(
    S& storage, std::integral_constant<size_t, 0>) {
  return storage;
}

}  // namespace sus::choice_type::__private
