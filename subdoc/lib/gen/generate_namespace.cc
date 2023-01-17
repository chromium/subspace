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
#include "subdoc/lib/gen/generate_function.h"
#include "subdoc/lib/gen/generate_head.h"
#include "subdoc/lib/gen/generate_record.h"
#include "subdoc/lib/gen/html_writer.h"
#include "subspace/containers/slice.h"

namespace subdoc::gen {

namespace {

void generate_namespace_overview(HtmlWriter::OpenDiv& namespace_div,
                                 std::string_view name,
                                 const NamespaceElement& element) {
  auto section_div = namespace_div.open_div();
  section_div.add_class("section");
  section_div.add_class("overview");

  {
    auto header_div = section_div.open_div();
    header_div.add_class("section-header");

    if (element.namespace_name.which() == Namespace::Tag::Named) {
      auto span = header_div.open_span();
      span.write_text("Namespace");
    }
    {
      auto name_anchor = header_div.open_a();
      name_anchor.add_href("#");
      name_anchor.add_class("namespace-name");
      name_anchor.write_text(name);
    }
  }
}

void generate_namespace_functions(
    HtmlWriter::OpenDiv& namespace_div, const NamespaceElement& element,
    sus::Slice<const sus::Tuple<std::string_view, const FunctionId&>>
        functions) {
  if (functions.is_empty()) return;

  auto section_div = namespace_div.open_div();
  section_div.add_class("section");
  section_div.add_class("functions");

  {
    auto functions_header_div = section_div.open_div();
    functions_header_div.add_class("section-header");
    functions_header_div.write_text("Functions");
  }
  {
    for (auto&& [name, function_id] : functions) {
      generate_function(section_div, element.functions.at(function_id),
                        /*is_static=*/false);
    }
  }
}
}  // namespace

void generate_namespace(const NamespaceElement& element,
                        const Options& options) noexcept {
  // The namespace path includes the namespace we're generating for, so drop
  // that one.
  sus::Slice<const Namespace> short_namespace_path =
      element.namespace_path.as_ref()[{1u, element.namespace_path.len() - 1u}];

  // For display in the html, we use the full path name of the namespace.
  std::string name = namespace_with_path_to_string(short_namespace_path,
                                                   element.namespace_name);
  std::string name_for_file = [&]() {
    if (element.namespace_name.which() == Namespace::Tag::Global) {
      // We're generating the global namespace, which will go in
      // `global-ns.html`. Namespaces can't be named `namespace` so this can't
      // collide with a real namespace `global::namespace`.
      return std::string("global-namespace");
    } else {
      // Otherwise, just use the local name of the namespace.
      return element.name;
    }
  }();

  const std::filesystem::path path =
      construct_html_file_path(options.output_root, short_namespace_path,
                               sus::Slice<const std::string>(), name_for_file);
  std::filesystem::create_directories(path.parent_path());
  auto html = HtmlWriter(open_file_for_writing(path).unwrap());

  generate_head(html, name, options);

  auto body = html.open_body();

  auto namespace_div = body.open_div();
  namespace_div.add_class("namespace");
  generate_namespace_overview(mref(namespace_div), name, element);

  sus::Vec<sus::Tuple<std::string_view, const FunctionId&>> sorted_functions;
  for (const auto& [function_id, function_element] : element.functions) {
    sorted_functions.push(sus::tuple(function_element.name, function_id));
  }
  sorted_functions.sort_unstable_by(
      [](const sus::Tuple<std::string_view, const FunctionId&>& a,
         const sus::Tuple<std::string_view, const FunctionId&>& b) {
        return a.get_ref<0>() <=> b.get_ref<0>();
      });

  generate_namespace_functions(mref(namespace_div), element,
                               sorted_functions.as_ref());

  // Recurse into namespaces and records.
  for (const auto& [u, element] : element.namespaces) {
    generate_namespace(element, options);
  }
  for (const auto& [u, element] : element.records) {
    generate_record(element, options);
  }
}

}  // namespace subdoc::gen
