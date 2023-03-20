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

// Used to determine if compiling under MSVC (or clang-cl which acts like a
// drop-in replacement and has matching behaviour).
#if defined(_MSC_VER)
#define SUS_COMPILER_IS_MSVC 1
#else
#define SUS_COMPILER_IS_MSVC 0
#endif

// Used to determine if compiling under clang (but not clang-cl).
#if defined(__clang__) && !defined(_MSC_VER)
#define SUS_COMPILER_IS_CLANG 1
#else
#define SUS_COMPILER_IS_CLANG 0
#endif

// Used to determine if compiling under clang-cl.
#if defined(__clang__) && defined(_MSC_VER)
#define SUS_COMPILER_IS_CLANG_CL 1
#else
#define SUS_COMPILER_IS_CLANG_CL 0
#endif

// Used to determine if compiling under gcc.
#if defined(__GNUC__) && !defined(__clang__)
#define SUS_COMPILER_IS_GCC 1
#else
#define SUS_COMPILER_IS_GCC 0
#endif

#if SUS_COMPILER_IS_MSVC
#define sus_if_msvc(x) x
#define sus_if_msvc_else(x, y) x
#else
#define sus_if_msvc(x)
#define sus_if_msvc_else(x, y) y
#endif

#if SUS_COMPILER_IS_CLANG
#define sus_if_clang(x) x
#define sus_if_clang_else(x, y) x
#else
#define sus_if_clang(x)
#define sus_if_clang_else(x, y) y
#endif
