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
      auto meta = head.open_meta();
      meta.add_property("og:type");
      meta.add_content("website");
    }
    {
      auto meta = head.open_meta();
      meta.add_property("og:site_name");
      meta.add_content(options.project_name);
    }

    auto page_title = std::string(title);
    if (!page_title.empty()) {
      page_title += " - ";
    }
    page_title += options.project_name;

    {
      auto title_tag = head.open_title();
      title_tag.write_text(page_title);
    }
    {
      auto meta = head.open_meta();
      meta.add_property("og:title");
      meta.add_content(page_title);
    }
    {
      auto meta = head.open_meta();
      meta.add_name("description");
      meta.add_content(description);
    }
    {
      auto meta = head.open_meta();
      meta.add_property("og:description");
      meta.add_content(description);
    }

    // Searching via https://pagefind.app.
    //
    // The CSS comes before the site-defined CSS in order for the site to
    // override things.
    {
      auto css = head.open_link();
      css.add_href("pagefind/pagefind-ui.css");
      css.add_rel("stylesheet");
    }
    {
      auto script = head.open_script(HtmlWriter::SingleLine);
      script.add_src("pagefind/pagefind-ui.js");
    }
    {
      auto script = head.open_script();
      script.write_html(
          "window.addEventListener('DOMContentLoaded', (event) => {");
      script.write_html(
          "  new PagefindUI({element: '#search', showSubResults: true});");
      script.write_html(  //
          "});");
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
    if (Option<const FavIcon&> icon = options.favicons.first();
        icon.is_some()) {
      auto meta = head.open_meta();
      meta.add_property("og:image");
      meta.add_content(icon.as_value().path);
    }
  }
  html.write_empty_line();
}

}  // namespace subdoc::gen
