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
#include <type_traits>

#include "concepts/from.h"
#include "mem/forward.h"

namespace sus::concepts::into {

namespace __private {
template <class FromType>
struct IntoRef {
  [[nodiscard]] constexpr IntoRef(FromType&& from) noexcept
      : from_(static_cast<FromType&&>(from)) {}

  template <::sus::concepts::from::From<FromType> ToType>
  constexpr operator ToType() && noexcept {
    return ToType::from(static_cast<FromType&&>(from_));
  }

  // Doesn't copy or move. `Into` should only be used as a temporary.
  IntoRef(const IntoRef&) = delete;
  IntoRef& operator=(const IntoRef&) = delete;

 private:
  FromType&& from_;
};

}  // namespace __private

template<class FromType, class ToType>
concept Into = ::sus::concepts::from::From<ToType, FromType>;

template <class FromType>
  requires(std::is_rvalue_reference_v<FromType &&>)
constexpr inline auto into(FromType&& from) noexcept {
  return __private::IntoRef(static_cast<FromType&&>(from));
}

template <class FromType>
  requires(std::is_lvalue_reference_v<FromType &&> &&
           !std::is_const_v<std::remove_reference_t<FromType>>)
constexpr inline auto move_into(FromType&& from) noexcept {
  return __private::IntoRef(
      static_cast<std::remove_cv_t<std::remove_reference_t<FromType>>&&>(from));
}

template <class FromType>
  requires(std::is_rvalue_reference_v<FromType &&> &&
           !std::is_const_v<std::remove_reference_t<FromType>>)
constexpr inline auto move_into(FromType&& from) noexcept {
  return __private::IntoRef(
      static_cast<std::remove_cv_t<std::remove_reference_t<FromType>>&&>(from));
}

}  // namespace sus::concepts::into

// Promote Into and its helper methods into the `sus` namespace.
namespace sus {
using ::sus::concepts::into::into;
using ::sus::concepts::into::move_into;
}  // namespace sus
