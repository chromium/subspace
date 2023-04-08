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

#include "subspace/macros/compiler.h"

/// Tells the compiler that the variable does not alias with any other variable,
/// allowing it to optimize around that.
///
/// If used on a variable that does alias, Undefined Behaviour may result.
#define sus_noalias_var sus_if_msvc_else(__restrict, __restrict__)
