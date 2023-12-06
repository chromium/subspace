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

#include "sus/assertions/panic.h"

/// Verifies that the input, evaluated to a `bool`, is true. Otherwise, it will
/// [`panic`]($sus_panic), printing a message and terminating the program.
///
/// See [`sus_check_with_message`]($sus_check_with_message) to add a
/// message to the display of the panic.
///
/// The displayed output can be controlled by overriding the behaviour of
/// [`sus_panic`]($sus_panic) as described there.
#define sus_check(...)               \
  if (!(__VA_ARGS__)) [[unlikely]] { \
    sus_panic();                     \
  }                                  \
  static_assert(true)

/// Verifies that the input `cond`, evaluated to a `bool`, is true. Otherwise,
/// it will [`panic`]($sus_panic), printing a customized message, and
/// terminating the program.
///
/// Use [`sus_check`]($sus_check) when there's nothing useful to add
/// in the message.
///
/// The displayed output can be controlled by overriding the behaviour of
/// [`sus_panic`]($sus_panic) as described there. If the
/// `SUS_PROVIDE_PRINT_PANIC_MESSAGE_HANDLER` macro does not consume the `msg`,
/// this macro will avoid instantiating it at all.
#define sus_check_with_message(cond, msg) \
  if (!(cond)) [[unlikely]] {             \
    sus_panic_with_message(msg);          \
  }                                       \
  static_assert(true)
