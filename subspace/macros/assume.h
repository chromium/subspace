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

#include "subspace/macros/compiler.h"
#include "subspace/marker/unsafe.h"

/// Tells the compiler that condition `x` is true and to optimize for it.
///
/// `[[assume(x)]]` will replace this macro in C++23. */
///
/// # Safety
/// If the condition `x` were to actually be false, Undefined Behaviour will
/// result.
#define sus_assume(unsafe_fn, x)                                \
  static_assert(std::same_as<std::decay_t<decltype(unsafe_fn)>, \
                             ::sus::marker::UnsafeFnMarker>);   \
  sus_if_msvc_else(__assume, __builtin_assume)(x)
