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

#include "subdoc/lib/gen/generate_source_link.h"

namespace subdoc::gen {

void generate_source_link(HtmlWriter::OpenDiv& div,
                          const CommentElement& element) {
  if (element.source_link.is_some()) {
    const SourceLink& link = element.source_link.as_value();

    auto source_link_div = div.open_div(HtmlWriter::SingleLine);
    source_link_div.add_class("src");
    source_link_div.add_class("rightside");
    {
      auto a = source_link_div.open_a();
      {
        std::ostringstream str;
        str << link.file_path << "#" << link.line;
        a.add_href(sus::move(str).str());
      }
      a.write_text("source");
    }
  }
}

}  // namespace subdoc::gen
