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
#include "subdoc/lib/gen/generate_cpp_path.h"
#include "subdoc/lib/gen/generate_function.h"
#include "subdoc/lib/gen/generate_head.h"
#include "subdoc/lib/gen/generate_record.h"
#include "subdoc/lib/gen/html_writer.h"
#include "sus/assertions/unreachable.h"
#include "sus/containers/slice.h"
#include "sus/prelude.h"

namespace subdoc::gen {

namespace {

using SortedNamespaceByName = sus::Tuple<std::string_view, u32, NamespaceId>;
using SortedFunctionByName = sus::Tuple<std::string_view, u32, FunctionId>;
using SortedRecordByName = sus::Tuple<std::string_view, u32, RecordId>;

std::string namespace_display_name(
    const NamespaceElement& element,
    const sus::Slice<const NamespaceElement*>& ancestors,
    const Options& options) noexcept {
  std::ostringstream out;

  if (element.namespace_name == Namespace::Tag::Global) {
    for (auto e : generate_cpp_path_for_namespace(element, ancestors, options)
                      .into_iter())
      out << e.name;
  } else {
    std::string project_name;
    for (auto [i, e] :
         generate_cpp_path_for_namespace(element, ancestors, options)
             .into_iter()
             .enumerate()) {
      if (i == 0u)
        project_name = sus::move(e.name);
      else {
        if (i > 1u) out << "::";
        out << sus::move(e.name);
      }
    }
    out << " - " << sus::move(project_name);
  }
  return sus::move(out).str();
}

void generate_namespace_overview(
    HtmlWriter::OpenDiv& namespace_div, const NamespaceElement& element,
    const sus::Slice<const NamespaceElement*>& ancestors,
    const Options& options) {
  auto section_div = namespace_div.open_div();
  section_div.add_class("section");
  section_div.add_class("overview");

  {
    auto header_div = section_div.open_div();
    header_div.add_class("section-header");
    if (element.namespace_name != Namespace::Tag::Global) {
      auto span = header_div.open_span();
      span.write_text("Namespace");
    }
    auto it = generate_cpp_path_for_namespace(element, ancestors, options)
                  .into_iter()
                  .enumerate();
    for (auto [i, e] : it) {
      if (e.link_href.empty()) {
        auto span = header_div.open_span();
        span.write_text(e.name);
      } else {
        if (i > 0u) {
          auto span = header_div.open_span(HtmlWriter::SingleLine);
          span.add_class("namespace-dots");
          span.write_text("::");
        }
        auto ancestor_anchor = header_div.open_a();
        ancestor_anchor.add_class([&e]() {
          switch (e.type) {
            case CppPathProject: return "project-name";
            case CppPathNamespace: return "namespace-name";
            case CppPathRecord: return "type-name";
            case CppPathFunction: sus::unreachable();
          }
          sus::unreachable();
        }());
        ancestor_anchor.add_href(e.link_href);
        ancestor_anchor.write_text(e.name);
      }
    }
    if (element.has_comment()) {
      auto desc_div = header_div.open_div();
      desc_div.add_class("description");
      desc_div.add_class("long");
      desc_div.write_html(element.comment.full());
    }
  }
}

void generate_namespace_references(
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
    auto items_list = section_div.open_ul();
    items_list.add_class("section-items");
    items_list.add_class("item-table");

    for (auto&& [name, sort_key, id] : namespaces)
      generate_namespace_reference(items_list, element.namespaces.at(id));
  }
}

void generate_record_references(HtmlWriter::OpenDiv& namespace_div,
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
    auto items_list = section_div.open_ul();
    items_list.add_class("section-items");
    items_list.add_class("item-table");

    for (auto&& [name, sort_key, key] : records) {
      generate_record_reference(items_list, element.records.at(key));
    }
  }
}

enum GenerateFunctionType {
  GenerateFunctions,
  GenerateOperators,
};

void generate_function_references(HtmlWriter::OpenDiv& namespace_div,
                                  const NamespaceElement& element,
                                  sus::Slice<SortedFunctionByName> functions,
                                  GenerateFunctionType type) {
  if (functions.is_empty()) return;

  auto section_div = namespace_div.open_div();
  section_div.add_class("section");
  section_div.add_class("functions");

  {
    auto header_div = section_div.open_div();
    header_div.add_class("section-header");
    switch (type) {
      case GenerateFunctions: header_div.write_text("Functions"); break;
      case GenerateOperators: header_div.write_text("Operators"); break;
    }
  }
  {
    auto items_list = section_div.open_ul();
    items_list.add_class("section-items");
    items_list.add_class("item-table");

    u32 overload_set;
    std::string_view prev_name;
    for (auto&& [name, sort_key, function_id] : functions) {
      if (name == prev_name)
        overload_set += 1u;
      else
        overload_set = 0u;
      prev_name = name;
      generate_function_reference(items_list, element.functions.at(function_id),
                                  /*is_static=*/false, overload_set);
    }
  }
}

}  // namespace

