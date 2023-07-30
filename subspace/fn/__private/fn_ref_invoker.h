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

#include <functional>

#include "subspace/mem/forward.h"
#include "subspace/mem/move.h"

namespace sus::fn::__private {

union Storage {
  void (*fnptr)();
  void* object;
};

/// Functions to call a functor `F` that is stored in `Storage`. The choice of
/// function encodes which member of `Storage` holds the functor.
template <class F>
struct Invoker {
  /// Calls the `F` in `Storage`, allowing mutable overlaods, when it is a
  /// function pointer.
  template <class R, class... Args>
  static R fnptr_call_mut(const union Storage& s, Args... args) {
    F f = reinterpret_cast<F>(s.fnptr);
    return std::invoke(*f, ::sus::forward<Args>(args)...);
  }

  /// Calls the `F` in `Storage`, as an lvalue, when it is a callable object.
  template <class R, class... Args>
  static R object_call_mut(const union Storage& s, Args... args) {
    F& f = *static_cast<F*>(s.object);
    return std::invoke(f, ::sus::forward<Args>(args)...);
  }

  /// Calls the `F` in `Storage`, as an rvalue, when it is a callable object.
  template <class R, class... Args>
  static R object_call_once(const union Storage& s, Args... args) {
    F& f = *static_cast<F*>(s.object);
    return std::invoke(::sus::move(f), ::sus::forward<Args>(args)...);
  }

  /// Calls the `F` in `Storage`, allowing only const overloads, when it is a
  /// function pointer.
  template <class R, class... Args>
  static R fnptr_call_const(const union Storage& s, Args... args) {
    const F f = reinterpret_cast<const F>(s.fnptr);
    return std::invoke(*f, ::sus::forward<Args>(args)...);
  }

  /// Calls the `F` in `Storage`, as a const object, when it is a callable
  /// object.
  template <class R, class... Args>
  static R object_call_const(const union Storage& s, Args... args) {
    const F& f = *static_cast<const F*>(s.object);
    return std::invoke(f, ::sus::forward<Args>(args)...);
  }
};

/// A function pointer type that matches all the invoke functions the `Invoker`.
template <class R, class... CallArgs>
using InvokeFnPtr = R (*)(const union Storage& s, CallArgs... args);

}  // namespace sus::fn::__private
