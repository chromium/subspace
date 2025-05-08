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

#include "panic.h"
#include "sus/assertions/panic.h"
#include "sus/macros/builtin.h"
#include "sus/marker/unsafe.h"

namespace sus::assertions {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
/// Indicates to the developer that the location should not be reached, and
/// terminates the program with a [`panic`]($sus::panic).
///
/// This is similar to [`std::unreachable!`](
/// https://doc.rust-lang.org/stable/std/macro.unreachable.html) in Rust,
/// and in the default build configuration will print an error message
/// indicating the location where the panic occurred.
///
/// Note that this is *not* the same as [`std::unreachable`](
/// https://en.cppreference.com/w/cpp/utility/unreachable) in C++ which is
/// Undefined Behaviour if reached. It is closer to [`std::abort`](
/// https://en.cppreference.com/w/cpp/utility/program/abort) except built on
/// top of [`sus::panic`]($sus::panic).
/// The Subspace library matches the safer behaviour of Rust to avoid confusion
/// and security bugs when working across languages. Use
/// [`sus::unreachable_unchecked`]($sus::unreachable_unchecked) to
/// indicate to the compiler the code is not reachable.
[[noreturn, gnu::always_inline, gnu::nodebug]] inline void unreachable(PanicLocation loc = PanicLocation::current()) {
  [[clang::always_inline]] ::sus::panic("entered unreachable code", loc);
}

/// Indicates to the compiler that the location will never be reached, allowing
/// it to optimize code generation accordingly. If this function is actually
/// reached, Undefined Behaviour will occur.
///
/// This is similar to [`std::unreachable`](
/// https://en.cppreference.com/w/cpp/utility/unreachable).
///
/// # Safety
/// This function must never actually be reached, or Undefined Behaviour occurs.
[[noreturn, gnu::always_inline, gnu::nodebug]] inline void unreachable_unchecked(::sus::marker::UnsafeFnMarker) {
#if __has_builtin(__builtin_unreachable)
  __builtin_unreachable();
#else
  __assume(false);
#endif
}
#pragma GCC diagnostic pop
}  // namespace ::sus::assertions

namespace sus {
  using sus::assertions::unreachable;
  using sus::assertions::unreachable_unchecked;
}
