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

#include <type_traits>

#include "subspace/mem/forward.h"
#include "subspace/mem/move.h"

namespace sus::fn {

// clang-format off

/// Invokes a `FnOnce` type, ensuring that it is invoked as an rvalue.
///
/// TODO: A Subspace clang-tidy check encourages the use of run_once() any time
/// a function invokes through type that has been matched against the `FnOnce`
/// concept.
template <class F, class... Args>
  requires(std::is_rvalue_reference_v<F&&> &&
           !std::is_const_v<std::remove_reference_t<F&&>>)
constexpr inline decltype(auto) run_once(F&& fn_once, Args&&... args) {
  return ::sus::move(fn_once)(::sus::forward<Args>(args)...);
}

/// Invokes a `FnMut` type, ensuring that it is invoked as a mutable lvalue
/// object.
///
/// This encourages the `FnMut` to be either stored or passed by value which
/// defines the scope of effect for the `FnMut`'s mutation during execution.
///
/// TODO: A Subspace clang-tidy check encourages the use of run_mut() any time a
/// function invokes through type that has been matched against the `FnMut`
/// concept.
template <class F, class... Args>
constexpr inline decltype(auto) run_mut(F& fn_mut, Args&&... args) {
  return fn_mut(::sus::forward<Args>(args)...);
}

/// Invokes a `Fn` type, ensuring that it is invoked as a const object.
///
/// TODO: A Subspace clang-tidy check encourages the use of run() any time a
/// function invokes through type that has been matched against the `Fn`
/// concept.
template <class F, class... Args>
constexpr inline decltype(auto) run(const F& fn, Args&&... args) {
  return fn(::sus::forward<Args>(args)...);
}

// clang-format on

}  // namespace sus::fn

// Promote the run helpers into the `sus` namespace.
namespace sus {
using ::sus::fn::run;
using ::sus::fn::run_mut;
using ::sus::fn::run_once;
}  // namespace sus
