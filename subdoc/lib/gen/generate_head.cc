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

void generate_head(HtmlWriter& html, std::string_view title,
                   std::string_view description,
                   const Options& options) noexcept {
  {
    auto head = html.open_head();
    {
      auto meta = head.open_meta();
      meta.add_name("generator");
      meta.add_content("subdoc");
    }
    {
      auto meta = head.open_meta();
      meta.add_name("viewport");
      meta.add_content("width=device-width, initial-scale=1");
    }
    {
      auto title_tag = head.open_title();
      if (!title.empty()) {
        title_tag.write_text(title);
        title_tag.write_text(" - ");
      }
      title_tag.write_text(options.project_name);
    }
    {
      auto meta = head.open_meta();
      meta.add_name("description");
      meta.add_content(description);
    }
    for (const std::string& path : options.stylesheets) {
      auto stylesheet_link = head.open_link();
      stylesheet_link.add_rel("stylesheet");
      stylesheet_link.add_href(path);
    }
    for (const auto& [i, favicon] : options.favicons.iter().enumerate()) {
      auto favicon_link = head.open_link();
      if (i == 0u)
        favicon_link.add_rel("icon");
      else
        favicon_link.add_rel("alternate icon");
      favicon_link.add_type(favicon.mime);
      favicon_link.add_href(favicon.path);
    }
  }
  html.write_empty_line();
}

}  // namespace subdoc::gen
