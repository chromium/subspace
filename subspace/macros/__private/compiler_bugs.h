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

#include "subspace/macros/builtin.h"
#include "subspace/macros/compiler.h"

// TODO: https://github.com/llvm/llvm-project/issues/56394
#if defined(__clang__) && \
    __clang_major__ > 0  // TODO: Update when the bug is fixed.
#define sus_clang_bug_56394(...) __VA_ARGS__
#define sus_clang_bug_56394_else(...)
#else
#define sus_clang_bug_56394(...)
#define sus_clang_bug_56394_else(...) __VA_ARGS__
#endif

// TODO: https://github.com/llvm/llvm-project/issues/58835
#if defined(__clang__) && \
    __clang_major__ > 0  // TODO: Update when the bug is fixed.
#define sus_clang_bug_58835(...) __VA_ARGS__
#define sus_clang_bug_58835_else(...)
#else
#define sus_clang_bug_58835(...)
#define sus_clang_bug_58835_else(...) __VA_ARGS__
#endif

// TODO: https://github.com/llvm/llvm-project/issues/54040
// Aggregate initialization via () paren syntax.
//
// Support for this landed and was reverted in clang 16, then relanded but is
// currently broken in 16:
// https://github.com/llvm/llvm-project/issues/61145
#if defined(__clang__) && (__clang_major__ <= 17)
#define sus_clang_bug_54040(...) __VA_ARGS__
#define sus_clang_bug_54040_else(...)
#else
#define sus_clang_bug_54040(...)
#define sus_clang_bug_54040_else(...) __VA_ARGS__
#endif

// TODO: https://github.com/llvm/llvm-project/issues/54050
// Aggregate initialization fails on template classes due to lack of CTAD for
// aggregates.
//
// There are still bugs with aggregate init in Clang 17:
// https://github.com/llvm/llvm-project/issues/61145
#if defined(__clang__) && \
    __clang_major__ <= 17  // TODO: Update when the bug is fixed.
#define sus_clang_bug_54050(...) __VA_ARGS__
#define sus_clang_bug_54050_else(...)
#else
#define sus_clang_bug_54050(...)
#define sus_clang_bug_54050_else(...) __VA_ARGS__
#endif

// TODO: https://github.com/llvm/llvm-project/issues/58836
#if defined(__clang__) && \
    __clang_major__ > 0  // TODO: Update when the bug is fixed.
#define sus_clang_bug_58836(...) __VA_ARGS__
#define sus_clang_bug_58836_else(...)
#else
#define sus_clang_bug_58836(...)
#define sus_clang_bug_58836_else(...) __VA_ARGS__
#endif

// TODO: https://github.com/llvm/llvm-project/issues/58837
#if defined(__clang__) && \
    __clang_major__ > 0  // TODO: Update when the bug is fixed.
#define sus_clang_bug_58837(...) __VA_ARGS__
#define sus_clang_bug_58837_else(...)
#else
#define sus_clang_bug_58837(...)
#define sus_clang_bug_58837_else(...) __VA_ARGS__
#endif

// TODO: https://github.com/llvm/llvm-project/issues/58859
#if defined(__clang__) && \
    __clang_major__ > 0  // TODO: Update when the bug is fixed.
#define sus_clang_bug_58859(...) __VA_ARGS__
#define sus_clang_bug_58859_else(...)
#else
#define sus_clang_bug_58859(...)
#define sus_clang_bug_58859_else(...) __VA_ARGS__
#endif

// TODO: https://github.com/llvm/llvm-project/issues/56394
#if defined(__clang__) && \
    __clang_major__ > 0  // TODO: Update when the bug is fixed.
#define sus_clang_bug_56394(...) __VA_ARGS__
#define sus_clang_bug_56394_else(...)
#else
#define sus_clang_bug_56394(...)
#define sus_clang_bug_56394_else(...) __VA_ARGS__
#endif

// clang reports is_trivial as true incorrectly.
// TODO: https://github.com/llvm/llvm-project/issues/60697
#if defined(__clang__) && \
    __clang_major__ > 0  // TODO: Update when the bug is fixed.
#define sus_clang_bug_60697(...) __VA_ARGS__
#define sus_clang_bug_60697_else(...)
#else
#define sus_clang_bug_60697(...)
#define sus_clang_bug_60697_else(...) __VA_ARGS__
#endif

// GCC internal compiler error when Ord fails.
// TODO: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107542
#if defined(__GNUC__) && \
    __GNUC__ < 13  // TODO: Update if the bug fix is backported.
#define sus_gcc_bug_107542(...) __VA_ARGS__
#define sus_gcc_bug_107542_else(...)
#else
#define sus_gcc_bug_107542(...)
#define sus_gcc_bug_107542_else(...) __VA_ARGS__
#endif

// TODO: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=108169
// GCC considers class-type template parameters as const.
#if defined(__GNUC__) && __GNUC__ > 0  // TODO: Update when the bug is fixed.
#define sus_gcc_bug_108169(...) __VA_ARGS__
#define sus_gcc_bug_108169_else(...)
#else
#define sus_gcc_bug_108169(...)
#define sus_gcc_bug_108169_else(...) __VA_ARGS__
#endif

// TODO: https://github.com/llvm/llvm-project/issues/49358
// Clang-cl doesn't support either [[no_unique_address]] nor
// [[msvc::no_unique_address]]
#if _MSC_VER && defined(__clang__) && \
    __clang_major__ > 0  // TODO: Update when the bug is fixed.
#define sus_clang_bug_49358(...) __VA_ARGS__
#define sus_clang_bug_49358_else(...)
#else
#define sus_clang_bug_49358(...)
#define sus_clang_bug_49358_else(...) __VA_ARGS__
#endif
