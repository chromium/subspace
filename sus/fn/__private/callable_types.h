// Copyright 2023 Google LLC
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
#include <functional>
#include <type_traits>

#include "sus/mem/forward.h"
#include "sus/mem/move.h"

namespace sus::fn::__private {

/// Whether a functor `F` is a function pointer that is callable with `Args...`
/// and will return a value that can be stored as `R`.
template <class F, class R, class... Args>
concept FunctionPointer =
    std::is_pointer_v<std::decay_t<F>> && requires(F f, Args... args) {
      { std::invoke(*f, args...) } -> std::convertible_to<R>;
    };

/// Whether a functor `T` is a function pointer.
template <class T>
concept IsFunctionPointer =
    std::is_pointer_v<T> && std::is_function_v<std::remove_pointer_t<T>>;

/// Whether a functor `T` can convert to a function pointer, typically this
/// means it's a captureless lambda.
template <class F>
concept ConvertsToFunctionPointer = requires(F f) {
  { +f } -> IsFunctionPointer;
};

/// Whether a functor `F` is a callable object (with an `operator()`) that is
/// once-callable as an rvalue with `Args...` and will return a value that can
/// be stored as `R`.
template <class F, class R, class... Args>
concept CallableOnceMut =
    !FunctionPointer<F, R, Args...> && requires(F&& f, Args... args) {
      {
        std::invoke(::sus::move(f), ::sus::forward<Args>(args)...)
      } -> std::convertible_to<R>;
    };

/// Whether a functor `F` is a callable object (with an `operator()`) that is
/// mutable-callable as an lvalue with `Args...` and will return a value that
/// can be stored as `R`.
template <class F, class R, class... Args>
concept CallableMut =
    !FunctionPointer<F, R, Args...> && requires(F& f, Args... args) {
      {
        std::invoke(f, ::sus::forward<Args>(args)...)
      } -> std::convertible_to<R>;
    };

/// Whether a functor `F` is a callable object (with an `operator()`) that is
/// const-callable with `Args...` and will return a value that can be stored as
/// `R`.
template <class F, class R, class... Args>
concept CallableConst =
    !FunctionPointer<F, R, Args...> && requires(const F& f, Args... args) {
      {
        std::invoke(f, ::sus::forward<Args>(args)...)
      } -> std::convertible_to<R>;
    };

}  // namespace sus::fn::__private
