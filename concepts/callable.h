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

template <class F, class... Args>
concept FunctionPointer = requires(F f, Args&&... args) {
                            { (+f)(forward<Args>(args)...) };
                          };

template <class F, class R, class... Args>
concept FunctionPointerReturns = (FunctionPointer<F, Args...>) &&
                                 requires(F f, Args&&... args) {
                                   {
                                     (+f)(forward<Args>(args)...)
                                     } -> std::same_as<R>;
                                 };

template <class F, class... Args>
concept CallableObjectOnce = (!FunctionPointer<F, Args...>) &&
                             requires(F f, Args&&... args) {
                               { static_cast<F&&>(f)(forward<Args>(args)...) };
                             };

template <class F, class... Args>
concept CallableOnce = FunctionPointer<F> || CallableObjectOnce<F, Args...>;

template <class F, class... Args>
concept CallableObjectMut = (!FunctionPointer<F, Args...>) &&
                            requires(F f, Args&&... args) {
                              { static_cast<F&>(f)(forward<Args>(args)...) };
                            };

template <class F, class... Args>
concept CallableMut = FunctionPointer<F> || CallableObjectMut<F, Args...>;

template <class F, class... Args>
concept CallableObjectConst = (!FunctionPointer<F, Args...>) &&
                              requires(F f, Args&&... args) {
                                {
                                  static_cast<const F&>(f)(
                                      forward<Args>(args)...)
                                };
                              };

template <class F, class... Args>
concept CallableConst =
    FunctionPointer<F, Args...> || CallableObjectConst<F, Args...>;

}  // namespace sus::concepts::callable
