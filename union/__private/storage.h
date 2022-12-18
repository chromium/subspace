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

#include "mem/move.h"
#include "tuple/tuple.h"
#include "union/__private/pack_index.h"

namespace sus::union_type::__private {

template <class... Ts>
struct UnwrapSingleTuple;

template <class T>
struct UnwrapSingleTuple<::sus::Tuple<T>> {
  using type = T;
};

template <class... Ts>
  requires(sizeof...(Ts) > 1)
struct UnwrapSingleTuple<::sus::Tuple<Ts...>> {
  using type = ::sus::Tuple<Ts...>;
};

/// The Storage class stores Tuples internally, but for Tuples of a single
/// object, they are stored and accessed as the interior object.
///
/// For example:
/// * Storage of `Tuple<i32, u32>` will be `Tuple<i32, u32>`.
/// * Storage of `Tuple<i32>` will be just `i32`.
template <size_t I, class... Ts>
using StorageTypeOfTag = UnwrapSingleTuple<PackIth<I, Ts...>>::type;

template <size_t I, class... Elements>
union Storage;

template <size_t I, class... Ts, class... Elements>
  requires(sizeof...(Ts) > 0 && sizeof...(Elements) > 0)
union Storage<I, ::sus::Tuple<Ts...>, Elements...> {
  Storage() {}
  ~Storage() {}

  using Type = ::sus::Tuple<Ts...>;

  inline void construct(Type&& tuple) {
    new (&tuple_) Type(::sus::move(tuple));
  }
  inline constexpr void set(Type&& tuple) { tuple_ = ::sus::move(tuple); }
  inline void move_construct(size_t tag, Storage&& from) {
    if (tag == I) {
      new (&tuple_) Type(::sus::move(from.tuple_));
    } else {
      more_.move_construct(tag, ::sus::move(from.more_));
    }
  }
  inline constexpr void move_assign(size_t tag, Storage&& from) {
    if (tag == I) {
      tuple_ = ::sus::move(from.tuple_);
    } else {
      more_.move_assign(tag, ::sus::move(from.more_));
    }
  }
  inline void copy_construct(size_t tag, const Storage& from) {
    if (tag == I) {
      new (&tuple_) Type(from.tuple_);
    } else {
      more_.copy_construct(tag, from.more_);
    }
  }
  inline constexpr void copy_assign(size_t tag, const Storage& from) {
    if (tag == I) {
      tuple_ = from.tuple_;
    } else {
      more_.copy_assign(tag, from.more_);
    }
  }
  inline void clone_construct(size_t tag, const Storage& from) {
    if (tag == I) {
      new (&tuple_) Type(::sus::clone(from.tuple_));
    } else {
      more_.clone_construct(tag, from.more_);
    }
  }
  inline constexpr void destroy(size_t tag) {
    if (tag == I) {
      tuple_.~Type();
    } else {
      more_.destroy(tag);
    }
  }
  inline constexpr bool eq(size_t tag, const Storage& other) const& {
    if (tag == I) {
      return tuple_ == other.tuple_;
    } else {
      return more_.eq(tag, other.more_);
    }
  }
  inline constexpr std::strong_ordering ord(size_t tag,
                                            const Storage& other) const& {
    if (tag == I) {
      return std::strong_order(tuple_, other.tuple_);
    } else {
      return more_.ord(tag, other.more_);
    }
  }
  inline constexpr std::weak_ordering weak_ord(size_t tag,
                                               const Storage& other) const& {
    if (tag == I) {
      return std::weak_order(tuple_, other.tuple_);
    } else {
      return more_.weak_ord(tag, other.more_);
    }
  }
  inline constexpr std::partial_ordering partial_ord(
      size_t tag, const Storage& other) const& {
    if (tag == I) {
      return std::partial_order(tuple_, other.tuple_);
    } else {
      return more_.partial_ord(tag, other.more_);
    }
  }

  constexpr auto get_ref() const& {
    return [this]<size_t... Is>(std::index_sequence<Is...>) {
      return ::sus::Tuple<const std::remove_reference_t<Ts>&...>::with(
          tuple_.template get_ref<Is>()...);
    }
    (std::make_index_sequence<sizeof...(Ts)>());
  }
  constexpr decltype(auto) get_ref() && = delete;
  constexpr auto get_mut() & {
    return [this]<size_t... Is>(std::index_sequence<Is...>) {
      return ::sus::Tuple<Ts&...>::with(tuple_.template get_mut<Is>()...);
    }
    (std::make_index_sequence<sizeof...(Ts)>());
  }
  inline constexpr auto into_inner() && { return ::sus::move(tuple_); }

