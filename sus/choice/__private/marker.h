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

// IWYU pragma: private
// IWYU pragma: friend "sus/.*"
#pragma once

#include <stddef.h>

#include <utility>  // TODO: Replace with our own integer_sequence.

#include "sus/choice/__private/index_of_value.h"
#include "sus/choice/__private/storage.h"
#include "sus/choice/__private/type_list.h"
#include "sus/construct/safe_from_reference.h"
#include "sus/lib/__private/forward_decl.h"
#include "sus/macros/__private/compiler_bugs.h"
#include "sus/mem/forward.h"
#include "sus/tuple/tuple.h"

namespace sus::choice_type::__private {

template <class ChoiceType, auto Tag, class... From>
struct StorageTypeFromChoiceHelper;

template <class... Cs, auto... Tags, auto Tag>
struct StorageTypeFromChoiceHelper<Choice<__private::TypeList<Cs...>, Tags...>,
                                   Tag> {
  static constexpr size_t tag_index =
      __private::IndexOfValue<Tag, Tags...>::value;
  using type = StorageTypeOfTag<tag_index, Cs...>;
};

/// A tool to get the storage type associated with a Tag in a `Choice` without
/// seeing the `Choice` type definition.
template <class ChoiceType, auto Tag>
using StorageTypeFromChoice =
    StorageTypeFromChoiceHelper<ChoiceType, Tag>::type;

template <class To, class... From>
struct VerifySafe;

template <class To, class From>
struct VerifySafe<To, From> {
  static constexpr bool value =
      ::sus::construct::SafelyConstructibleFromReference<To, From>;
};

template <class... To, class... From>
  requires(sizeof...(To) > 1)
struct VerifySafe<sus::Tuple<To...>, From...> {
  static constexpr bool value =
      (... && ::sus::construct::SafelyConstructibleFromReference<To, From>);
};

template <class To, class... From>
struct AllConvertibleFrom;

template <class To, class From>
struct AllConvertibleFrom<To, From> {
  static constexpr bool value = ::std::convertible_to<From, To>;
};

template <class... To, class... From>
  requires(sizeof...(To) > 1)
struct AllConvertibleFrom<::sus::Tuple<To...>, From...> {
  static constexpr bool value = (... && ::std::convertible_to<From, To>);
};

template <auto Tag, class... Ts>
struct ChoiceMarker;

template <auto Tag>
struct ChoiceMarkerVoid {
  // This largely exists to support use in Gtest's EXPECT_EQ, which uses them as
  // a const&, since marker types should normally be converted quickly to the
  // concrete type.
  template <class C, auto... Vs>
  inline constexpr operator Choice<C, Vs...>() const& noexcept {
    return Choice<C, Vs...>::template with<Tag>();
  }

  template <class C, auto... Vs>
  inline constexpr operator Choice<C, Vs...>() && noexcept {
    return Choice<C, Vs...>::template with<Tag>();
  }
};

template <auto Tag, class T>
struct [[nodiscard]] ChoiceMarker<Tag, T> {
  sus_clang_bug_54040(constexpr inline ChoiceMarker(T&& value)
                      : value(::sus::forward<T>(value)){});

  T&& value;

  // Gtest macros force evaluation against a const reference.
  // https://github.com/google/googletest/issues/4350
  //
  // So we allow constructing the target type from a const ref `value`. This
  // can perform explicit conversions which isn't correct but is required in
  // order to be able to call this method twice, because `value` itself may not
  // be copyable (marker types are not) but construction from const is more
  // likely.
  template <class C, auto... Vs>
  inline constexpr operator Choice<C, Vs...>() const& noexcept {
    using U = StorageTypeFromChoice<Choice<C, Vs...>, Tag>;
    static_assert(
        AllConvertibleFrom<U, ::sus::mem::remove_rvalue_reference<T>>::value,
        "ChoiceMarker<Tag>(T) can't convert const T& to U for "
        "Choice::with<Tag>(U). "
        "Note that this is a test-only code path for Gtest support. "
        "Typically the T object is consumed as an rvalue, consider using "
        "EXPECT_TRUE(a == b) if needed.");
    static_assert(
        AllConvertibleFrom<U, T&&>::value,
        "ChoiceMarker<Tag>(T) can't convert T to U for Choice::with<Tag>(U)");
    static_assert(
        VerifySafe<U, T&&>::value,
        "Unable to safely convert to a different reference type, as conversion "
        "would produce a reference to a temporary. The ChoiceMarker's value "
        "type must match the Choice's. For example a `Choice holding "
        "`const u32&` can not be constructed from a ChoiceMarker holding "
        "`const i16&`, but it can be constructed from `i32&&`.");
    return Choice<C, Vs...>::template with<Tag>(U(value));
  }

