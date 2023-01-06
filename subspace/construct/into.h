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

#include <concepts>
#include <type_traits>

#include "subspace/construct/__private/into_ref.h"
#include "subspace/construct/from.h"
#include "subspace/macros/compiler.h"

namespace sus::construct {

/// A concept that declares `FromType` can be converted to `ToType`, through the
/// `From` concept.
///
/// When true, `ToType::from(FromType)` can be used to construct `ToType`.
///
/// This is the inverse of the `From` concept, meant to be used on methods that
/// want to receive any type and which will explicitly convert what they are
/// given type.
///
/// This concept is not implementable directly, as it's satisfied for `T` by
/// implementing the `From<T>` trait on a different type.
///
/// # Templates
/// To receive `sus::into()` correctly for a templated function argument:
/// - Avoid std::same_as<T>, use std::convertible_to<T> instead, as this will
///   accept the output of sus::into().
/// - If the argument is a fixed dependent type, like the following:
///   ```
///   template <class T, class In = Foo<T>>
///   void f(In i) {}
///   ```
///   Insert an extra template parameter that uses std::convertible_to and the
///   template type, such as:
///   ```
///   template <class T, class Exact = Foo<T>, std::convertible_to<Exact> In>
///   void f(In i) {}
///   ```
///   As this will allow receiving types converted to `Exact` by the caller
///   using `sus::into()`.
///
/// # Arrays
/// Receiving an array is possible by implementing `from()` as a templated
/// method, with a `size_t` template parameter for the size of the incoming
/// array. For example:
/// ```
/// // sus::construct::Into<Slice<T>, T[]> trait.
/// template <size_t N>
/// static constexpr inline Slice from(T (&data)[N]) {
///   return Slice(data, N);
/// }
/// ```
/// Then `sus::array_into(an_array)` can be used to construct `Slice<T>`.
template <class FromType, class ToType>
concept Into = ::sus::construct::From<ToType, FromType>;

/// Converts from the given value to whatever a receiver requires.
///
/// The result will be receivable if `Into<GivenType, ReceiverType>` is
/// satisfied.
template <class FromType>
  requires(!std::is_const_v<std::remove_reference_t<FromType>> &&
           std::is_rvalue_reference_v<FromType &&> &&
           !std::is_array_v<FromType>)
constexpr inline auto into(
    FromType&& from sus_if_clang([[clang::lifetimebound]])) noexcept {
  return __private::IntoRef(
      static_cast<std::remove_reference_t<FromType>&&>(from));
}

template <class FromType>
  requires(std::is_trivially_copyable_v<FromType> && !std::is_array_v<FromType>)
constexpr inline auto into(
    const FromType& from sus_if_clang([[clang::lifetimebound]])) noexcept {
  return __private::IntoRef<const FromType&>(from);
}

template <class FromType>
  requires(!std::is_const_v<std::remove_reference_t<FromType>> &&
           std::is_trivially_move_constructible_v<FromType> &&
           !std::is_array_v<FromType>)
constexpr inline auto into(
    FromType& from sus_if_clang([[clang::lifetimebound]])) noexcept {
  return __private::IntoRef(
      static_cast<std::remove_reference_t<FromType>&&>(from));
}

/// Converts from the given array to whatever a receiver requires.
///
/// The result will be receivable if `Into<GivenType, ReceiverType[]>` is
/// satisfied.
template <class FromType, size_t N>
constexpr inline auto array_into(
    FromType (&from)[N] sus_if_clang([[clang::lifetimebound]])) noexcept {
  return __private::IntoRefArray<FromType, N>(from);
}

/// Moves from and converts from the given value to whatever a receiver
/// requires.
///
/// This is sugar for `sus::into(sus::move(x))`, which can be written as
/// `sus::move_into(x)`.
///
/// The result will be receivable if `Into<GivenType, ReceiverType>` is
/// satisfied.
template <class FromType>
  requires(!std::is_const_v<std::remove_reference_t<FromType>> &&
           !std::is_array_v<FromType>)
constexpr inline auto move_into(
    FromType& from sus_if_clang([[clang::lifetimebound]])) noexcept {
  return __private::IntoRef(
      static_cast<std::remove_cv_t<std::remove_reference_t<FromType>>&&>(from));
}

template <class FromType>
  requires(std::is_rvalue_reference_v<FromType &&> &&
           !std::is_const_v<std::remove_reference_t<FromType>> &&
           !std::is_array_v<FromType>)
constexpr inline auto move_into(
    FromType&& from sus_if_clang([[clang::lifetimebound]])) noexcept {
  return __private::IntoRef(
      static_cast<std::remove_cv_t<std::remove_reference_t<FromType>>&&>(from));
}

}  // namespace sus::construct

// Promote Into and its helper methods into the `sus` namespace.
namespace sus {
using ::sus::construct::array_into;
using ::sus::construct::into;
using ::sus::construct::move_into;
}  // namespace sus
