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

#pragma once

#include <string>

#include "subdoc/lib/gen/generate_cpp_path.h"
#include "subdoc/lib/gen/html_writer.h"
#include "subdoc/lib/gen/options.h"
#include "sus/iter/iterator.h"
#include "sus/prelude.h"

namespace subdoc::gen {

void generate_search_header(HtmlWriter::OpenMain& main) noexcept;

void generate_search_title(
    HtmlWriter::OpenDiv& div,
    sus::iter::IntoIterator<CppPathElement> auto ii) noexcept {
  auto search_title = div.open_h(1u, HtmlWriter::SingleLine);
  search_title.add_search_weight(2_f32);
  search_title.add_class("search-title");

  auto s = std::string();
  for (CppPathElement e : sus::move(ii).into_iter()) {
    if (e.type == CppPathProject) continue;
    if (s.size() > 0u) s += std::string_view("::");
    s += std::string_view(e.name);
  }
  search_title.write_text(s);
}

}  // namespace subdoc::gen
