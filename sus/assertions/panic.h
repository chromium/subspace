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
#include <string>
#include <string_view>

#include "sus/macros/builtin.h"
#include "sus/macros/inline.h"

namespace sus {

/// Checking for (e.g. [`sus_check`]($sus_check)) and handling
/// (e.g. [`sus::panic`]($sus::panic),
/// [`sus_unreachable`]($sus_unreachable)) unexpected runtime
/// conditions.
namespace assertions {}
}  // namespace sus

namespace sus::assertions {

/// Records the location where a panic occured.
struct PanicLocation {
  const char* file_name;
  const unsigned line;
  const unsigned column;

  constexpr static PanicLocation current(
      const char* file_name = __builtin_FILE(),
      unsigned line = __builtin_LINE(),
  #if __has_builtin(__builtin_COLUMN)
      unsigned column = __builtin_COLUMN()
  #else
      unsigned column = 0u
  #endif
  ) noexcept {
    return PanicLocation{
        .file_name = file_name,
        .line = line,
        .column = column,
    };
  }
};

namespace __private {
void print_panic_message(const char& msg,
                         const PanicLocation& location) noexcept;
void print_panic_message(std::string_view msg,
                         const PanicLocation& location) noexcept;
void print_panic_location(const PanicLocation& location) noexcept;
}  // namespace __private

#if defined(SUS_PROVIDE_PRINT_PANIC_LOCATION_HANDLER)
#  define _sus_panic_location_handler(loc) \
    SUS_PROVIDE_PRINT_PANIC_LOCATION_HANDLER(loc)
#else
#  define _sus_panic_location_handler(loc) \
    ::sus::assertions::__private::print_panic_location(loc)
#endif

#if defined(SUS_PROVIDE_PRINT_PANIC_MESSAGE_HANDLER)
#  define _sus_panic_message_handler(msg, loc) \
    SUS_PROVIDE_PRINT_PANIC_LOCATION_HANDLER(msg, loc)
#else
#  define _sus_panic_message_handler(msg, loc) \
    ::sus::assertions::__private::print_panic_message(msg, loc)
#endif

#if defined(SUS_PROVIDE_PANIC_HANDLER)
#  define _sus_panic_handler() SUS_PROVIDE_PANIC_HANDLER()
#elif __has_builtin(__builtin_trap)
#  define _sus_panic_handler() __builtin_trap()
#else
#  define _sus_panic_handler() std::abort()
#endif

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
/// the calling code. The `SUS_PROVIDE_PRINT_PANIC_LOCATION_HANDLER()` macro
/// receives a single argument which is a [`PanicLocation`](
/// $sus::assertions::PanicLocation).
///
/// # Safety
///
/// If `SUS_PROVIDE_PANIC_HANDLER()` is defined, the macro _must_ not return or
/// Undefined Behaviour will result.
[[noreturn, gnu::always_inline, gnu::nodebug]] inline void panic(::sus::assertions::PanicLocation loc = ::sus::assertions::PanicLocation::current()) noexcept
{
  _sus_panic_location_handler(loc);
  _sus_panic_handler();
}

/// Terminate the program, after printing a message.
///
/// The default behaviour of this function is to abort(). The behaviour of this
/// function can be overridden by defining a `SUS_PROVIDE_PANIC_HANDLER()` macro
/// when compiling the library.
///
/// The panic message will be printed to stderr before aborting. This behaviour
/// can be overridden by defining a `SUS_PROVIDE_PRINT_PANIC_MESSAGE_HANDLER()`
/// macro when compiling the library. The
/// `SUS_PROVIDE_PRINT_PANIC_MESSAGE_HANDLER()` macro receives two arguments:
/// * A message, which is a `const char*`, a `std::string_view` or a
///   `std::string`. Overloads should be used to handle each case.
/// * A [`PanicLocation`]($sus::assertions::PanicLocation).
/// If the `SUS_PROVIDE_PRINT_PANIC_MESSAGE_HANDLER` macro does not consume the
/// `msg`, this macro will avoid instantiating it at all.
[[noreturn, gnu::always_inline, gnu::nodebug]] inline void panic_with_message(std::string_view message, ::sus::assertions::PanicLocation loc = ::sus::assertions::PanicLocation::current()) noexcept
{
  _sus_panic_message_handler(message, loc);
  _sus_panic_handler();
}

}  // namespace sus::assertions

namespace sus {
  using ::sus::assertions::panic;
  using ::sus::assertions::panic_with_message;
}
