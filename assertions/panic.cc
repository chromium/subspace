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

namespace sus::__private {

// Defined outside the header to avoid fprintf in the header.
void print_panic_message(const char& msg) {
  fprintf(stderr, "PANIC! %s\n", &msg);
}

}  // namespace sus::__private
