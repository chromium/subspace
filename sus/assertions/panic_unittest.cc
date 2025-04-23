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

#ifdef TEST_MODULE
import sus;
#else
#include "sus/assertions/panic.h"
#endif

#include "fmt/format.h"
#include "googletest/include/gtest/gtest.h"
#include "panic.h"

// Incredibly, on Posix we can use [0-9] but on Windows we can't. Yet on Windows
// we can use `\d` and on Posix we can't (or it doesn't match).
#if GTEST_USES_SIMPLE_RE
#  define DIGIT "\\d"
#else
#  define DIGIT "[0-9]"
#endif

// TODO: add cases for `SUS_PROVIDE_PANIC_HANDLER` and
// `SUS_PROVIDE_PRINT_PANIC_MESSAGE_HANDLER` when it's clear how to test them
// with modules.
#if defined(SUS_PANIC_ELIDE_MESSAGE)
#  define EXPECTED_MESSAGE(message) "^$"
#else
#  define EXPECTED_MESSAGE(message) "^PANIC! at " message ".*panic_unittest.cc:" DIGIT "+:" DIGIT "+\n$"
#endif

namespace sus {
namespace {


TEST(PanicDeathTest, Panic) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(sus::panic(),
               EXPECTED_MESSAGE(""));
#endif
}

TEST(PanicDeathTest, WithMessage) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(sus::panic("hello world"), EXPECTED_MESSAGE("'hello world', "));
#endif
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(sus::panic(
                   std::string_view("hello world123").substr(0u, 11u)),
               EXPECTED_MESSAGE("'hello world', "));
#endif
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(sus::panic(std::string("hello world")),
               EXPECTED_MESSAGE("'hello world', "));
#endif
}

}  // namespace
}  // namespace sus
