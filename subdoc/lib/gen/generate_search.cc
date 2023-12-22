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

#include "subdoc/lib/gen/generate_search.h"

#include "sus/prelude.h"

namespace subdoc::gen {

void generate_search_header(HtmlWriter::OpenMain& main) noexcept {
  auto search_header_div = main.open_div();
  search_header_div.add_class("search-header");

  {
    auto search_div = main.open_search();
    {
      auto search_form = search_div.open_form();
      search_form.add_class("search-form");

      auto search_input = search_form.open_input();
      search_input.add_class("search-input");
      search_input.add_name("search");
      search_input.add_autocomplete("off");
      search_input.add_spellcheck("false");
      search_input.add_placeholder("Click or press 'S' to search...");
      search_input.add_onblur(
          "this.placeholder = 'Click or press \\'S\\' to search...'");
      search_input.add_onfocus("this.placeholder = 'Type your search here.'");
    }
  }
}

}  // namespace subdoc::gen
