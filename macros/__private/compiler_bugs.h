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

#include "macros/builtin.h"
#include "macros/compiler.h"

// TODO: https://github.com/llvm/llvm-project/issues/56394
#if __clang_major__ <= 16  // TODO: Update when the bug is fixed.
#define sus_clang_bug_56394(...) __VA_ARGS__
#define sus_clang_bug_56394_else(...)
#else
#define sus_clang_bug_56394(...)
#define sus_clang_bug_56394_else(...) __VA_ARGS__
#endif

// TODO: https://github.com/llvm/llvm-project/issues/58835
#if __clang_major__ <= 16  // TODO: Update when the bug is fixed.
#define sus_clang_bug_58835(...) __VA_ARGS__
#define sus_clang_bug_58835_else(...)
#else
#define sus_clang_bug_58835(...)
#define sus_clang_bug_58835_else(...) __VA_ARGS__
#endif

// TODO: https://github.com/llvm/llvm-project/issues/54040
// Aggregate initialization in Clang doesn't work when:
// * A destructor is present in the aggregate.
// * A member of the aggregate has a concept-defined conversion operator.
#if __clang_major__ <= 16  // TODO: Update when the bug is fixed.
#define sus_clang_bug_54040(...) __VA_ARGS__
#define sus_clang_bug_54040_else(...)
#else
#define sus_clang_bug_54040(...)
#define sus_clang_bug_54040_else(...) __VA_ARGS__
#endif

// TODO: https://github.com/llvm/llvm-project/issues/54040
// Aggregate initialization fails on template classes due to lack of CTAD for
// aggregates.
#if __clang_major__ <= 16  // TODO: Update when the bug is fixed.
#define sus_clang_bug_54050(...) __VA_ARGS__
#define sus_clang_bug_54050_else(...)
#else
#define sus_clang_bug_54050(...)
#define sus_clang_bug_54050_else(...) __VA_ARGS__
#endif

// TODO: https://github.com/llvm/llvm-project/issues/58836
#if __clang_major__ <= 16  // TODO: Update when the bug is fixed.
#define sus_clang_bug_58836(...) __VA_ARGS__
#define sus_clang_bug_58836_else(...)
#else
#define sus_clang_bug_58836(...)
#define sus_clang_bug_58836_else(...) __VA_ARGS__
#endif

// TODO: https://github.com/llvm/llvm-project/issues/58837
#if __clang_major__ <= 16  // TODO: Update when the bug is fixed.
#define sus_clang_bug_58837(...) __VA_ARGS__
#define sus_clang_bug_58837_else(...)
#else
#define sus_clang_bug_58837(...)
#define sus_clang_bug_58837_else(...) __VA_ARGS__
#endif

// TODO: https://github.com/llvm/llvm-project/issues/58859
#if __clang_major__ <= 16  // TODO: Update when the bug is fixed.
#define sus_clang_bug_58859(...) __VA_ARGS__
#define sus_clang_bug_58859_else(...)
#else
#define sus_clang_bug_58859(...)
#define sus_clang_bug_58859_else(...) __VA_ARGS__
#endif