  template <class C, auto... Vs>
  inline constexpr operator Choice<C, Vs...>() && noexcept {
    using U = StorageTypeFromChoice<Choice<C, Vs...>, Tag>;
    static_assert(
        AllConvertibleFrom<U, T&&>::value,
        "ChoiceMarker<Tag>(T) can't convert T to U for Choice::with<Tag>(U)");
    static_assert(
        VerifySafe<U, T&&>::value,
        "Unable to safely convert to a different reference type, as conversion "
        "would produce a reference to a temporary. The ChoiceMarker's value "
        "type must match the Choice's. For example a `Choice holding "
        "`const u32&` can not be constructed from a ChoiceMarker holding "
        "`const i16&`, but it can be constructed from `i32&&`.");
    return Choice<C, Vs...>::template with<Tag>(::sus::forward<T>(value));
  }
};

template <auto Tag, class... Ts>
  requires(sizeof...(Ts) > 1)
struct [[nodiscard]] ChoiceMarker<Tag, Ts...> {
  sus_clang_bug_54040(
      constexpr inline ChoiceMarker(::sus::tuple_type::Tuple<Ts&&...>&& values)
      : values(::sus::move(values)){});

  ::sus::tuple_type::Tuple<Ts&&...> values;

  // Gtest macros force evaluation against a const reference.
  // https://github.com/google/googletest/issues/4350
  //
  // So we allow constructing the target type from a const ref `value`. This
  // can perform explicit conversions which isn't correct but is required in
  // order to be able to call this method twice, because `value` itself may not
  // be copyable (marker types are not) but construction from const is more
  // likely.
  template <class C, auto... Vs>
  inline constexpr operator Choice<C, Vs...>() const& noexcept {
    using Us = StorageTypeFromChoice<Choice<C, Vs...>, Tag>;
    static_assert(
        AllConvertibleFrom<Us, ::sus::mem::remove_rvalue_reference<Ts>...>::value,
        "ChoiceMarker<Tag>(Ts...) can't convert const Ts& to Us for "
        "Choice::with<Tag>(Us...). "
        "Note that this is a test-only code path for Gtest support. "
        "Typically the T object is consumed as an rvalue, consider using "
        "EXPECT_TRUE(a == b) if needed.");
    static_assert(AllConvertibleFrom<Us, Ts&&...>::value,
                  "ChoiceMarker<Tag>(Ts...) can't convert Ts to Us for "
                  "Choice::with<Tag>(Us...)");
    static_assert(
        VerifySafe<Us, Ts&&...>::value,
        "Unable to safely convert to a different reference type, as conversion "
        "would produce a reference to a temporary. The ChoiceMarker's value "
        "type must match the Choice's. For example a `Choice holding "
        "`const u32&` can not be constructed from a ChoiceMarker holding "
        "`const i16&`, but it can be constructed from `i32&&`.");
    using TupleType =
        decltype(std::declval<Choice<C, Vs...>&&>().template into_inner<Tag>());
    auto make_tuple =
        [this]<size_t... Is>(std::integer_sequence<size_t, Is...>) {
          return TupleType(typename std::tuple_element<Is, TupleType>::type(values.template at<Is>())...);
        };
    return Choice<C, Vs...>::template with<Tag>(
        make_tuple(std::make_integer_sequence<size_t, sizeof...(Ts)>()));
  }

  template <class C, auto... Vs>
  inline constexpr operator Choice<C, Vs...>() && noexcept {
    static_assert(
        AllConvertibleFrom<StorageTypeFromChoice<Choice<C, Vs...>, Tag>,
                           Ts&&...>::value,
        "ChoiceMarker<Tag>(Ts...) can't convert Ts to Us for "
        "Choice::with<Tag>(Us...)");
    static_assert(
        VerifySafe<StorageTypeFromChoice<Choice<C, Vs...>, Tag>,
                   Ts&&...>::value,
        "Unable to safely convert to a different reference type, as conversion "
        "would produce a reference to a temporary. The ChoiceMarker's value "
        "type must match the Choice's. For example a `Choice holding "
        "`const u32&` can not be constructed from a ChoiceMarker holding "
        "`const i16&`, but it can be constructed from `i32&&`.");
    using TupleType =
        decltype(std::declval<Choice<C, Vs...>&&>().template into_inner<Tag>());
    auto make_tuple =
        [this]<size_t... Is>(std::integer_sequence<size_t, Is...>) {
          return TupleType(::sus::forward<Ts>(values.template at_mut<Is>())...);
        };
    return Choice<C, Vs...>::template with<Tag>(
        make_tuple(std::make_integer_sequence<size_t, sizeof...(Ts)>()));
  }
};

}  // namespace sus::choice_type::__private
