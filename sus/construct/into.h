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

#include "sus/construct/__private/into_ref.h"
#include "sus/construct/from.h"
#include "sus/macros/compiler.h"
#include "sus/macros/lifetimebound.h"
#include "sus/mem/copy.h"
#include "sus/mem/forward.h"
#include "sus/mem/move.h"

namespace sus::construct {

/// A concept that declares `FromType` can be converted to `ToType`.
///
/// When true, the conversion can be done in one of three ways:
/// * `ToType::from(FromType)` can be used to construct `ToType`, which is the
///   preferred way to write conversions. It avoids any accidental conversions
///   as it does not have an ambiguous appearance with a copy or move.
/// * `ToType` is the same as `FromType`, in which case a reference to the
///   object is passed along, and no construction or conversion happens.
///
/// This is the inverse direction from the [`From`]($sus::construct::From)
/// concept, while also a broader generalization. It is meant to
/// be used on methods that want to receive any type that can explicitly be
/// converted to a specific type.
///
/// This concept is not implementable directly, as it's satisfied for `T` by
/// implementing the [`From<T>`]($sus::construct::From) concept on a different
/// type.
/// It is only possible to satisfy this concept for a `ToType` that is not a
/// reference, as it needs to be able to construct `ToType`.
///
/// # Templates
///
/// To receive [`into()`]($sus::construct::into) correctly for a templated
/// function argument:
/// * Avoid [`std::same_as<T>`](
///   https://en.cppreference.com/w/cpp/concepts/same_as), use
///   [`std::convertible_to<T>`](
///   https://en.cppreference.com/w/cpp/concepts/convertible_to) instead, as
///   this will accept the marker type returned from
///   [into]($sus::construct::into).
/// * If the argument is a fixed dependent type, like the following:
///   ```
///   template <class T, class In = Foo<T>>
///   void f(In i) {}
///   ```
///   Insert an extra template parameter that uses
///   [`std::convertible_to`](
///   https://en.cppreference.com/w/cpp/concepts/convertible_to)
///   and the template type, such as:
///   ```
///   template <class T, class Exact = Foo<T>, std::convertible_to<Exact> In>
///   void f(In i) {}
///   ```
///   As this will allow receiving types converted to `Exact` by the caller
///   using [`into`]($sus::construct::into).
///
/// # Arrays
///
/// Receiving an array is possible by implementing
/// [`From`]($sus::construct::From) with `from()` being a templated method
/// templated on a `size_t` template parameter which represents the size of the
/// incoming array. For example:
/// ```
/// // sus::construct::Into<Slice<T>, T[N]> trait.
/// template <size_t N>
/// static constexpr Slice from(T (&data)[N]) noexcept {
///   return Slice(data, N);
/// }
/// ```
/// Then `sus::into(an_array)` can be used to construct
/// [`Slice<T>`]($sus::collections::Slice).
template <class FromType, class ToType>
concept Into =
    ::sus::construct::From<ToType, FromType> || std::same_as<ToType, FromType>;

/// Converts from the given value to whatever a receiver requires.
///
/// The result will be receivable if [`Into<FromType,
/// ToType>`]($sus::construct::Into) is satisfied where `ToType` is deduced by
/// the type constructed from the return value of `into`.
///
/// The value returned by `into` should be immediately converted into the
/// desired type, and never held as an lvalue itself. The returned type holds a
/// reference to the input that is used to construct the deduced `ToType` from
/// it.
///
/// If the argument to `into` is [`Copy`]($sus::mem::Copy) then it will be
/// copied if it is an lvalue or const. If the argument to `into` is an rvalue,
/// it will be moved when constructing the `ToType`.
template <class FromType>
constexpr inline auto into(FromType&& from sus_lifetimebound) noexcept {
  return __private::IntoRef<FromType&&>(::sus::forward<FromType>(from));
}

/// Moves from and converts from the given value to whatever a receiver
/// requires.
///
/// This is sugar for `sus::into(sus::move(x))`, which can be written as
/// `sus::move_into(x)`. The result will be receivable if [`Into<FromType,
/// ToType>`]($sus::construct::Into) is satisfied.
///
/// To capture an rvalue, use [`into`]($sus::construct::into). Note that arrays
/// can not be moved, so can not be given to `move_into`.
template <class FromType>
  requires(!std::is_const_v<std::remove_reference_t<FromType>> &&
           !std::is_array_v<FromType>)
constexpr inline auto move_into(FromType& from sus_lifetimebound) noexcept {
  return __private::IntoRef(
      static_cast<std::remove_cv_t<std::remove_reference_t<FromType>>&&>(from));
}

template <class FromType>
  requires(std::is_rvalue_reference_v<FromType &&> &&
           !std::is_const_v<std::remove_reference_t<FromType>> &&
           !std::is_array_v<FromType>)
constexpr inline auto move_into(FromType&& from sus_lifetimebound) noexcept {
  return __private::IntoRef(
      static_cast<std::remove_cv_t<std::remove_reference_t<FromType>>&&>(from));
}

/// A concept that declares `FromType` can (sometimes) be converted to `ToType`
/// through the `TryFrom` concept or through an identity transformation.
template <class FromType, class ToType>
concept TryInto = ::sus::construct::TryFrom<ToType, FromType> ||
                  std::same_as<ToType, FromType>;

/// Attempts to convert from the given value to a `ToType`.
///
/// Unlike [`into()`]($sus::construct::into), this function can not use type
/// deduction to determine the
/// receiving type as it needs to determine the [`Result`]($sus::result::Result)
/// type and allow the caller the chance to handle the error condition.
///
/// The `TryFrom` concept requires a `try_from()` method that returns a
/// [`Result`]($sus::result::Result). That [`Result`]($sus::result::Result)
/// will be the return type of this function.
///
/// # Example
/// ```
/// auto valid = sus::try_into<u8>(123_i32).unwrap_or_default();
/// sus::check(valid == 123);
/// auto invalid = sus::try_into<u8>(-1_i32).unwrap_or_default();
/// sus::check(invalid == 0);
/// ```
template <class ToType, TryInto<ToType> FromType>
constexpr inline auto try_into(FromType&& from) noexcept {
  return ToType::try_from(::sus::forward<FromType>(from));
}

}  // namespace sus::construct

// Promote the into functions into the `sus` namespace.
namespace sus {
using ::sus::construct::into;
using ::sus::construct::move_into;
using ::sus::construct::try_into;
}  // namespace sus
