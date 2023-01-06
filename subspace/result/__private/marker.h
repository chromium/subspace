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

#include "subspace/mem/move.h"
#include "subspace/mem/mref.h"

namespace sus::result::__private {

template <class T>
struct OkMarker {
  sus_clang_bug_54040(constexpr inline OkMarker(T&& value)
                      : value(::sus::forward<T>(value)){});

  T&& value;

  template <class U, class E>
  inline constexpr operator Result<U, E>() && noexcept
      // `value` is a const reference.
    requires(std::is_const_v<std::remove_reference_t<T>>)
  {
    return Result<U, E>::with(value);
  }

  template <class U, class E>
  inline constexpr operator Result<U, E>() && noexcept
      // `value` is a mutable lvalue reference.
    requires(
        std::same_as<T &&, std::remove_const_t<std::remove_reference_t<T>>&>)
  {
    return Result<U, E>::with(mref(value));
  }

  template <class U, class E>
  inline constexpr operator Result<U, E>() && noexcept
      // `value` is an rvalue reference.
    requires(
        std::same_as<T &&, std::remove_const_t<std::remove_reference_t<T>> &&>)
  {
    return Result<U, E>::with(::sus::move(value));
  }
};

template <class T>
struct ErrMarker {
  sus_clang_bug_54040(constexpr inline ErrMarker(T&& value)
                      : value(::sus::forward<T>(value)){});

  T&& value;

  template <class U, class E>
  inline constexpr operator Result<U, E>() && noexcept
      // `value` is a const reference.
    requires(std::is_const_v<std::remove_reference_t<T>>)
  {
    return Result<U, E>::with_err(value);
  }

  template <class U, class E>
  inline constexpr operator Result<U, E>() && noexcept
      // `value` is a mutable lvalue reference.
    requires(
        std::same_as<T &&, std::remove_const_t<std::remove_reference_t<T>>&>)
  {
    return Result<U, E>::with_err(mref(value));
  }

  template <class U, class E>
  inline constexpr operator Result<U, E>() && noexcept
      // `value` is an rvalue reference.
    requires(
        std::same_as<T &&, std::remove_const_t<std::remove_reference_t<T>> &&>)
  {
    return Result<U, E>::with_err(::sus::move(value));
  }
};

}  // namespace sus::result::__private
