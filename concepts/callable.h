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

#include "mem/forward.h"

namespace sus::concepts::callable {

template <class T>
concept FunctionPointer = requires(T t) {
                            { std::is_pointer_v<decltype(+t)> };
                          };

// clang-format off
template <class T, class R, class... Args>
concept FunctionPointerReturns = (
    FunctionPointer<T> &&
    requires (T t, Args&&... args) {
        { t(forward<Args>(args)...) } -> std::same_as<R>;
    }
);
// clang-format on

namespace __private {

template <class T, class R, class... Args>
inline constexpr bool lambda_callable_const(R (T::*)(Args...) const) {
  return true;
};

template <class T, class R, class... Args>
inline constexpr bool lambda_callable_mut(R (T::*)(Args...)) {
  return true;
};

}  // namespace __private

// clang-format off
template <class T, class R, class... Args>
concept LambdaReturnsConst = (
    !FunctionPointerReturns<T, R, Args...> &&
    requires (const T& t, Args&&... args) {
        { t(forward<Args>(args)...) } -> std::same_as<R>;
    }
);

template <class T, class R, class... Args>
concept LambdaReturnsMut = (
    !FunctionPointer<T> &&
    requires (T& t, Args&&... args) {
        { t(forward<Args>(args)...) } -> std::same_as<R>;
    }
);

template <class T, class R, class... Args>
concept LambdaReturnsOnce = (
    !FunctionPointer<T> &&
    requires (T&& t, Args&&... args) {
        { forward<T>(t)(forward<Args>(args)...) } -> std::same_as<R>;
    }
);
// clang-format on

template <class T, class R, class... Args>
concept LambdaReturns =
    LambdaReturnsConst<T, R, Args...> || LambdaReturnsMut<T, R, Args...>;

template <class T>
concept LambdaConst = __private::lambda_callable_const(&T::operator());

template <class T>
concept LambdaMut = LambdaConst<T> ||
                    __private::lambda_callable_mut(&T::operator());
}  // namespace sus::concepts::callable
