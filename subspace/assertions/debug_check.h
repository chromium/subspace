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

#include "subspace/assertions/check.h"
#include "subspace/macros/assume.h"
#include "subspace/macros/compiler.h"

/// Check a condition in debug builds, causing a `panic()` if the condition
/// fails. Nothing is checked in release builds.
///
/// The condition must not have side effects, and should not call any functions
/// or methods (including operator methods), as it will often not have any
/// effect in that case (especially in MSVC https://godbolt.org/z/7T7E3P598)
#define sus_debug_check(...) _sus__debug_check_impl(__VA_ARGS__)

#if !defined(NDEBUG)
#define _sus__debug_check_impl(...) ::sus::check(__VA_ARGS__)
#else
#define _sus__debug_check_impl(...) \
  sus_assume(::sus::marker::unsafe_fn, __VA_ARGS__)
#endif
