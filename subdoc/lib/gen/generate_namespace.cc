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
#include "sus/containers/slice.h"

namespace subdoc::gen {

namespace {

using SortedNamespaceByName = sus::Tuple<std::string_view, u32, NamespaceId>;
using SortedFunctionByName = sus::Tuple<std::string_view, u32, FunctionId>;
using SortedRecordByName = sus::Tuple<std::string_view, u32, RecordId>;

std::string namespace_display_name(const NamespaceElement& element) noexcept {
  // The namespace path includes the namespace we're generating for, so drop
  // that one.
  sus::Slice<Namespace> short_namespace_path =
      element.namespace_path.as_slice()["1.."_r];

  // For display in the html, we use the full path name of the namespace.
  return namespace_with_path_to_string(short_namespace_path,
                                       element.namespace_name);
}

void generate_namespace_overview(HtmlWriter::OpenDiv& namespace_div,
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
      name_anchor.write_text(namespace_display_name(element));
    }
    if (element.has_comment()) {
      auto desc_div = header_div.open_div();
      desc_div.add_class("description");
      desc_div.write_html(element.comment.raw_text);
    }
  }
}

void generate_namespace_namespaces(
    HtmlWriter::OpenDiv& namespace_div, const NamespaceElement& element,
    sus::Slice<SortedNamespaceByName> namespaces) {
  if (namespaces.is_empty()) return;

  auto section_div = namespace_div.open_div();
  section_div.add_class("section");
  section_div.add_class("namespaces");

  {
    auto header_div = section_div.open_div();
    header_div.add_class("section-header");
    header_div.write_text("Namespaces");
  }
  {
    for (auto&& [name, sort_key, id] : namespaces)
      generate_namespace_reference(section_div, element.namespaces.at(id));
  }
}

void generate_namespace_records(HtmlWriter::OpenDiv& namespace_div,
                                const NamespaceElement& element,
                                sus::Slice<SortedRecordByName> records,
                                RecordType record_type) {
  if (records.is_empty()) return;

  auto section_div = namespace_div.open_div();
  section_div.add_class("section");
  section_div.add_class("records");
  switch (record_type) {
    case RecordType::Class: [[fallthrough]];
    case RecordType::Struct: section_div.add_class("classes"); break;
    case RecordType::Union: section_div.add_class("unions"); break;
  }

  {
    auto header_div = section_div.open_div();
    header_div.add_class("section-header");
    switch (record_type) {
      case RecordType::Class: [[fallthrough]];
      case RecordType::Struct: header_div.write_text("Classes"); break;
      case RecordType::Union: header_div.write_text("Unions"); break;
    }
  }
  {
    for (auto&& [name, sort_key, key] : records) {
      generate_record_reference(section_div, element.records.at(key));
    }
  }
}

void generate_namespace_functions(
    HtmlWriter::OpenDiv& namespace_div, const NamespaceElement& element,
    sus::Slice<SortedFunctionByName> functions) {
  if (functions.is_empty()) return;

  auto section_div = namespace_div.open_div();
  section_div.add_class("section");
  section_div.add_class("functions");

  {
    auto header_div = section_div.open_div();
    header_div.add_class("section-header");
    header_div.write_text("Functions");
  }
  {
    u32 overload_set;
    std::string_view prev_name;
    for (auto&& [name, sort_key, function_id] : functions) {
      if (name == prev_name)
        overload_set += 1u;
      else
        overload_set = 0u;
      prev_name = name;
      generate_function(section_div, element.functions.at(function_id),
                        /*is_static=*/false, overload_set);
    }
  }
}

}  // namespace

void generate_namespace(const NamespaceElement& element,
                        const Options& options) noexcept {
  const std::filesystem::path path =
      construct_html_file_path_for_namespace(options.output_root, element);
  std::filesystem::create_directories(path.parent_path());

  auto html = HtmlWriter(open_file_for_writing(path).unwrap());
  generate_head(html, namespace_display_name(element), options);

  auto body = html.open_body();

  auto namespace_div = body.open_div();
  namespace_div.add_class("namespace");
  generate_namespace_overview(namespace_div, element);

  {
    sus::Vec<SortedNamespaceByName> sorted;
    for (const auto& [key, sub_element] : element.namespaces) {
      sorted.push(sus::tuple(sub_element.name, sub_element.sort_key, key));
    }
    sorted.sort_unstable_by(
        [](const SortedNamespaceByName& a, const SortedNamespaceByName& b) {
          auto ord = a.at<0>() <=> b.at<0>();
          if (ord != 0) return ord;
          return a.at<1>() <=> b.at<1>();
        });

    generate_namespace_namespaces(namespace_div, element,
                                  sorted.as_slice());
  }

  {
    sus::Vec<SortedRecordByName> classes;
    sus::Vec<SortedRecordByName> unions;
    for (const auto& [key, sub_element] : element.records) {
      switch (sub_element.record_type) {
        case RecordType::Class: [[fallthrough]];
        case RecordType::Struct:
          classes.push(sus::tuple(sub_element.name, sub_element.sort_key, key));
          break;
        case RecordType::Union:
          unions.push(sus::tuple(sub_element.name, sub_element.sort_key, key));
          break;
      }
    }
    classes.sort_unstable_by(
        [](const SortedRecordByName& a, const SortedRecordByName& b) {
          auto ord = a.at<0>() <=> b.at<0>();
          if (ord != 0) return ord;
          return a.at<1>() <=> b.at<1>();
        });
    unions.sort_unstable_by(
        [](const SortedRecordByName& a, const SortedRecordByName& b) {
          auto ord = a.at<0>() <=> b.at<0>();
          if (ord != 0) return ord;
          return a.at<1>() <=> b.at<1>();
        });

    generate_namespace_records(namespace_div, element, classes.as_slice(),
                               RecordType::Class);
    generate_namespace_records(namespace_div, element, unions.as_slice(),
                               RecordType::Union);
  }

  {
    sus::Vec<SortedFunctionByName> sorted;
    for (const auto& [function_id, sub_element] : element.functions) {
      sorted.push(
          sus::tuple(sub_element.name, sub_element.sort_key, function_id));
    }
    sorted.sort_unstable_by(
        [](const SortedFunctionByName& a, const SortedFunctionByName& b) {
          auto ord = a.at<0>() <=> b.at<0>();
          if (ord != 0) return ord;
          return a.at<1>() <=> b.at<1>();
        });

    generate_namespace_functions(namespace_div, element,
                                 sorted.as_slice());
  }

  // Recurse into namespaces and records.
  for (const auto& [u, sub_element] : element.namespaces) {
    generate_namespace(sub_element, options);
  }
  for (const auto& [u, sub_element] : element.records) {
    generate_record(sub_element, options);
  }
}

void generate_namespace_reference(HtmlWriter::OpenDiv& section_div,
                                  const NamespaceElement& element) noexcept {
  auto item_div = section_div.open_div();
  item_div.add_class("section-item");

  {
    auto name_link = item_div.open_a();
    name_link.add_class("namespace-name");
    name_link.add_href(
        construct_html_file_path_for_namespace(std::filesystem::path(), element)
            .string());
    name_link.write_text(element.name);
  }
  if (element.has_comment()) {
    auto desc_div = item_div.open_div();
    desc_div.add_class("description");
    desc_div.write_html(element.comment.summary());
  }
}

}  // namespace subdoc::gen
