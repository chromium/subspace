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

#include "sus/macros/__private/compiler_bugs.h"
#include "sus/macros/compiler.h"

/// A macro to provide a platform-agnostic `no_unique_address` attribute name.
///
/// This is required at all because MSVC uses its own syntax to force users to
/// opt into it:
/// https://devblogs.microsoft.com/cppblog/msvc-cpp20-and-the-std-cpp20-switch/
//
// clang-format off
#define _sus_no_unique_address \
    sus_clang_bug_49358_else(sus_if_msvc(msvc::)no_unique_address)
// clang-format on
