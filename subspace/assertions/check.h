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

#include "subspace/assertions/panic.h"
#include "subspace/macros/inline.h"

namespace sus::assertions {

constexpr sus_always_inline void check(
    bool cond,
    const std::source_location location = std::source_location::current()) {
  if (!cond) [[unlikely]]
    ::sus::panic(location);
}

constexpr sus_always_inline void check_with_message(
    bool cond,
    /* TODO: string view type, or format + args */ const char& msg,
    const std::source_location location = std::source_location::current()) {
  if (!cond) [[unlikely]]
    ::sus::panic_with_message(msg, location);
}

}  // namespace sus::assertions

// Promote check() and check_with_message() into the `sus` namespace.
namespace sus {
using ::sus::assertions::check;
using ::sus::assertions::check_with_message;
}  // namespace sus
