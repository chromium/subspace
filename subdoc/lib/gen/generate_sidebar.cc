
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

#include "subdoc/lib/gen/generate_sidebar.h"

#include "subdoc/lib/gen/files.h"

namespace subdoc::gen {

void generate_sidebar(HtmlWriter::OpenBody& body, const Database& db,
                      std::string_view pretitle, std::string_view title,
                      std::string_view subtitle, sus::Vec<SidebarLink> links,
                      const Options& options) noexcept {
  auto nav = body.open_nav();
  nav.add_class("sidebar");
  {
    auto a = nav.open_a();
    a.add_class("sidebar-logo-link");
    a.add_href(construct_html_url_for_namespace(db.global));

    if (!options.project_logo.empty()) {
      auto border = a.open_div();
      border.add_class("sidebar-logo-border");

      auto logo = border.open_img();
      logo.add_class("sidebar-logo");
      logo.add_src(options.project_logo);
    }
  }
  {
    auto pretitle_div = nav.open_div();
    pretitle_div.add_class("sidebar-pretitle");
    pretitle_div.add_class("sidebar-text");
    pretitle_div.write_text(pretitle);
  }
  {
    auto title_div = nav.open_div();
    title_div.add_class("sidebar-title");
    title_div.add_class("sidebar-text");
    auto title_a = title_div.open_a();
    title_a.add_href("#");
    title_a.write_text(title);
  }
  {
    auto subtitle_div = nav.open_div();
    subtitle_div.add_class("sidebar-subtitle");
    subtitle_div.add_class("sidebar-text");
    subtitle_div.write_text(subtitle);
  }

  auto links_div = nav.open_div();
  links_div.add_class("sidebar-links");
  links_div.add_class("sidebar-text");
  {
    auto ul = links_div.open_ul();
    for (const SidebarLink& link : links) {
      auto li = ul.open_li();
      auto a = li.open_a();
      a.add_href(link.href);
      a.write_text(link.text);
    }
  }
}

}  // namespace subdoc::gen