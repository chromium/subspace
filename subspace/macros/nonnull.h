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

/// Defines an attribute to place on a function definition that declares all
/// pointer arguments are not null. To actually receive null would be Undefined
/// Behaviour, as the compiler is free to optimize for them never being null.
/// Thus it should only be used on internal methods or methods that are marked
/// as unsafe.
///
/// This is placed as a modifier on the method, before its return type, such as
/// `sus_nonnull_fn void f(i32* p);
#define sus_nonnull_fn sus_if_msvc_else(, __attribute__((nonnull)))

/// Defines an attribute to place before the type of a pointer-type function
/// parameter that declares the pointer is not null. To actually receive null
/// would be Undefined Behaviour, as the compiler is free to optimize for it
/// never being null. Thus it should only be used on internal methods or methods
/// that are marked as unsafe.
///
/// This is placed as a modifier on a function parameter before its type, such
/// as in `void f(sus_nonnull_arg i32* ptr)`.
#define sus_nonnull_arg sus_if_msvc(_Notnull_)

/// Defines a pointer variable or function parameter to not be null. If null is
/// assigned to the variable, Undefined Behaviour results. Thus for function
/// parameters it should only be used on internal methods or methods that are
/// marked as unsafe.
///
/// This is placed as a modifier on the pointer `*`, such as in
/// `i32* sus_nonnull_var ptr_`.
#define sus_nonnull_var sus_if_clang(_Nonnull)
