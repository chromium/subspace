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

#include "subspace/mem/forward.h"

namespace sus::fn::callable {

template <class T>
concept FunctionPointer = requires(T t) {
                            { std::is_pointer_v<decltype(+t)> };
                          };

// clang-format off
template <class T, class R, class... Args>
concept FunctionPointerReturns = (
    FunctionPointer<T> &&
    requires (T t, Args&&... args) {
        { t(forward<Args>(args)...) } -> std::convertible_to<R>;
    }
);
// clang-format on

// clang-format off
template <class T, class... Args>
concept FunctionPointerWith = (
    FunctionPointer<T> &&
    requires (T t, Args&&... args) {
        t(forward<Args>(args)...);
    }
);
// clang-format on

namespace __private {

template <class T, class R, class... Args>
inline constexpr bool callable_const(R (T::*)(Args...) const) {
  return true;
};

template <class T, class R, class... Args>
inline constexpr bool callable_mut(R (T::*)(Args...)) {
  return true;
};

}  // namespace __private

// clang-format off
template <class T, class R, class... Args>
concept CallableObjectReturnsConst = (
    !FunctionPointer<T> &&
    requires (const T& t, Args&&... args) {
        { t(forward<Args>(args)...) } -> std::convertible_to<R>;
    }
);

template <class T, class... Args>
concept CallableObjectWithConst = (
    !FunctionPointer<T> &&
    requires (const T& t, Args&&... args) {
     t(forward<Args>(args)...);
    }
);

template <class T, class R, class... Args>
concept CallableObjectReturnsMut = (
    !FunctionPointer<T> &&
    requires (T& t, Args&&... args) {
        { t(forward<Args>(args)...) } -> std::convertible_to<R>;
    }
);

template <class T, class... Args>
concept CallableObjectWithMut = (
    !FunctionPointer<T> &&
    requires (T& t, Args&&... args) {
        t(forward<Args>(args)...);
    }
);
// clang-format on

template <class T, class R, class... Args>
concept CallableObjectReturns = CallableObjectReturnsConst<T, R, Args...> ||
                                CallableObjectReturnsMut<T, R, Args...>;

template <class T, class... Args>
concept CallableObjectWith =
    CallableObjectWithConst<T, Args...> || CallableObjectWithMut<T, Args...>;

template <class T>
concept CallableObjectConst = __private::callable_const(&T::operator());

template <class T>
concept CallableObjectMut = CallableObjectConst<T> ||
                            __private::callable_mut(&T::operator());

template <class T, class... Args>
concept CallableWith =
    FunctionPointerWith<T, Args...> || CallableObjectWith<T, Args...>;

template <class T, class R, class... Args>
concept CallableReturns = FunctionPointerReturns<T, R, Args...> ||
                          CallableObjectReturns<T, R, Args...>;

}  // namespace sus::fn::callable