  Type tuple_;
  Storage<I + 1, Elements...> more_;
};

template <size_t I, class T, class... Elements>
  requires(sizeof...(Elements) > 0)
union Storage<I, ::sus::Tuple<T>, Elements...> {
  Storage() {}
  ~Storage() {}

  using Type = ::sus::Tuple<T>;

  template<class U>
  inline void construct(U&& value) {
    new (&tuple_)::sus::Tuple<T>(::sus::Tuple<T>::with(::sus::forward<U>(value)));
  }
  inline constexpr void set(T&& value) {
    tuple_ = Type::with(::sus::move(value));
  }
  inline void move_construct(size_t tag, Storage&& from) {
    if (tag == I) {
      new (&tuple_) Type(::sus::move(from.tuple_));
    } else {
      more_.move_construct(tag, ::sus::move(from.more_));
    }
  }
  inline constexpr void move_assign(size_t tag, Storage&& from) {
    if (tag == I) {
      tuple_ = ::sus::move(from.tuple_);
    } else {
      more_.move_assign(tag, ::sus::move(from.more_));
    }
  }
  inline void copy_construct(size_t tag, const Storage& from) {
    if (tag == I) {
      new (&tuple_) Type(from.tuple_);
    } else {
      more_.copy_construct(tag, from.more_);
    }
  }
  inline constexpr void copy_assign(size_t tag, const Storage& from) {
    if (tag == I) {
      tuple_ = from.tuple_;
    } else {
      more_.copy_assign(tag, from.more_);
    }
  }
  inline void clone_construct(size_t tag, const Storage& from) {
    if (tag == I) {
      auto x = ::sus::clone(from.tuple_);
      new (&tuple_) Type(sus::move(x));
    } else {
      more_.clone_construct(tag, from.more_);
    }
  }
  inline constexpr void destroy(size_t tag) {
    if (tag == I) {
      tuple_.~Type();
    } else {
      more_.destroy(tag);
    }
  }
  inline constexpr bool eq(size_t tag, const Storage& other) const& {
    if (tag == I) {
      return tuple_ == other.tuple_;
    } else {
      return more_.eq(tag, other.more_);
    }
  }
  inline constexpr auto ord(size_t tag, const Storage& other) const& {
    if (tag == I) {
      return std::strong_order(tuple_, other.tuple_);
    } else {
      return more_.ord(tag, other.more_);
    }
  }
  inline constexpr auto weak_ord(size_t tag, const Storage& other) const& {
    if (tag == I) {
      return std::weak_order(tuple_, other.tuple_);
    } else {
      return more_.weak_ord(tag, other.more_);
    }
  }
  inline constexpr auto partial_ord(size_t tag, const Storage& other) const& {
    if (tag == I) {
      return std::partial_order(tuple_, other.tuple_);
    } else {
      return more_.partial_ord(tag, other.more_);
    }
  }