void generate_namespace(const NamespaceElement& element,
                        sus::Vec<const NamespaceElement*> ancestors,
                        const Options& options) noexcept {
  if (element.is_empty()) return;
  if (element.hidden()) return;

  const std::filesystem::path path =
      construct_html_file_path_for_namespace(options.output_root, element);
  std::filesystem::create_directories(path.parent_path());

  auto html = HtmlWriter(open_file_for_writing(path).unwrap());
  generate_head(html, namespace_display_name(element, ancestors, options),
                options);

  auto body = html.open_body();

  auto namespace_div = body.open_div();
  namespace_div.add_class("namespace");
  generate_namespace_overview(namespace_div, element, ancestors, options);

  {
    sus::Vec<SortedNamespaceByName> sorted;
    for (const auto& [key, sub_element] : element.namespaces) {
      if (sub_element.hidden()) continue;
      if (sub_element.is_empty()) continue;

      sorted.push(sus::tuple(sub_element.name, sub_element.sort_key, key));
    }
    sorted.sort_unstable_by(
        [](const SortedNamespaceByName& a, const SortedNamespaceByName& b) {
          auto ord = a.at<0>() <=> b.at<0>();
          if (ord != 0) return ord;
          return a.at<1>() <=> b.at<1>();
        });

    generate_namespace_references(namespace_div, element, sorted.as_slice());
  }

  {
    sus::Vec<SortedRecordByName> classes;
    sus::Vec<SortedRecordByName> unions;
    for (const auto& [key, sub_element] : element.records) {
      if (sub_element.hidden()) continue;

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

    generate_record_references(namespace_div, element, classes.as_slice(),
                               RecordType::Class);
    generate_record_references(namespace_div, element, unions.as_slice(),
                               RecordType::Union);
  }

  {
    sus::Vec<SortedFunctionByName> sorted_functions;
    sus::Vec<SortedFunctionByName> sorted_operators;
    for (const auto& [function_id, sub_element] : element.functions) {
      if (sub_element.hidden()) continue;

      if (!sub_element.is_operator) {
        sorted_functions.push(
            sus::tuple(sub_element.name, sub_element.sort_key, function_id));
      } else {
        sorted_operators.push(
            sus::tuple(sub_element.name, sub_element.sort_key, function_id));
      }
    }
    sorted_functions.sort_unstable_by(
        [](const SortedFunctionByName& a, const SortedFunctionByName& b) {
          auto ord = a.at<0>() <=> b.at<0>();
          if (ord != 0) return ord;
          return a.at<1>() <=> b.at<1>();
        });
    sorted_operators.sort_unstable_by(
        [](const SortedFunctionByName& a, const SortedFunctionByName& b) {
          auto ord = a.at<0>() <=> b.at<0>();
          if (ord != 0) return ord;
          return a.at<1>() <=> b.at<1>();
        });
    generate_function_references(namespace_div, element, sorted_functions,
                                 GenerateFunctions);
    generate_function_references(namespace_div, element, sorted_operators,
                                 GenerateOperators);
  }

  // Recurse into namespaces, records and functions.
  ancestors.push(&element);
  for (const auto& [u, sub_element] : element.namespaces) {
    if (sub_element.hidden()) continue;
    generate_namespace(sub_element, sus::clone(ancestors), options);
  }
  for (const auto& [u, sub_element] : element.records) {
    if (sub_element.hidden()) continue;
    generate_record(sub_element, ancestors, sus::vec(), options);
  }
  {
    sus::Vec<SortedFunctionByName> sorted;
    for (const auto& [function_id, sub_element] : element.functions) {
      if (sub_element.hidden()) continue;
      sorted.push(
          sus::tuple(sub_element.name, sub_element.sort_key, function_id));
    }
    sorted.sort_unstable_by(
        [](const SortedFunctionByName& a, const SortedFunctionByName& b) {
          auto ord = a.at<0>() <=> b.at<0>();
          if (ord != 0) return ord;
          return a.at<1>() <=> b.at<1>();
        });
    u32 overload_set;
    std::string_view prev_name;
    for (auto&& [name, sort_key, function_id] : sorted) {
      if (name == prev_name)
        overload_set += 1u;
      else
        overload_set = 0u;
      prev_name = name;
      generate_function(element.functions.at(function_id), ancestors,
                        overload_set, options);
    }
  }
}

void generate_namespace_reference(HtmlWriter::OpenUl& items_list,
                                  const NamespaceElement& element) noexcept {
  auto item_li = items_list.open_li();
  item_li.add_class("section-item");

  {
    auto item_div = item_li.open_div();
    item_div.add_class("item-name");

    auto name_link = item_div.open_a();
    name_link.add_class("namespace-name");
    if (!element.hidden()) {
      name_link.add_href(construct_html_file_path_for_namespace(
                             std::filesystem::path(), element)
                             .string());
    } else {
      llvm::errs() << "WARNING: Reference to hidden NamespaceElement "
                   << element.name << " in namespace "
                   << element.namespace_path;
    }
    name_link.write_text(element.name);
  }
  {
    auto desc_div = item_li.open_div();
    desc_div.add_class("description");
    desc_div.add_class("short");
    if (element.has_comment()) desc_div.write_html(element.comment.summary());
  }
}

}  // namespace subdoc::gen
