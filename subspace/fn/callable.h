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

namespace __private {
template <class R, class... Args>
void CallablePointer(R (*)(Args...)) noexcept {}
}  // namespace __private

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
    requires (T t) {
        __private::CallablePointer(+t);
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
    requires (T t) {
        __private::CallablePointer<R, Args...>(+t);
    }
);
// clang-format on

// clang-format off
template <class T, class... Args>
concept FunctionPointerWith = (
    FunctionPointer<T> &&
    requires (T t, Args&&... args) {
        t(::sus::mem::forward<Args>(args)...);
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
        { t(::sus::mem::forward<Args>(args)...) } -> std::convertible_to<R>;
    }
);

template <class T, class... Args>
concept CallableObjectWithConst = (
    !FunctionPointer<T> &&
    requires (const T& t, Args&&... args) {
     t(::sus::mem::forward<Args>(args)...);
    }
);

template <class T, class R, class... Args>
concept CallableObjectReturnsMut = (
    !FunctionPointer<T> &&
    requires (T& t, Args&&... args) {
        { t(::sus::mem::forward<Args>(args)...) } -> std::convertible_to<R>;
    }
);

template <class T, class... Args>
concept CallableObjectWithMut = (
    !FunctionPointer<T> &&
    requires (T& t, Args&&... args) {
        t(::sus::mem::forward<Args>(args)...);
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
