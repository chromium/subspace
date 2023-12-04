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

#include <cstdlib>
#include <source_location>
#include <string>
#include <string_view>

#include "sus/macros/builtin.h"
#include "sus/macros/inline.h"

namespace sus {

/// Checking for (e.g. [`check`]($sus::assertions::check)) and handling
/// (e.g. [`unreachable`]($sus::assertions::unreachable)) unexpected runtime
/// conditions.
namespace assertions {}
}  // namespace sus

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
/// The default behaviour of this function is to `__builtin_trap()` when
/// possible and [`std::abort()`](
/// https://en.cppreference.com/w/cpp/utility/program/abort) otherwise.
/// The behaviour of this function can be overridden by defining a
/// `SUS_PROVIDE_PANIC_HANDLER()` macro when compiling the library.
///
/// The panic message will be printed to stderr before aborting. This behaviour
/// can be overridden by defining a `SUS_PROVIDE_PRINT_PANIC_LOCATION_HANDLER()`
/// macro when compiling. The same handler must be used as when building the
/// library itself. So if used as a shared library, it can not be modified by
/// the calling code.
///
/// # Safety
///
/// If `SUS_PROVIDE_PANIC_HANDLER()` is defined, the macro _must_ not return or
/// Undefined Behaviour will result.
[[noreturn]] sus_always_inline void panic(
    const std::source_location location =
        std::source_location::current()) noexcept {
#if defined(SUS_PROVIDE_PRINT_PANIC_LOCATION_HANDLER)
  SUS_PROVIDE_PRINT_PANIC_LOCATION_HANDLER(location);
#else
  __private::print_panic_location(location);
#endif

#if defined(SUS_PROVIDE_PANIC_HANDLER)
  SUS_PROVIDE_PANIC_HANDLER();
#elif __has_builtin(__builtin_trap)
  __builtin_trap();
#else
  std::abort();
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
[[noreturn]] sus_always_inline void panic_with_message(
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
  std::abort();
#endif
}

[[noreturn]] sus_always_inline void panic_with_message(
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
  std::abort();
#endif
}

[[noreturn]] sus_always_inline void panic_with_message(
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
  std::abort();
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
