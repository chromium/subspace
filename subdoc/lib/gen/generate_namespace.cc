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

#include "subdoc/lib/gen/generate_namespace.h"

#include <filesystem>

#include "subdoc/lib/gen/files.h"
#include "subdoc/lib/gen/generate_head.h"
#include "subdoc/lib/gen/html_writer.h"
#include "subspace/containers/slice.h"

namespace subdoc::gen {

namespace {

void generate_namespace_overview(HtmlWriter::OpenDiv& namespace_div,
                                 const NamespaceElement& element) {
  auto section_div = namespace_div.open_div();
  section_div.add_class("section");
  section_div.add_class("overview");

  {
    auto record_header_div = section_div.open_div();
    record_header_div.add_class("section-header");
    {
      auto record_type_span = record_header_div.open_span();
      record_type_span.write_text("Namespace");
    }
    {
      auto name_anchor = record_header_div.open_a();
      name_anchor.add_href("#");
      name_anchor.add_class("namespace-name");
      name_anchor.write_text(element.name);
    }
  }
}

}  // namespace

void generate_namespace(const NamespaceElement& element,
                        const Options& options) noexcept {
  const std::filesystem::path path = construct_html_file_path(
      options.output_root, element.namespace_path.as_ref(),
      sus::Slice<const std::string>(), element.name);
  std::filesystem::create_directories(path.parent_path());
  auto html = HtmlWriter(open_file_for_writing(path).unwrap());

  generate_head(html, element.name, options);

  auto body = html.open_body();

  auto namespace_div = body.open_div();
  namespace_div.add_class("namespace");
  generate_namespace_overview(mref(namespace_div), element);
}

}  // namespace subdoc::gen
