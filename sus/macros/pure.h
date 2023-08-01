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

#include "sus/macros/compiler.h"

/// Used to mark a function as "pure", meaning it does not change any values
/// outside of its own scope.
///
/// A pure function is allowed to dereference pointers, and access global
/// memory, but it may not change them. To do so can cause Undefined Behaviour.
#define sus_pure sus_if_msvc_else([[nodiscard]], __attribute__((pure)))

/// Used to mark a function as "const", meaning it does not change any values
/// outside of its own scope, and does not read global memory.
///
/// Functions that dereference pointers can not be marked `sus_pure_const`.
///
/// A const function is allowed to only read from its inputs and determine an
/// output from them, without going through pointers or accessing global memory.
/// To do so anyway can cause Undefined Behaviour.
#define sus_pure_const sus_if_msvc_else([[nodiscard]], __attribute__((const)))
