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

#include "sus/construct/from.h"

namespace sus::construct ::__private {

template <class FromType>
struct IntoRef final {
  [[nodiscard]] constexpr IntoRef(FromType&& from) noexcept
      : from_(static_cast<FromType&&>(from)) {}

  template <std::same_as<std::decay_t<FromType>> ToType>
  constexpr operator ToType() && noexcept {
    return static_cast<ToType&&>(from_);
  }

  template <::sus::construct::From<FromType> ToType>
    requires(!std::same_as<std::decay_t<FromType>, ToType>)
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

  template <std::same_as<FromType (&)[N]> ToType>
  constexpr operator ToType() && noexcept {
    return static_cast<ToType&&>(from_);
  }

  template <::sus::construct::From<FromType (&)[N]> ToType>
    requires(!std::same_as<FromType, ToType>)
  constexpr operator ToType() && noexcept {
    return ToType::template from<N>(from_);
  }

  // Doesn't copy or move. `IntoRefArray` should only be used as a temporary.
  IntoRefArray(const IntoRefArray&) = delete;
  IntoRefArray& operator=(const IntoRefArray&) = delete;

 private:
  FromType (&from_)[N];
};

}  // namespace sus::construct::__private
