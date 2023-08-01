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

#include "sus/macros/compiler.h"
#include "sus/marker/unsafe.h"

/// Tells the compiler that condition `expr` is true and to optimize for it.
///
/// The `expr` must not have side effects, and should not call any functions or
/// methods (including operator methods), as it will often not have any effect
/// in that case (especially in MSVC https://godbolt.org/z/7T7E3P598)
///
/// `[[assume(expr)]]` will replace this macro in C++23. */
///
/// # Safety
/// If the condition `expr` were to actually be false, Undefined Behaviour will
/// result.
// clang-format off
#define sus_assume(unsafe_fn, expr)                                       \
  static_assert(std::same_as<std::decay_t<decltype(unsafe_fn)>,           \
                             ::sus::marker::UnsafeFnMarker>);             \
  sus_if_clang(_Pragma("clang diagnostic push"))                          \
  sus_if_clang(_Pragma("clang diagnostic ignored \"-Wassume\""))      \
  sus_if_msvc_else(__assume, sus_if_clang_else(__builtin_assume, if))(expr) \
  sus_if_msvc_else(, sus_if_clang_else(, {} else __builtin_unreachable())) \
  sus_if_clang(_Pragma("clang diagnostic pop"))
// clang-format on
