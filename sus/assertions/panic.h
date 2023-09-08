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

#include <stdlib.h>

#include <source_location>
#include <string>
#include <string_view>

#include "sus/macros/builtin.h"
#include "sus/macros/compiler.h"
#include "sus/macros/inline.h"

#if __has_attribute(not_tail_called)
#define _sus__not_tail_called __attribute__((not_tail_called))
#else
#define _sus__not_tail_called
#endif

#if __has_attribute(nomerge)
#define _sus__nomerge __attribute__((nomerge))
#else
#define _sus__nomerge
#endif

#define _sus__crash_attributes _sus__not_tail_called _sus__nomerge

namespace sus::assertions {

namespace __private {
void print_panic_message(const char& msg,
                         const std::source_location& location) noexcept;
void print_panic_message(std::string_view msg,
                         const std::source_location& location) noexcept;
void print_panic_location(const std::source_location& location) noexcept;
}  // namespace __private

/// Terminate the program.
///
/// The default behaviour of this function is to `abort()`. The behaviour of
/// this function can be overridden by defining a `SUS_PROVIDE_PANIC_HANDLER()`
/// macro when compiling the library.
///
/// The panic message will be printed to stderr before aborting. This behaviour
/// can be overridden by defining a `SUS_PROVIDE_PRINT_PANIC_LOCATION_HANDLER()`
/// macro when compiling the library.
///
/// # Safety
///
/// If `SUS_PROVIDE_PANIC_HANDLER()` is defined, the macro _must_ not return or
/// Undefined Behaviour will result.
[[noreturn]] _sus__crash_attributes inline void panic(
    const std::source_location location =
        std::source_location::current()) noexcept {
#if defined(SUS_PROVIDE_PRINT_PANIC_LOCATION_HANDLER)
  SUS_PROVIDE_PRINT_PANIC_LOCATION_HANDLER(location);
#else
  __private::print_panic_location(location);
#endif
#if defined(SUS_PROVIDE_PANIC_HANDLER)
  SUS_PROVIDE_PANIC_HANDLER();
#else
  abort();
#endif
}

/// Terminate the program, after printing a message.
///
/// The default behaviour of this function is to abort(). The behaviour of this
/// function can be overridden by defining a `SUS_PROVIDE_PANIC_HANDLER()` macro
/// when compiling the library.
///
/// The panic message will be printed to stderr before aborting. This behaviour
/// can be overridden by defining a `SUS_PROVIDE_PRINT_PANIC_MESSAGE_HANDLER()`
/// macro when compiling the library.
[[noreturn]] _sus__crash_attributes inline void panic_with_message(
    const char* msg, const std::source_location location =
                         std::source_location::current()) noexcept {
#if defined(SUS_PROVIDE_PRINT_PANIC_MESSAGE_HANDLER)
  SUS_PROVIDE_PRINT_PANIC_MESSAGE_HANDLER(msg, location);
#else
  __private::print_panic_message(msg, location);
#endif
#if defined(SUS_PROVIDE_PANIC_HANDLER)
  SUS_PROVIDE_PANIC_HANDLER();
#else
  abort();
#endif
}

[[noreturn]] _sus__crash_attributes inline void panic_with_message(
    std::string_view msg, const std::source_location location =
                              std::source_location::current()) noexcept {
#if defined(SUS_PROVIDE_PRINT_PANIC_MESSAGE_HANDLER)
  SUS_PROVIDE_PRINT_PANIC_MESSAGE_HANDLER(msg, location);
#else
  __private::print_panic_message(msg, location);
#endif
#if defined(SUS_PROVIDE_PANIC_HANDLER)
  SUS_PROVIDE_PANIC_HANDLER();
#else
  abort();
#endif
}

[[noreturn]] _sus__crash_attributes inline void panic_with_message(
    const std::string& msg, const std::source_location location =
                                std::source_location::current()) noexcept {
#if defined(SUS_PROVIDE_PRINT_PANIC_MESSAGE_HANDLER)
  SUS_PROVIDE_PRINT_PANIC_MESSAGE_HANDLER(msg.c_str(), location);
#else
  __private::print_panic_message(msg.c_str(), location);
#endif
#if defined(SUS_PROVIDE_PANIC_HANDLER)
  SUS_PROVIDE_PANIC_HANDLER();
#else
  abort();
#endif
}

}  // namespace sus::assertions

#undef _sus__not_tail_called
#undef _sus__nomerge
#undef __sus__crash_attributes

namespace sus {
using ::sus::assertions::panic;
using ::sus::assertions::panic_with_message;
}  // namespace sus
