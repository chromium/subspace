// Copyright 2023 Google LLC
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

#include "subdoc/lib/gen/search.h"

#include <sstream>

#include "sus/prelude.h"

namespace subdoc::gen {

std::string split_for_search(std::string_view s) noexcept {
  if (s.size() == 0u) return std::string();

  bool is_camel_case = false;
  for (usize i; i < s.size() - 1u; i += 1u) {
    if (s[i] >= 'a' && s[i] <= 'z' && s[i + 1u] >= 'A' && s[i + 1u] <= 'Z') {
      is_camel_case = true;
    }
  }

  std::stringstream split_string;
  const char* query = ":_";
  if (is_camel_case) {
    query = ":_ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  }
  auto last = 0_usize, start = 0_usize;
  while (true) {
    auto found = s.find_first_of(query, start);
    if (found == std::string::npos) {
      split_string << std::string_view(s).substr(last);
      break;
    }
    auto append = std::string_view(s).substr(last, found - last);
    if (append.size() > 0u) split_string << append << std::string_view(" ");

    auto view = std::string_view(s).substr(found);
    if (view.starts_with(":") || view.starts_with("_")) {
      last = found + 1u;
      start = found + 1u;
    } else {
      last = found;
      start = found + 1;
    }
  }

  return sus::move(split_string).str();
}

}  // namespace subdoc::gen