  inline constexpr decltype(auto) get_ref() const& {
    return tuple_.template get_ref<0>();
  }
  constexpr decltype(auto) get_ref() && = delete;
  inline constexpr decltype(auto) get_mut() & {
    return tuple_.template get_mut<0>();
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
  requires(sizeof...(Ts) > 0)
union Storage<I, ::sus::Tuple<Ts...>> {
  Storage() {}
  ~Storage() {}

  using Type = ::sus::Tuple<Ts...>;

  inline void construct(Type&& tuple) {
    new (&tuple_) Type(::sus::move(tuple));
  }
  inline constexpr void set(Type&& tuple) { tuple_ = ::sus::move(tuple); }
  inline void move_construct(size_t tag, Storage&& from) {
    ::sus::check(tag == I);
    new (&tuple_) Type(::sus::move(from.tuple_));
  }
  inline constexpr void move_assign(size_t tag, Storage&& from) {
    ::sus::check(tag == I);
    tuple_ = ::sus::move(from.tuple_);
  }
  inline void copy_construct(size_t tag, const Storage& from) {
    ::sus::check(tag == I);
    new (&tuple_) Type(from.tuple_);
  }
  inline constexpr void copy_assign(size_t tag, const Storage& from) {
    ::sus::check(tag == I);
    tuple_ = from.tuple_;
  }
  inline void clone_construct(size_t tag, const Storage& from) {
    ::sus::check(tag == I);
    new (&tuple_) Type(::sus::clone(from.tuple_));
  }
  inline constexpr void destroy(size_t tag) {
    ::sus::check(tag == I);
    tuple_.~Type();
  }
  inline constexpr bool eq(size_t tag, const Storage& other) const& {
    ::sus::check(tag == I);
    return tuple_ == other.tuple_;
  }
  inline constexpr auto ord(size_t tag, const Storage& other) const& {
    ::sus::check(tag == I);
    return std::strong_order(tuple_, other.tuple_);
  }
  inline constexpr auto weak_ord(size_t tag, const Storage& other) const& {
    ::sus::check(tag == I);
    return std::weak_order(tuple_, other.tuple_);
  }
  inline constexpr auto partial_ord(size_t tag, const Storage& other) const& {
    ::sus::check(tag == I);
    return std::partial_order(tuple_, other.tuple_);
  }

  constexpr auto get_ref() const& {
    return [this]<size_t... Is>(std::index_sequence<Is...>) {
      return ::sus::Tuple<const std::remove_reference_t<Ts>&...>::with(
          tuple_.template get_ref<Is>()...);
    }
    (std::make_index_sequence<sizeof...(Ts)>());
  }
  constexpr auto get_mut() & {
    return [this]<size_t... Is>(std::index_sequence<Is...>) {
      return ::sus::Tuple<Ts&...>::with(tuple_.template get_mut<Is>()...);
    }
    (std::make_index_sequence<sizeof...(Ts)>());
  }
  inline constexpr auto into_inner() && { return ::sus::move(tuple_); }

  [[sus_no_unique_address]] Type tuple_;
};

template <size_t I, class T>
union Storage<I, ::sus::Tuple<T>> {
  Storage() {}
  ~Storage() {}

  using Type = ::sus::Tuple<T>;

  template<class U>
  inline void construct(U&& value) {
    new (&tuple_)::sus::Tuple<T>(::sus::Tuple<T>::with(::sus::forward<U>(value)));
  }
  inline constexpr void set(T&& value) {
    tuple_ = Type::with(::sus::move(value));
  }
  inline void move_construct(size_t tag, Storage&& from) {
    ::sus::check(tag == I);
    new (&tuple_) Type(::sus::move(from.tuple_));
  }
  inline constexpr void move_assign(size_t tag, Storage&& from) {
    ::sus::check(tag == I);
    tuple_ = ::sus::move(from.tuple_);
  }
  inline void copy_construct(size_t tag, const Storage& from) {
    ::sus::check(tag == I);
    new (&tuple_) Type(from.tuple_);
  }
  inline constexpr void copy_assign(size_t tag, const Storage& from) {
    ::sus::check(tag == I);
    tuple_ = from.tuple_;
  }
  inline void clone_construct(size_t tag, const Storage& from) {
    ::sus::check(tag == I);
    new (&tuple_) Type(::sus::clone(from.tuple_));
  }
  inline constexpr void destroy(size_t tag) {
    ::sus::check(tag == I);
    tuple_.~Type();
  }
  inline constexpr bool eq(size_t tag, const Storage& other) const& {
    ::sus::check(tag == I);
    return tuple_ == other.tuple_;
  }
  inline constexpr auto ord(size_t tag, const Storage& other) const& {
    ::sus::check(tag == I);
    return std::strong_order(tuple_, other.tuple_);
  }
  inline constexpr auto weak_ord(size_t tag, const Storage& other) const& {
    ::sus::check(tag == I);
    return std::weak_order(tuple_, other.tuple_);
  }
  inline constexpr auto partial_ord(size_t tag, const Storage& other) const& {
    ::sus::check(tag == I);
    return std::partial_order(tuple_, other.tuple_);
  }

  inline constexpr decltype(auto) get_ref() const& {
    return tuple_.template get_ref<0>();
  }
  inline constexpr decltype(auto) get_mut() & {
    return tuple_.template get_mut<0>();
  }
  inline constexpr decltype(auto) into_inner() && {
    return ::sus::move(tuple_).template into_inner<0>();
  }

  // TODO: Switch away from Tuple for 1 object when we don't need to use the
  // use-after-move checks there.
  [[sus_no_unique_address]] ::sus::Tuple<T> tuple_;
};

template <auto I, class S>
static constexpr const auto& find_storage(const S& storage) {
  return find_storage(storage, std::integral_constant<size_t, size_t{I}>());
}

template <size_t I, class S>
static constexpr const auto& find_storage(const S& storage,
                                          std::integral_constant<size_t, I>) {
  return find_storage(storage.more_, std::integral_constant<size_t, I - 1u>());
}

template <class S>
static constexpr const auto& find_storage(const S& storage,
                                          std::integral_constant<size_t, 0>) {
  return storage;
}

template <auto I, class S>
static constexpr auto& find_storage_mut(S& storage) {
  return find_storage_mut(storage, std::integral_constant<size_t, size_t{I}>());
}

template <size_t I, class S>
static constexpr auto& find_storage_mut(S& storage,
                                        std::integral_constant<size_t, I>) {
  return find_storage_mut(storage.more_,
                          std::integral_constant<size_t, I - 1u>());
}

template <class S>
static constexpr auto& find_storage_mut(S& storage,
                                        std::integral_constant<size_t, 0>) {
  return storage;
}

}  // namespace sus::union_type::__private
