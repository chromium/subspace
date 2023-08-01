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
  static constexpr bool from_const =
      ::sus::construct::SafelyConstructibleFromReference<To, const From&>;
  static constexpr bool from_rvalue =
      ::sus::construct::SafelyConstructibleFromReference<To, From&&>;
};

template <class... To, class... From>
  requires(sizeof...(To) > 1)
struct VerifySafe<sus::Tuple<To...>, From...> {
  static constexpr bool from_const =
      (... &&
       ::sus::construct::SafelyConstructibleFromReference<To, const From&>);
  static constexpr bool from_rvalue =
      (... && ::sus::construct::SafelyConstructibleFromReference<To, From&&>);
};

template <class To, class... From>
struct AllConvertible;

template <class To, class From>
struct AllConvertible<To, From> {
  static constexpr bool from_const = ::std::constructible_from<To, const From&>;
  static constexpr bool from_rvalue = ::std::constructible_from<To, From&&>;
};

template <class... To, class... From>
  requires(sizeof...(To) > 1)
struct AllConvertible<::sus::Tuple<To...>, From...> {
  static constexpr bool from_const =
      (... && ::std::constructible_from<To, const From&>);
  static constexpr bool from_rvalue =
      (... && ::std::constructible_from<To, From&&>);
};

template <auto Tag, class... Ts>
struct ChoiceMarker;

template <auto Tag>
struct ChoiceMarkerVoid {
  // If the Choice's type can construct from a const ref `value` (roughly, is
  // copy-constructible, but may change types), then the marker can do the same.
  //
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
struct ChoiceMarker<Tag, T> {
  sus_clang_bug_54040(constexpr inline ChoiceMarker(T&& value)
                      : value(::sus::forward<T>(value)){});

  T&& value;

  // If the Choice's type can construct from a const ref `value` (roughly, is
  // copy-constructible, but may change types), then the marker can do the same.
  //
  // This largely exists to support use in Gtest's EXPECT_EQ, which uses them as
  // a const&, since marker types should normally be converted quickly to the
  // concrete type.
  template <class C, auto... Vs>
    requires(AllConvertible<StorageTypeFromChoice<Choice<C, Vs...>, Tag>,
                            T>::from_const)
  inline constexpr operator Choice<C, Vs...>() const& noexcept {
    static_assert(
        VerifySafe<StorageTypeFromChoice<Choice<C, Vs...>, Tag>, T>::from_const,
        "Unable to safely convert to a different reference type, as conversion "
        "would produce a reference to a temporary. The ChoiceMarker's value "
        "type must match the Choice's. For example a `Choice holding "
        "`const u32&` can not be constructed from a ChoiceMarker holding "
        "`const i16&`, but it can be constructed from `i32&&`.");
    return Choice<C, Vs...>::template with<Tag>(value);
  }

  template <class C, auto... Vs>
    requires(AllConvertible<StorageTypeFromChoice<Choice<C, Vs...>, Tag>,
                            T>::from_rvalue)
  inline constexpr operator Choice<C, Vs...>() && noexcept {
    static_assert(
        VerifySafe<StorageTypeFromChoice<Choice<C, Vs...>, Tag>,
                   T>::from_rvalue,
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
struct ChoiceMarker<Tag, Ts...> {
  sus_clang_bug_54040(
      constexpr inline ChoiceMarker(::sus::tuple_type::Tuple<Ts&&...>&& values)
      : values(::sus::move(values)){});

  ::sus::tuple_type::Tuple<Ts&&...> values;

  // If the Choice's types can construct from const ref `values` (roughly, is
  // copy-constructible, but may change types), then the marker can do the
  // same.
  //
  // This largely exists to support use in Gtest's EXPECT_EQ, which uses them
  // as a const&, since marker types should normally be converted quickly to
  // the concrete type.
  template <class C, auto... Vs>
    requires(AllConvertible<StorageTypeFromChoice<Choice<C, Vs...>, Tag>,
                            Ts...>::from_const)
  inline constexpr operator Choice<C, Vs...>() const& noexcept {
    static_assert(
        VerifySafe<StorageTypeFromChoice<Choice<C, Vs...>, Tag>,
                   Ts...>::from_const,
        "Unable to safely convert to a different reference type, as conversion "
        "would produce a reference to a temporary. The ChoiceMarker's value "
        "type must match the Choice's. For example a `Choice holding "
        "`const u32&` can not be constructed from a ChoiceMarker holding "
        "`const i16&`, but it can be constructed from `i32&&`.");
    using TupleType =
        decltype(std::declval<Choice<C, Vs...>&&>().template into_inner<Tag>());
    auto make_tuple =
        [this]<size_t... Is>(std::integer_sequence<size_t, Is...>) {
          return TupleType::with(values.template at<Is>()...);
        };
    return Choice<C, Vs...>::template with<Tag>(
        make_tuple(std::make_integer_sequence<size_t, sizeof...(Ts)>()));
  }

  template <class C, auto... Vs>
    requires(AllConvertible<StorageTypeFromChoice<Choice<C, Vs...>, Tag>,
                            Ts...>::from_rvalue)
  inline constexpr operator Choice<C, Vs...>() && noexcept {
    static_assert(
        VerifySafe<StorageTypeFromChoice<Choice<C, Vs...>, Tag>,
                   Ts...>::from_rvalue,
        "Unable to safely convert to a different reference type, as conversion "
        "would produce a reference to a temporary. The ChoiceMarker's value "
        "type must match the Choice's. For example a `Choice holding "
        "`const u32&` can not be constructed from a ChoiceMarker holding "
        "`const i16&`, but it can be constructed from `i32&&`.");
    using TupleType =
        decltype(std::declval<Choice<C, Vs...>&&>().template into_inner<Tag>());
    auto make_tuple =
        [this]<size_t... Is>(std::integer_sequence<size_t, Is...>) {
          return TupleType::with(
              ::sus::forward<Ts>(values.template at_mut<Is>())...);
        };
    return Choice<C, Vs...>::template with<Tag>(
        make_tuple(std::make_integer_sequence<size_t, sizeof...(Ts)>()));
  }
};

}  // namespace sus::choice_type::__private
