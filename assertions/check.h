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

#include "assertions/panic.h"

namespace sus {

constexpr inline void check(bool cond) {
  // Expect `!cond` to be false.
  if (!cond) [[unlikely]] {
    ::sus::panic();
  }
}

constexpr inline void check_with_message(
    bool cond,
    /* TODO: string view type */ const char& msg) {
  // Expect `!cond` to be false.
  if (!cond) [[unlikely]] {
    ::sus::panic_with_message(msg);
  }
}

}  // namespace sus
