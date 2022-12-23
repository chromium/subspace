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

#include "construct/from.h"

namespace sus::construct {

namespace __private {

template <class FromType>
struct IntoRef final {
  [[nodiscard]] constexpr IntoRef(FromType&& from) noexcept
      : from_(static_cast<FromType&&>(from)) {}

  template <::sus::construct::From<FromType> ToType>
  constexpr operator ToType() && noexcept {
    return ToType::from(static_cast<FromType&&>(from_));
  }

  // Doesn't copy or move. `IntoRef` should only be used as a temporary.
  IntoRef(const IntoRef&) = delete;
  IntoRef& operator=(const IntoRef&) = delete;

 private:
  FromType&& from_;
};

template <class FromType, size_t N>
struct IntoRefArray final {
  [[nodiscard]] constexpr IntoRefArray(FromType (&from)[N]) noexcept
      : from_(from) {}

  template <::sus::construct::From<FromType (&)[N]> ToType>
  constexpr operator ToType() && noexcept {
    return ToType::template from<N>(from_);
  }

  // Doesn't copy or move. `IntoRefArray` should only be used as a temporary.
  IntoRefArray(const IntoRefArray&) = delete;
  IntoRefArray& operator=(const IntoRefArray&) = delete;

 private:
  FromType (&from_)[N];
};

}  // namespace __private

template <class FromType, class ToType>
concept Into = ::sus::construct::From<ToType, FromType>;

template <class FromType>
  requires(std::is_rvalue_reference_v<FromType &&>)
constexpr inline auto into(FromType&& from) noexcept {
  return __private::IntoRef(
      static_cast<std::remove_reference_t<FromType>&&>(from));
}

template <class FromType>
  requires(std::is_trivially_copyable_v<FromType>)
constexpr inline auto into(const FromType& from) noexcept {
  return __private::IntoRef<const FromType&>(from);
}

template <class FromType>
  requires(std::is_trivially_move_constructible_v<FromType>)
constexpr inline auto into(FromType& from) noexcept {
  return __private::IntoRef(static_cast<FromType&&>(from));
}

template <class FromType, size_t N>
constexpr inline auto into(FromType (&from)[N]) noexcept {
  return __private::IntoRefArray<FromType, N>(from);
}

template <class FromType>
  requires(!std::is_const_v<FromType>)
constexpr inline auto move_into(FromType& from) noexcept {
  return __private::IntoRef(
      static_cast<std::remove_cv_t<std::remove_reference_t<FromType>>&&>(from));
}

template <class FromType>
  requires(std::is_rvalue_reference_v<FromType &&> &&
           !std::is_const_v<FromType>)
constexpr inline auto move_into(FromType&& from) noexcept {
  return __private::IntoRef(
      static_cast<std::remove_cv_t<std::remove_reference_t<FromType>>&&>(from));
}

}  // namespace sus::construct

// Promote Into and its helper methods into the `sus` namespace.
namespace sus {
using ::sus::construct::into;
using ::sus::construct::move_into;
}  // namespace sus
