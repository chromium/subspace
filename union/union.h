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

#include <type_traits>

#include "macros/no_unique_address.h"
#include "mem/clone.h"
#include "mem/copy.h"
#include "mem/move.h"
#include "mem/never_value.h"
#include "mem/replace.h"
#include "ops/eq.h"
#include "ops/ord.h"
#include "tuple/tuple.h"
#include "union/__private/tag_type.h"
#include "union/__private/type_list.h"
#include "union/value_types.h"

namespace sus::union_type {

namespace __private {

using ::sus::num::__private::unchecked_add;
using ::sus::num::__private::unchecked_sub;

template <class T, class... Ts>
static constexpr bool AllSameType = (... && std::same_as<T, Ts>);

template <class TagType, auto SearchValue, TagType I, auto... Vs>
struct FindValue;

template <class TagType, auto SearchValue, TagType I, auto V, auto... Vs>
struct FindValue<TagType, SearchValue, I, V, Vs...> {
  using index = std::conditional_t<
      SearchValue == V, std::integral_constant<TagType, I>,
      typename FindValue<TagType, SearchValue, unchecked_add(I, TagType{1u}),
                         Vs...>::index>;
};

template <class TagType, auto SearchValue, TagType I>
struct FindValue<TagType, SearchValue, I> {
  using index = void;  // We didn't find the value, it's not part of the Union.
};

template <class TagType, auto SearchValue, auto... Vs>
using ValueToIndex = FindValue<TagType, SearchValue, 0u, Vs...>::index;

template <size_t I, class... Ts>
struct FindTypeTuple;

template <size_t I, class T, class... Ts>
struct FindTypeTuple<I, T, Ts...> {
  using type = FindTypeTuple<I - 1, Ts...>::type;
};

template <class T, class... Ts>
struct FindTypeTuple<0, T, Ts...> {
  using type = T;
};

template <class... Ts>
struct UnwrapTuple;

template <class T>
struct UnwrapTuple<::sus::Tuple<T>> {
  using type = T;
};

template <class... Ts>
  requires(sizeof...(Ts) > 1)
struct UnwrapTuple<::sus::Tuple<Ts...>> {
  using type = ::sus::Tuple<Ts...>;
};

template <size_t I, class... Ts>
using IndexToTypes = UnwrapTuple<typename FindTypeTuple<I, Ts...>::type>::type;

template <auto... Vs>
struct CheckUnique;

template <auto V>
struct CheckUnique<V> {
  static constexpr bool value = true;
};

template <auto V, auto V2>
struct CheckUnique<V, V2> {
  static constexpr bool value = V != V2;
};

template <auto V, auto V2, auto... Vs>
  requires(sizeof...(Vs) > 0)
struct CheckUnique<V, V2, Vs...> {
  static constexpr bool value =
      V != V2 && CheckUnique<V, Vs...>::value && CheckUnique<V2, Vs...>::value;
};

template <auto... Vs>
concept AllValuesAreUnique = CheckUnique<Vs...>::value;

template <class T, class... Ts>
struct ValuesTypeHelper {
  using type = T;
};

// Compiler won't allow spliting the argument pack with a template alias, so we
// need to defer to a struct.
template <class... Ts>
using ValuesType = typename ValuesTypeHelper<Ts...>::type;

template <class ValueType1, class Types1, class ValueType2, class Types2>
struct UnionIsEqHelper;

template <class ValueType1, class... Types1, class ValueType2, class... Types2>
struct UnionIsEqHelper<ValueType1, TypeList<Types1...>, ValueType2,
                       TypeList<Types2...>> {
  static constexpr bool value = (::sus::ops::Eq<ValueType1, ValueType2> &&
                                 ... && ::sus::ops::Eq<Types1, Types2>);
};

// Out of line from the requires clause, in a struct, to work around
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108067.
template <class ValueType1, class Types1, class ValueType2, class Types2>
concept UnionIsEq =
    UnionIsEqHelper<ValueType1, Types1, ValueType2, Types2>::value;

template <class ValueType1, class Types1, class ValueType2, class Types2>
struct UnionIsOrdHelper;

template <class ValueType1, class... Types1, class ValueType2, class... Types2>
struct UnionIsOrdHelper<ValueType1, TypeList<Types1...>, ValueType2,
                        TypeList<Types2...>> {
  static constexpr bool value = (::sus::ops::Ord<ValueType1, ValueType2> &&
                                 ... && ::sus::ops::Ord<Types1, Types2>);
};

// Out of line from the requires clause, in a struct, to work around
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108067.
template <class ValueType1, class Types1, class ValueType2, class Types2>
concept UnionIsOrd =
    UnionIsOrdHelper<ValueType1, Types1, ValueType2, Types2>::value;

template <class ValueType1, class Types1, class ValueType2, class Types2>
struct UnionIsWeakOrdHelper;

template <class ValueType1, class... Types1, class ValueType2, class... Types2>
struct UnionIsWeakOrdHelper<ValueType1, TypeList<Types1...>, ValueType2,
                            TypeList<Types2...>> {
  // clang-format off
  static constexpr bool value =
      ((!::sus::ops::Ord<ValueType1, ValueType2> || ... ||
        !::sus::ops::Ord<Types1, Types2>)
        &&
       (::sus::ops::WeakOrd<ValueType1, ValueType2> && ... &&
        ::sus::ops::WeakOrd<Types1, Types2>));
  // clang-format on
};

// Out of line from the requires clause, in a struct, to work around
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108067.
template <class ValueType1, class Types1, class ValueType2, class Types2>
concept UnionIsWeakOrd =
    UnionIsWeakOrdHelper<ValueType1, Types1, ValueType2, Types2>::value;

template <class ValueType1, class Types1, class ValueType2, class Types2>
struct UnionIsPartialOrdHelper;

template <class ValueType1, class... Types1, class ValueType2, class... Types2>
struct UnionIsPartialOrdHelper<ValueType1, TypeList<Types1...>, ValueType2,
                               TypeList<Types2...>> {
  // clang-format off
  static constexpr bool value =
      (!::sus::ops::WeakOrd<ValueType1, ValueType2> || ... ||
       !::sus::ops::WeakOrd<Types1, Types2>)
      &&
      (::sus::ops::PartialOrd<ValueType1, ValueType2> && ... &&
       ::sus::ops::PartialOrd<Types1, Types2>);
  // clang-format on
};

// Out of line from the requires clause, in a struct, to work around
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108067.
template <class ValueType1, class Types1, class ValueType2, class Types2>
concept UnionIsPartialOrd =
    UnionIsPartialOrdHelper<ValueType1, Types1, ValueType2, Types2>::value;

template <class ValueType1, class Types1, class ValueType2, class Types2>
struct UnionIsAnyOrdHelper;

template <class ValueType1, class... Types1, class ValueType2, class... Types2>
struct UnionIsAnyOrdHelper<ValueType1, TypeList<Types1...>, ValueType2,
                           TypeList<Types2...>> {
  // clang-format off
  static constexpr bool value =
      (::sus::ops::PartialOrd<ValueType1, ValueType2> && ... &&
       ::sus::ops::PartialOrd<Types1, Types2>);
  // clang-format on
};

// Out of line from the requires clause, in a struct, to work around
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108067.
template <class ValueType1, class Types1, class ValueType2, class Types2>
concept UnionIsAnyOrd =
    UnionIsAnyOrdHelper<ValueType1, Types1, ValueType2, Types2>::value;

template <class... Ts>
concept AllMove = (... && ::sus::mem::Move<Ts>);
template <class... Ts>
concept AllCopy = (... && ::sus::mem::Copy<Ts>);
template <class... Ts>
concept AllClone = (... && ::sus::mem::Clone<Ts>);
template <class... Ts>
concept AllTriviallyDestructible = (... &&
                                    std::is_trivially_destructible_v<Ts>);
template <class... Ts>
concept AllTriviallyMoveConstructible =
    (... && std::is_trivially_move_constructible_v<Ts>);
template <class... Ts>
concept AllTriviallyMoveAssignable = (... &&
                                      std::is_trivially_move_assignable_v<Ts>);
template <class... Ts>
concept AllTriviallyCopyConstructible =
    (... && std::is_trivially_copy_constructible_v<Ts>);
template <class... Ts>
concept AllTriviallyCopyAssignable = (... &&
                                      std::is_trivially_move_assignable_v<Ts>);

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
  inline void move_construct(auto tag, Storage&& from) {
    if (tag == I) {
      new (&tuple_) Type(::sus::move(from.tuple_));
    } else {
      more_.move_construct(tag, ::sus::move(from.more_));
    }
  }
  inline constexpr void move_assign(auto tag, Storage&& from) {
    if (tag == I) {
      tuple_ = ::sus::move(from.tuple_);
    } else {
      more_.move_assign(tag, ::sus::move(from.more_));
    }
  }
  inline void copy_construct(auto tag, const Storage& from) {
    if (tag == I) {
      new (&tuple_) Type(from.tuple_);
    } else {
      more_.copy_construct(tag, from.more_);
    }
  }
  inline constexpr void copy_assign(auto tag, const Storage& from) {
    if (tag == I) {
      tuple_ = from.tuple_;
    } else {
      more_.copy_assign(tag, from.more_);
    }
  }
  inline void clone_construct(auto tag, const Storage& from) {
    if (tag == I) {
      new (&tuple_) Type(::sus::clone(from.tuple_));
    } else {
      more_.clone_construct(tag, from.more_);
    }
  }
  inline constexpr void destroy(auto tag) {
    if (tag == I) {
      tuple_.~Type();
    } else {
      more_.destroy(tag);
    }
  }
  inline constexpr bool eq(auto tag, const Storage& other) const& {
    if (tag == I) {
      return tuple_ == other.tuple_;
    } else {
      return more_.eq(tag, other.more_);
    }
  }
  inline constexpr std::strong_ordering ord(auto tag,
                                            const Storage& other) const& {
    if (tag == I) {
      return std::strong_order(tuple_, other.tuple_);
    } else {
      return more_.ord(tag, other.more_);
    }
  }
  inline constexpr std::weak_ordering weak_ord(auto tag,
                                               const Storage& other) const& {
    if (tag == I) {
      return std::weak_order(tuple_, other.tuple_);
    } else {
      return more_.weak_ord(tag, other.more_);
    }
  }
  inline constexpr std::partial_ordering partial_ord(
      auto tag, const Storage& other) const& {
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
    }(std::make_index_sequence<sizeof...(Ts)>());
  }
  constexpr decltype(auto) get_ref() && = delete;
  constexpr auto get_mut() & {
    return [this]<size_t... Is>(std::index_sequence<Is...>) {
      return ::sus::Tuple<Ts&...>::with(tuple_.template get_mut<Is>()...);
    }(std::make_index_sequence<sizeof...(Ts)>());
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

  inline void construct(T&& value) {
    new (&tuple_) Type(Type::with(::sus::move(value)));
  }
  inline constexpr void set(T&& value) {
    tuple_ = Type::with(::sus::move(value));
  }
  inline void move_construct(auto tag, Storage&& from) {
    if (tag == I) {
      new (&tuple_) Type(::sus::move(from.tuple_));
    } else {
      more_.move_construct(tag, ::sus::move(from.more_));
    }
  }
  inline constexpr void move_assign(auto tag, Storage&& from) {
    if (tag == I) {
      tuple_ = ::sus::move(from.tuple_);
    } else {
      more_.move_assign(tag, ::sus::move(from.more_));
    }
  }
  inline void copy_construct(auto tag, const Storage& from) {
    if (tag == I) {
      new (&tuple_) Type(from.tuple_);
    } else {
      more_.copy_construct(tag, from.more_);
    }
  }
  inline constexpr void copy_assign(auto tag, const Storage& from) {
    if (tag == I) {
      tuple_ = from.tuple_;
    } else {
      more_.copy_assign(tag, from.more_);
    }
  }
  inline void clone_construct(auto tag, const Storage& from) {
    if (tag == I) {
      auto x = ::sus::clone(from.tuple_);
      new (&tuple_) Type(sus::move(x));
    } else {
      more_.clone_construct(tag, from.more_);
    }
  }
  inline constexpr void destroy(auto tag) {
    if (tag == I) {
      tuple_.~Type();
    } else {
      more_.destroy(tag);
    }
  }
  inline constexpr bool eq(auto tag, const Storage& other) const& {
    if (tag == I) {
      return tuple_ == other.tuple_;
    } else {
      return more_.eq(tag, other.more_);
    }
  }
  inline constexpr auto ord(auto tag, const Storage& other) const& {
    if (tag == I) {
      return std::strong_order(tuple_, other.tuple_);
    } else {
      return more_.ord(tag, other.more_);
    }
  }
  inline constexpr auto weak_ord(auto tag, const Storage& other) const& {
    if (tag == I) {
      return std::weak_order(tuple_, other.tuple_);
    } else {
      return more_.weak_ord(tag, other.more_);
    }
  }
  inline constexpr auto partial_ord(auto tag, const Storage& other) const& {
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
    return ::sus::mem::move_or_copy_ref(tuple_.template get_mut<0>());
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
  inline void move_construct(auto tag, Storage&& from) {
    ::sus::check(tag == I);
    new (&tuple_) Type(::sus::move(from.tuple_));
  }
  inline constexpr void move_assign(auto tag, Storage&& from) {
    ::sus::check(tag == I);
    tuple_ = ::sus::move(from.tuple_);
  }
  inline void copy_construct(auto tag, const Storage& from) {
    ::sus::check(tag == I);
    new (&tuple_) Type(from.tuple_);
  }
  inline constexpr void copy_assign(auto tag, const Storage& from) {
    ::sus::check(tag == I);
    tuple_ = from.tuple_;
  }
  inline void clone_construct(auto tag, const Storage& from) {
    ::sus::check(tag == I);
    new (&tuple_) Type(::sus::clone(from.tuple_));
  }
  inline constexpr void destroy(auto tag) {
    ::sus::check(tag == I);
    tuple_.~Type();
  }
  inline constexpr bool eq(auto tag, const Storage& other) const& {
    ::sus::check(tag == I);
    return tuple_ == other.tuple_;
  }
  inline constexpr auto ord(auto tag, const Storage& other) const& {
    ::sus::check(tag == I);
    return std::strong_order(tuple_, other.tuple_);
  }
  inline constexpr auto weak_ord(auto tag, const Storage& other) const& {
    ::sus::check(tag == I);
    return std::weak_order(tuple_, other.tuple_);
  }
  inline constexpr auto partial_ord(auto tag, const Storage& other) const& {
    ::sus::check(tag == I);
    return std::partial_order(tuple_, other.tuple_);
  }

  constexpr auto get_ref() const& {
    return [this]<size_t... Is>(std::index_sequence<Is...>) {
      return ::sus::Tuple<const std::remove_reference_t<Ts>&...>::with(
          tuple_.template get_ref<Is>()...);
    }(std::make_index_sequence<sizeof...(Ts)>());
  }
  constexpr auto get_mut() & {
    return [this]<size_t... Is>(std::index_sequence<Is...>) {
      return ::sus::Tuple<Ts&...>::with(tuple_.template get_mut<Is>()...);
    }(std::make_index_sequence<sizeof...(Ts)>());
  }
  inline constexpr auto into_inner() && { return ::sus::move(tuple_); }

  [[sus_no_unique_address]] Type tuple_;
};

template <size_t I, class T>
union Storage<I, ::sus::Tuple<T>> {
  Storage() {}
  ~Storage() {}

  using Type = ::sus::Tuple<T>;

  inline void construct(T&& value) {
    new (&tuple_)::sus::Tuple<T>(::sus::Tuple<T>::with(::sus::move(value)));
  }
  inline constexpr void set(T&& value) {
    tuple_ = Type::with(::sus::move(value));
  }
  inline void move_construct(auto tag, Storage&& from) {
    ::sus::check(tag == I);
    new (&tuple_) Type(::sus::move(from.tuple_));
  }
  inline constexpr void move_assign(auto tag, Storage&& from) {
    ::sus::check(tag == I);
    tuple_ = ::sus::move(from.tuple_);
  }
  inline void copy_construct(auto tag, const Storage& from) {
    ::sus::check(tag == I);
    new (&tuple_) Type(from.tuple_);
  }
  inline constexpr void copy_assign(auto tag, const Storage& from) {
    ::sus::check(tag == I);
    tuple_ = from.tuple_;
  }
  inline void clone_construct(auto tag, const Storage& from) {
    ::sus::check(tag == I);
    new (&tuple_) Type(::sus::clone(from.tuple_));
  }
  inline constexpr void destroy(auto tag) {
    ::sus::check(tag == I);
    tuple_.~Type();
  }
  inline constexpr bool eq(auto tag, const Storage& other) const& {
    ::sus::check(tag == I);
    return tuple_ == other.tuple_;
  }
  inline constexpr auto ord(auto tag, const Storage& other) const& {
    ::sus::check(tag == I);
    return std::strong_order(tuple_, other.tuple_);
  }
  inline constexpr auto weak_ord(auto tag, const Storage& other) const& {
    ::sus::check(tag == I);
    return std::weak_order(tuple_, other.tuple_);
  }
  inline constexpr auto partial_ord(auto tag, const Storage& other) const& {
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
    return ::sus::mem::move_or_copy_ref(tuple_.template get_mut<0>());
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
  return find_storage(
      storage.more_,
      std::integral_constant<size_t, unchecked_sub(I, size_t{1})>());
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
  return find_storage_mut(
      storage.more_,
      std::integral_constant<size_t, unchecked_sub(I, size_t{1})>());
}

template <class S>
static constexpr auto& find_storage_mut(S& storage,
                                        std::integral_constant<size_t, 0>) {
  return storage;
}

}  // namespace __private

template <class Types, auto... Values>
class Union;

template <class... Ts, auto... Values>
  requires(sizeof...(Ts) > 0u && __private::AllSameType<decltype(Values)...> &&
           ::sus::ops::Eq<__private::ValuesType<decltype(Values)...>> &&
           __private::AllValuesAreUnique<Values...>)
class Union<__private::TypeList<Ts...>, Values...> {
  static_assert(
      sizeof...(Ts) == sizeof...(Values),
      "The number of types and values in the union don't match. Use "
      "`sus_value_types()` to define the Union's value-type pairings.");

  using TagType = __private::TagType<sizeof...(Values) + 2u>;
  static_assert(!std::is_void_v<TagType>,
                "A union can only hold a number of members representable in "
                "size_t (minus two).");
  static constexpr TagType kNeverValue =
      ::sus::num::__private::unchecked_not(TagType{0u});
  static constexpr TagType kUseAfterMove = ::sus::num::__private::unchecked_sub(
      ::sus::num::__private::unchecked_not(TagType{0u}), TagType{1u});
  // The tag holds an index, but never the number of possible values so it is in
  // 0...{NumValues-1}.
  //
  // The ~0 value (all bits are 1) is reserved for using the tag as a
  // never-value field, so the tag must be large enough to hold all values
  // without using that bit sequence.
  //
  // The (~0 - 1) values (all bits are 1 except the lowest order bit) is
  // reserved to mark the type as moved-from. Any use of the type afterward will
  // panic due to the tag being outside the range of acceptable values.
  static_assert(sizeof...(Values) <= kNeverValue);

  using Storage = __private::Storage<0, Ts...>;
  using ValuesType = __private::ValuesType<decltype(Values)...>;

  template <ValuesType V>
  static constexpr TagType get_index() {
    using Index = __private::ValueToIndex<TagType, V, Values...>;
    static_assert(!std::is_void_v<Index>,
                  "The value V is not part of the Union.");
    return Index::value;
  }

  template <ValuesType V>
  static constexpr TagType index = get_index<V>();

  template <ValuesType V>
  using TypesFromValue = __private::IndexToTypes<size_t{index<V>}, Ts...>;

 public:
  // TODO: Can we construct Tuples of trivially constructible things (or some
  // set of things) without placement new and thus make this constexpr?
  template <ValuesType V, class U, int&..., class Arg = TypesFromValue<V>>
    requires(std::convertible_to<U, Arg>)
  static Union with(U values) {
    auto u = Union(index<V>);
    find_storage_mut<index<V>>(u.storage_).construct(::sus::move(values));
    return u;
  }

  ~Union()
    requires(__private::AllTriviallyDestructible<ValuesType, Ts...>)
  = default;
  ~Union()
    requires(!__private::AllTriviallyDestructible<ValuesType, Ts...>)
  {
    if (tag_ != kUseAfterMove) storage_.destroy(tag_);
  }

  Union(Union&& o)
    requires(__private::AllMove<ValuesType, Ts...> &&
             __private::AllTriviallyMoveConstructible<ValuesType, Ts...>)
  = default;
  Union(Union&& o)
    requires(!__private::AllMove<ValuesType, Ts...>)
  = delete;

  Union(Union&& o)
    requires(__private::AllMove<ValuesType, Ts...> &&
             !__private::AllTriviallyMoveConstructible<ValuesType, Ts...>)
      : tag_(::sus::mem::replace(::sus::mref(o.tag_),
                                 // Attempt to catch use-after-move by setting
                                 // the tag to an unused value.
                                 kUseAfterMove)) {
    check(tag_ != kUseAfterMove);
    storage_.move_construct(tag_, ::sus::move(o.storage_));
  }

  Union& operator=(Union&& o)
    requires(__private::AllMove<ValuesType, Ts...> &&
             __private::AllTriviallyMoveAssignable<ValuesType, Ts...>)
  = default;
  Union& operator=(Union&& o)
    requires(!__private::AllMove<ValuesType, Ts...>)
  = delete;

  Union& operator=(Union&& o)
    requires(__private::AllMove<ValuesType, Ts...> &&
             !__private::AllTriviallyMoveAssignable<ValuesType, Ts...>)
  {
    check(o.tag_ != kUseAfterMove);
    if (tag_ == o.tag_) {
      storage_.move_assign(tag_, ::sus::move(o.storage_));
    } else {
      if (tag_ != kUseAfterMove) storage_.destroy(tag_);
      tag_ = ::sus::move(o.tag_);
      storage_.move_construct(tag_, ::sus::move(o.storage_));
    }
    o.tag_ = kUseAfterMove;
    return *this;
  }

  Union(const Union& o)
    requires(__private::AllCopy<ValuesType, Ts...> &&
             __private::AllTriviallyCopyConstructible<ValuesType, Ts...>)
  = default;
  Union(const Union& o)
    requires(!__private::AllCopy<ValuesType, Ts...>)
  = delete;

  Union(const Union& o)
    requires(__private::AllCopy<ValuesType, Ts...> &&
             !__private::AllTriviallyCopyConstructible<ValuesType, Ts...>)
      : tag_(o.tag_) {
    check(o.tag_ != kUseAfterMove);
    storage_.copy_construct(tag_, o.storage_);
  }

  Union& operator=(const Union& o)
    requires(__private::AllCopy<ValuesType, Ts...> &&
             __private::AllTriviallyCopyAssignable<ValuesType, Ts...>)
  = default;
  Union& operator=(const Union& o)
    requires(!__private::AllCopy<ValuesType, Ts...>)
  = delete;

  Union& operator=(const Union& o)
    requires(__private::AllCopy<ValuesType, Ts...> &&
             !__private::AllTriviallyCopyAssignable<ValuesType, Ts...>)
  {
    check(o.tag_ != kUseAfterMove);
    if (tag_ == o.tag_) {
      storage_.copy_assign(tag_, o.storage_);
    } else {
      if (tag_ != kUseAfterMove) storage_.destroy(tag_);
      tag_ = o.tag_;
      storage_.copy_construct(tag_, o.storage_);
    }
    return *this;
  }

  // sus::mem::Clone<Union<Ts...>> trait.
  constexpr Union clone() const& noexcept
    requires(__private::AllClone<Ts...> && !__private::AllCopy<Ts...>)
  {
    check(tag_ != kUseAfterMove);
    auto u = Union(::sus::clone(tag_));
    u.storage_.clone_construct(tag_, storage_);
    return u;
  }

  /// Support for using Union in a `switch()` statment.
  inline operator ValuesType() const& { return which(); }

  /// Returns which is the active member of the Union.
  ///
  /// Typically to access the data in the Union, a switch statement would be
  /// used, so as to call the getter or setter methods with the right value
  /// specified as a template parameter. When used in a switch statement, the
  /// which() method can be omitted.
  ///
  /// # Example
  /// ```
  /// switch (my_union) {
  ///   case Value1: return my_union.get_ref<Value1>().stuff;
  ///   case Value2: return my_union.get_ref<Value2>().andmore;
  ///   case Value3: return my_union.get_ref<Value3>().stufftoo;
  /// }
  /// ```
  ///
  /// # Inspiration
  ///
  ///                       ████████
  ///                   ████▓▓░░▓▓██
  ///                 ██▓▓▓▓▓▓▓▓██
  ///               ██▓▓▓▓░░▓▓██
  ///             ██░░▓▓▓▓▓▓██
  ///           ██▓▓▓▓▓▓▓▓▓▓██
  ///           ██▓▓▓▓░░▓▓▓▓██
  ///   ████████▓▓▓▓▓▓▓▓▓▓▓▓▓▓████████
  /// ██▓▓░░▓▓▓▓▓▓░░▓▓▓▓▓▓▓▓▓▓░░▓▓▓▓▓▓██
  /// ██████████████████▓▓██████████████
  ///         ██      ██      ██
  ///         ██  ██  ██      ██
  ///         ██  ██  ████      ██
  ///         ██        ██      ██
  ///         ██▒▒      ██      ██
  ///         ██▒▒        ██      ██
  ///           ████████████████████
  ///                   ██  ██
  ///                 ██  ██▓▓██
  ///                 ▓▓  ██▓▓▓▓██
  ///               ██  ████░░▓▓▓▓██            ▓▓▓▓▓▓▓▓▓▓▓▓
  ///             ██  ██  ██▓▓▓▓░░▓▓██        ▓▓░░░░░░░░░░░░
  ///             ██  ██    ████▓▓▓▓▓▓██      ▓▓▓▓▓▓▓▓▓▓▓▓▓▓
  /// ▓▓▓▓      ██  ██    ██▓▓▓▓▓▓░░▓▓██    ▓▓▓▓░░░░░░░░░░░░
  /// ▓▓▓▓▓▓▓▓▓▓██████▓▓▓▓██▓▓░░▓▓▓▓██▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
  ///                     ██████████        ▓▓▓▓░░░░░░░░░░░░
  ///                       ██  ██            ▓▓▓▓▓▓▓▓▓▓▓▓▓▓
  ///                         ██  ██          ▓▓░░░░░░░░░░░░
  ///                           ██  ██          ▓▓▓▓▓▓▓▓▓▓▒▒
  ///                             ████
  ValuesType which() const& {
    check(tag_ != kUseAfterMove);
    constexpr ValuesType values[] = {Values...};
    return values[size_t{tag_}];
  }

  template <ValuesType V>
  decltype(auto) get_ref() const& {
    ::sus::check(tag_ == index<V>);
    return __private::find_storage<index<V>>(storage_).get_ref();
  }

  template <ValuesType V>
  decltype(auto) get_mut() & {
    ::sus::check(tag_ == index<V>);
    return __private::find_storage_mut<index<V>>(storage_).get_mut();
  }

  template <ValuesType V>
  decltype(auto) into_inner() && {
    ::sus::check(tag_ == index<V>);
    return ::sus::mem::move_or_copy_ref(__private::find_storage_mut<index<V>>(storage_))
        .into_inner();
  }

  template <ValuesType V, class U, int&..., class Arg = TypesFromValue<V>>
    requires(std::convertible_to<U, Arg>)
  void set(U values) {
    if (tag_ == index<V>) {
      __private::find_storage_mut<index<V>>(storage_).set(::sus::move(values));
    } else {
      if (tag_ != kUseAfterMove) storage_.destroy(tag_);
      tag_ = index<V>;
      __private::find_storage_mut<index<V>>(storage_).construct(::sus::move(values));
    }
  }

  /// sus::ops::Eq<Union<Ts...>, Union<Us...>> trait.
  template <class... Us, auto V, auto... Vs>
    requires(__private::UnionIsEq<ValuesType, __private::TypeList<Ts...>,
                                  decltype(V), __private::TypeList<Us...>>)
  friend inline constexpr bool operator==(
      const Union& l,
      const Union<__private::TypeList<Us...>, V, Vs...>& r) noexcept {
    check(l.tag_ != kUseAfterMove && r.tag_ != kUseAfterMove);
    return l.tag_ == r.tag_ && l.storage_.eq(l.tag_, r.storage_);
  }

  template <class... Us, auto V, auto... Vs>
    requires(!__private::UnionIsEq<ValuesType, __private::TypeList<Ts...>,
                                   decltype(V), __private::TypeList<Us...>>)
  friend inline constexpr bool operator==(
      const Union& l,
      const Union<__private::TypeList<Us...>, V, Vs...>& r) noexcept = delete;

  /// sus::ops::Ord<Union<Ts...>, Union<Us...>> trait.
  template <class... Us, auto V, auto... Vs>
    requires(__private::UnionIsOrd<ValuesType, __private::TypeList<Ts...>,
                                   decltype(V), __private::TypeList<Us...>>)
  friend inline constexpr auto operator<=>(
      const Union& l,
      const Union<__private::TypeList<Us...>, V, Vs...>& r) noexcept {
    check(l.tag_ != kUseAfterMove && r.tag_ != kUseAfterMove);
    const auto value_order = std::strong_order(l.which(), r.which());
    if (value_order != std::strong_ordering::equivalent) {
      return value_order;
    } else {
      return l.storage_.ord(l.tag_, r.storage_);
    }
  }

  /// sus::ops::WeakOrd<Union<Ts...>, Union<Us...>> trait.
  template <class... Us, auto V, auto... Vs>
    requires(__private::UnionIsWeakOrd<ValuesType, __private::TypeList<Ts...>,
                                       decltype(V), __private::TypeList<Us...>>)
  friend inline constexpr auto operator<=>(
      const Union& l,
      const Union<__private::TypeList<Us...>, V, Vs...>& r) noexcept {
    check(l.tag_ != kUseAfterMove && r.tag_ != kUseAfterMove);
    const auto value_order = std::weak_order(l.which(), r.which());
    if (value_order != std::weak_ordering::equivalent) {
      return value_order;
    } else {
      return l.storage_.weak_ord(l.tag_, r.storage_);
    }
  }

  /// sus::ops::PartialOrd<Union<Ts...>, Union<Us...>> trait.
  template <class... Us, auto V, auto... Vs>
    requires(
        __private::UnionIsPartialOrd<ValuesType, __private::TypeList<Ts...>,
                                     decltype(V), __private::TypeList<Us...>>)
  friend inline constexpr auto operator<=>(
      const Union& l,
      const Union<__private::TypeList<Us...>, V, Vs...>& r) noexcept {
    check(l.tag_ != kUseAfterMove && r.tag_ != kUseAfterMove);
    const auto value_order = std::partial_order(l.which(), r.which());
    if (value_order != std::partial_ordering::equivalent) {
      return value_order;
    } else {
      return l.storage_.partial_ord(l.tag_, r.storage_);
    }
  }

  template <class... Us, auto V, auto... Vs>
    requires(!__private::UnionIsAnyOrd<ValuesType, __private::TypeList<Ts...>,
                                       decltype(V), __private::TypeList<Us...>>)
  friend inline constexpr auto operator<=>(
      const Union<__private::TypeList<Ts...>, Values...>& l,
      const Union<__private::TypeList<Us...>, V, Vs...>& r) = delete;

 private:
  Union(TagType tag) : tag_(tag) {}

  [[sus_no_unique_address]] Storage storage_;
  TagType tag_;

  sus_class_never_value_field(unsafe_fn, Union, tag_, kNeverValue);
};

}  // namespace sus::union_type

// Promote Union into the `sus` namespace.
namespace sus {
using ::sus::union_type::Union;
}
