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

#include "subdoc/lib/gen/generate_head.h"

namespace subdoc::gen {

void generate_head(HtmlWriter& html, std::string_view title_string,
                   const Options& options) noexcept {
  {
    auto head = html.open_head();
    {
      auto title = head.open_title();
      title.write_text(title_string);
    }
    {
      auto default_stylesheet_link = head.open_link();
      default_stylesheet_link.add_rel("stylesheet");
      default_stylesheet_link.add_href(options.default_stylesheet_path);
    }
  }
  html.write_empty_line();
}

}  // namespace subdoc::gen
