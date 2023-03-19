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

#include <concepts>

#include "subspace/result/__private/is_result_type.h"

namespace sus::construct {

/// A concept that indicates `ToType` can be constructed from a `FromType`, via
/// `ToType::from(FromType)`.
///
/// This concept is rarely used directly, instead prefer to use the
/// `sus::construct::Into` concept, as it also supports identity
/// transformations.
///
/// When a type is `From<T, O>`, it is also `Into<O, T>`. Then a variable `o` of
/// type `O` can be explicitly converted to `T`, with type deduction, via
/// `sus::into(o)`.
///
/// # Arrays
/// It's possible to convert from an array, in which case `From<T, O(&)[]>` is
/// satisfied. To do so, implement `from()` as a templated method,
/// with a `size_t` template parameter for the size of the incoming array. For
/// example:
/// ```
/// // sus::construct::From<Slice<T>, O[]> trait.
/// template <size_t N>
/// static constexpr inline Slice from(T (&data)[N]) {
///   return Slice(data, N);
/// }
/// ```
template <class ToType, class FromType>
concept From = requires(FromType&& from) {
  { ToType::from(static_cast<FromType&&>(from)) } -> std::same_as<ToType>;
};

/// A concept that indicates `ToType` can be (sometimes) constructed from a
/// `FromType`, via `ToType::try_from(FromType)`.
///
/// Unlike `sus::construct::From`, using the `try_from()` method gives the
/// opportunity to catch failures since the return type is a
/// `sus::Result`.
template <class ToType, class FromType>
concept TryFrom = requires(FromType&& from) {
  { ToType::try_from(static_cast<FromType&&>(from)) };
  requires std::same_as<
      typename ::sus::result::__private::IsResultType<decltype(ToType::try_from(
          static_cast<FromType&&>(from)))>::ok_type,
      ToType>;
};

}  // namespace sus::construct
