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

#include <concepts>

#include "sus/construct/from.h"
#include "sus/mem/forward.h"

namespace sus::construct ::__private {

template <class FromType>
struct IntoRef final {
  [[nodiscard]] constexpr IntoRef(FromType&& from) noexcept
      : from_(::sus::forward<FromType>(from)) {}

  template <std::same_as<std::remove_reference_t<FromType>> ToType>
  constexpr operator ToType() && noexcept {
    return static_cast<ToType&&>(from_);
  }

  template <::sus::construct::From<FromType> ToType>
    requires(!std::same_as<std::remove_reference_t<FromType>, ToType>)
  constexpr operator ToType() && noexcept {
    return ToType::from(::sus::forward<FromType>(from_));
  }

  // Doesn't copy or move. `IntoRef` should only be used as a temporary.
  IntoRef(const IntoRef&) {
    static_assert(std::same_as<FromType, FromType>,
                  "into() must immediately be converted to a type by forcing a "
                  "conversion, it can not be stored");
  }
  IntoRef& operator=(const IntoRef&) = delete;

 private:
  FromType&& from_;
};

}  // namespace sus::construct::__private
