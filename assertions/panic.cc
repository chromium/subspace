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

#include "assertions/panic.h"

#include <stdio.h>

namespace sus {

// Defined outside the header to avoid inlining, to optimize for code size.
[[noreturn]] void panic_with_message(
    /* TODO: string view type, or format + args */ const char& msg) {
  // TODO: allow configuring messages away at build time.
  fprintf(stderr, "PANIC! %s\n", &msg);
  // TODO: allow configuring this to some other method of aborting.
  ::abort();
}

}  // namespace sus
