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
#include "sus/macros/nodebug.h"

namespace sus {

/// Checking for (e.g. [`sus_check`]($sus_check)) and handling
/// (e.g. [`sus::panic`]($sus::panic),
/// [`sus::unreachable`]($sus::unreachable)) unexpected runtime
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
void print_panic_message(std::string_view msg,
                         const PanicLocation& location) noexcept;
}  // namespace __private

/// Terminate the program, after printing a message.
///
/// The default behaviour of this function is to `__builtin_trap()` when
/// possible and [`std::abort()`](
/// https://en.cppreference.com/w/cpp/utility/program/abort) otherwise. The
/// behaviour of this function can be overridden by defining a
/// `SUS_PROVIDE_PANIC_HANDLER()` macro when compiling. The panic
/// message will be printed to stderr before aborting. This behaviour can be
/// overridden by defining a `SUS_PROVIDE_PRINT_PANIC_MESSAGE_HANDLER()` macro
/// when compiling. Message handling can be suppressed entirely by
/// defining a `SUS_PANIC_ELIDE_MESSAGE` macro. This can be advantageous for
/// optimised builds, as it turns `panic` into just calling the panic handler.
/// `SUS_PROVIDE_PANIC_HANDLER`, `SUS_PROVIDE_PRINT_PANIC_MESSAGE_HANDLER`, and
/// `SUS_PANIC_ELIDE_MESSAGE` must all have a consistent definition when
/// building Subspace and any binaries that link against it. This means you can
/// change them for different build configurations, but cannot change them for
/// different targets within the same build configuration. If used as a shared
/// library, the compilation of calling code must match how the Subspace library
/// was built.
///
/// The `SUS_PROVIDE_PRINT_PANIC_MESSAGE_HANDLER()` macro receives two arguments:
/// * A message, which is a `const char*`, a `std::string_view` or a
///   `std::string`. Overloads should be used to handle each case.
/// * A [`PanicLocation`]($sus::assertions::PanicLocation).
///
/// # Safety
///
/// If `SUS_PROVIDE_PANIC_HANDLER()` is defined, the macro _must_ not return or
/// Undefined Behaviour will result.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
[[noreturn]] _sus_always_inline _sus_nodebug
inline void panic(std::string_view message = "",
                  PanicLocation loc = PanicLocation::current()) noexcept
{
#if !defined(SUS_PANIC_ELIDE_MESSAGE)
#  if defined(SUS_PROVIDE_PRINT_PANIC_MESSAGE_HANDLER)
    SUS_PROVIDE_PRINT_PANIC_MESSAGE_HANDLER(message, loc);
#  else
    ::sus::assertions::__private::print_panic_message(message, loc);
#  endif // SUS_PROVIDE_PRINT_PANIC_MESSAGE_HANDLER
#endif // SUS_PANIC_ELIDE_MESSAGE

#if defined(SUS_PROVIDE_PANIC_HANDLER)
  SUS_PROVIDE_PANIC_HANDLER();
#elif __has_builtin(__builtin_trap)
  __builtin_trap();
#else
  std::abort();
#endif // SUS_PROVIDE_PANIC_HANDLER
}
#pragma GCC diagnostic pop

}  // namespace sus::assertions

namespace sus {
  using ::sus::assertions::panic;
}
