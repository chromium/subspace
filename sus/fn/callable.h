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

// IWYU pragma: private, include "sus/fn/fn.h"
// IWYU pragma: friend "sus/.*"
#pragma once

#include <concepts>
#include <functional>
#include <type_traits>

#include "sus/mem/forward.h"

namespace sus::fn::callable {

template <class T>
concept FunctionPointer = requires(const T& t) {
  { std::is_pointer_v<decltype(+t)> };
};

/// Verifies that T is a function pointer (or captureless lambda) that returns
/// a type convertible to `R` when called with `Args`.
///
/// This concept allows conversion of `Args` to the function's actual receiving
/// types and conversion from the function's return type to `R`.
///
// clang-format off
template <class T, class R, class... Args>
concept FunctionPointerReturns = (
    FunctionPointer<T> &&
    std::convertible_to<std::invoke_result_t<T, Args...>, R> &&
    // We verify that `T` can be stored in a function pointer. Types must match
    // more strictly than just for invoking it.
    requires (R(*p)(Args...), T& t) {
        { p = t };
    }
);
// clang-format on

/// Verifies that T is a function pointer (or captureless lambda) that receives
/// exactly `Args` as its parameters without conversion, and returns `R` without
/// conversion.
///
/// This is concept is useful if you intend to store the pointer in a strongly
/// typed function pointer, as the types must match exactly. If you only intend
/// to call the function pointer, prefer `FunctionPointerReturns` which allows
/// appropriate conversions.
//
// clang-format off
template <class T, class R, class... Args>
concept FunctionPointerMatches = (
    FunctionPointer<T> &&
    // We verify that `T` can be stored in a function pointer. Types must match
    // more strictly than just for invoking it.
    requires (R(*p)(Args...), T& t) {
        { p = t };
    }
);
// clang-format on

// clang-format off
template <class T, class... Args>
concept FunctionPointerWith = (
    FunctionPointer<T> &&
    requires (T& t, Args&&... args) {
        std::invoke(t, ::sus::mem::forward<Args>(args)...);
    }
);
// clang-format on

namespace __private {

template <class T, class R, class... Args>
inline constexpr bool callable_const(R (T::*)(Args...) const) {
  return true;
};

}  // namespace __private

template <class T, class R, class... Args>
concept CallableObjectReturnsOnce =
    !FunctionPointer<T> && requires(T& t, Args&&... args) {
      {
        std::invoke(static_cast<T&&>(t), ::sus::mem::forward<Args>(args)...)
      } -> std::convertible_to<R>;
    };

template <class T, class R, class... Args>
concept CallableObjectReturnsConst =
    !FunctionPointer<T> && requires(const T& t, Args&&... args) {
      {
        std::invoke(t, ::sus::mem::forward<Args>(args)...)
      } -> std::convertible_to<R>;
    };

template <class T, class R, class... Args>
concept CallableObjectReturnsMut =
    !FunctionPointer<T> && requires(T& t, Args&&... args) {
      {
        std::invoke(t, ::sus::mem::forward<Args>(args)...)
      } -> std::convertible_to<R>;
    };

template <class T>
concept CallableObjectConst = __private::callable_const(&T::operator());

}  // namespace sus::fn::callable
