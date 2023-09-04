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

#include "sus/assertions/panic.h"

#include <stdio.h>

namespace sus::assertions::__private {

// Defined outside the header to avoid fprintf in the header.
void print_panic_message(const char* msg,
                         const std::source_location& location) noexcept {
  fprintf(stderr, "PANIC! at '%s', %s:%lu:%lu\n", msg, location.file_name(),
          static_cast<unsigned long>(location.line()),
          static_cast<unsigned long>(location.column()));
}

void print_panic_message(std::string_view msg,
                         const std::source_location& location) noexcept {
  fprintf(stderr, "PANIC! at '");
  for (char c : msg) fprintf(stderr, "%c", c);
  fprintf(stderr, "', %s:%lu:%lu\n", location.file_name(),
          static_cast<unsigned long>(location.line()),
          static_cast<unsigned long>(location.column()));
}

void print_panic_location(const std::source_location& location) noexcept {
  fprintf(stderr, "PANIC! at %s:%lu:%lu\n", location.file_name(),
          static_cast<unsigned long>(location.line()),
          static_cast<unsigned long>(location.column()));
}

}  // namespace sus::assertions::__private
