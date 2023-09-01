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
#include "subdoc/lib/gen/generate_concept.h"
#include "subdoc/lib/gen/generate_cpp_path.h"
#include "subdoc/lib/gen/generate_function.h"
#include "subdoc/lib/gen/generate_head.h"
#include "subdoc/lib/gen/generate_record.h"
#include "subdoc/lib/gen/generate_sidebar.h"
#include "subdoc/lib/gen/html_writer.h"
#include "subdoc/lib/gen/markdown_to_html.h"
#include "subdoc/lib/parse_comment.h"
#include "sus/assertions/unreachable.h"
#include "sus/collections/slice.h"
#include "sus/prelude.h"

namespace subdoc::gen {

namespace {

using SortedNamespaceByName = sus::Tuple<std::string_view, u32, NamespaceId>;
using SortedConceptByName = sus::Tuple<std::string_view, u32, ConceptId>;
using SortedFunctionByName = sus::Tuple<std::string_view, u32, FunctionId>;
using SortedRecordByName = sus::Tuple<std::string_view, u32, RecordId>;

std::string namespace_display_name(
    const NamespaceElement& element,
    sus::Slice<const NamespaceElement*> ancestors,
    const Options& options) noexcept {
  std::ostringstream out;

  if (element.namespace_name != Namespace::Tag::Global) {
    for (auto [i, e] :
         generate_cpp_path_for_namespace(element, ancestors, options)
             .into_iter()
             .enumerate()) {
      if (i > 0u) {  // First element is the project name.
        if (i > 1u) out << "::";
        out << sus::move(e.name);
      }
    }
  }
  return sus::move(out).str();
}

void generate_namespace_overview(HtmlWriter::OpenDiv& namespace_div,
                                 const NamespaceElement& element,
                                 sus::Slice<const NamespaceElement*> ancestors,
                                 const MarkdownToHtml& comment_html,
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
            case CppPathFunction:
              break;  // Function can't be an ancesor of a record.
            case CppPathConcept:
              break;  // Concept can't be an ancestor of a namespace.
          }
          sus::unreachable();
        }());
        ancestor_anchor.add_href(e.link_href);
        ancestor_anchor.write_text(e.name);
      }
    }
  }
  {
    auto desc_div = section_div.open_div();
    desc_div.add_class("description");
    desc_div.add_class("long");
    desc_div.write_html(comment_html.full_html);
  }
}

sus::Result<void, MarkdownToHtmlError> generate_namespace_references(
    HtmlWriter::OpenDiv& namespace_div, const NamespaceElement& element,
    sus::Slice<SortedNamespaceByName> namespaces,
    ParseMarkdownPageState& page_state) {
  auto section_div = namespace_div.open_div();
  section_div.add_class("section");
  section_div.add_class("namespaces");

  {
    auto header_div = section_div.open_div();
    header_div.add_class("section-header");
    auto header_name = header_div.open_a();
    header_name.add_name("namespaces");
    header_name.add_href("#namespaces");
    header_name.write_text("Namespaces");
  }
  {
    auto items_list = section_div.open_ul();
    items_list.add_class("section-items");
    items_list.add_class("item-table");

    for (auto&& [name, sort_key, id] : namespaces) {
      {
        auto item_li = items_list.open_li();
        item_li.add_class("section-item");

        if (auto result = generate_namespace_reference(
                item_li, element.namespaces.at(id), page_state);
            result.is_err()) {
          return sus::err(sus::move(result).unwrap_err());
        }
      }

      {
        sus::Vec<SortedNamespaceByName> sorted;
        for (const auto& [key, sub_element] :
             element.namespaces.at(id).namespaces) {
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

        for (auto&& [sub_name, sub_sort_key, sub_id] : sorted) {
          auto item_li = items_list.open_li();
          item_li.add_class("nested");
          item_li.add_class("section-item");
          if (auto result = generate_namespace_reference(
                  item_li, element.namespaces.at(id).namespaces.at(sub_id),
                  page_state);
              result.is_err()) {
            return sus::err(sus::move(result).unwrap_err());
          }
        }
      }
    }
  }

  return sus::ok();
}

sus::Result<void, MarkdownToHtmlError> generate_concept_references(
    HtmlWriter::OpenDiv& namespace_div, const NamespaceElement& element,
    sus::Slice<SortedConceptByName> concepts,
    ParseMarkdownPageState& page_state) {
  if (concepts.is_empty()) return sus::ok();

  auto section_div = namespace_div.open_div();
  section_div.add_class("section");
  section_div.add_class("concepts");

  {
    auto header_div = section_div.open_div();
    header_div.add_class("section-header");
    auto header_name = header_div.open_a();
    header_name.add_name("concepts");
    header_name.add_href("#concepts");
    header_name.write_text("Concepts");
  }
  {
    auto items_list = section_div.open_ul();
    items_list.add_class("section-items");
    items_list.add_class("item-table");

    for (auto&& [name, sort_key, key] : concepts) {
      if (auto result = generate_concept_reference(
              items_list, element.concepts.at(key), page_state);
          result.is_err()) {
        return sus::err(sus::move(result).unwrap_err());
      }
    }
  }

  return sus::ok();
}

sus::Result<void, MarkdownToHtmlError> generate_record_references(
    HtmlWriter::OpenDiv& namespace_div, const NamespaceElement& element,
    sus::Slice<SortedRecordByName> records, RecordType record_type,
    ParseMarkdownPageState& page_state) {
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
      case RecordType::Struct: {
        auto header_name = header_div.open_a();
        header_name.add_name("classes");
        header_name.add_href("#classes");
        header_name.write_text("Classes");
        break;
      }
      case RecordType::Union: {
        auto header_name = header_div.open_a();
        header_name.add_name("unions");
        header_name.add_href("#unions");
        header_name.write_text("Unions");
        break;
      }
    }
  }
  {
    auto items_list = section_div.open_ul();
    items_list.add_class("section-items");
    items_list.add_class("item-table");

    for (auto&& [name, sort_key, key] : records) {
      if (auto result = generate_record_reference(
              items_list, element.records.at(key), page_state);
          result.is_err()) {
        return sus::err(sus::move(result).unwrap_err());
      }
    }
  }

  return sus::ok();
}

enum GenerateFunctionType {
  GenerateFunctions,
  GenerateOperators,
};

sus::Result<void, MarkdownToHtmlError> generate_function_references(
    HtmlWriter::OpenDiv& namespace_div, const NamespaceElement& element,
    sus::Slice<SortedFunctionByName> functions, GenerateFunctionType type,
    ParseMarkdownPageState& page_state) {
  if (functions.is_empty()) return sus::ok();

  auto section_div = namespace_div.open_div();
  section_div.add_class("section");
  section_div.add_class("functions");

  {
    auto header_div = section_div.open_div();
    header_div.add_class("section-header");
    switch (type) {
      case GenerateFunctions: {
        auto header_name = header_div.open_a();
        header_name.add_name("functions");
        header_name.add_href("#functions");
        header_name.write_text("Functions");
        break;
      }
      case GenerateOperators: {
        auto header_name = header_div.open_a();
        header_name.add_name("operators");
        header_name.add_href("#operators");
        header_name.write_text("Operators");
        break;
      }
    }
  }
  {
    auto items_list = section_div.open_ul();
    items_list.add_class("section-items");
    items_list.add_class("item-table");

    for (auto&& [name, sort_key, function_id] : functions) {
      if (auto result = generate_function_reference(
              items_list, element.functions.at(function_id), page_state);
          result.is_err()) {
        return sus::err(sus::move(result).unwrap_err());
      }
    }
  }

  return sus::ok();
}

}  // namespace

sus::Result<void, MarkdownToHtmlError> generate_namespace(
    const Database& db, const NamespaceElement& element,
    sus::Vec<const NamespaceElement*> ancestors,
    const Options& options) noexcept {
  if (element.hidden()) return sus::ok();

  ParseMarkdownPageState page_state(db, options);

  MarkdownToHtml md_html;
  if (auto try_comment = element.get_comment(); try_comment.is_some()) {
    auto try_md_html = markdown_to_html(try_comment.as_value(), page_state);
    if (try_md_html.is_err())
      return sus::err(sus::move(try_md_html).unwrap_err());
    md_html = sus::move(try_md_html).unwrap();
  }

  const std::filesystem::path path =
      construct_html_file_path_for_namespace(options.output_root, element);
  std::filesystem::create_directories(path.parent_path());
  auto html = HtmlWriter(open_file_for_writing(path).unwrap());
  generate_head(html, namespace_display_name(element, ancestors, options),
                md_html.summary_text, options);

  sus::Vec<SortedNamespaceByName> sorted_namespaces;
  for (const auto& [key, sub_element] : element.namespaces) {
    if (sub_element.hidden()) continue;
    if (sub_element.is_empty()) continue;

    sorted_namespaces.push(
        sus::tuple(sub_element.name, sub_element.sort_key, key));
  }
  sorted_namespaces.sort_unstable_by(
      [](const SortedNamespaceByName& a, const SortedNamespaceByName& b) {
        auto ord = a.at<0>() <=> b.at<0>();
        if (ord != 0) return ord;
        return a.at<1>() <=> b.at<1>();
      });

  sus::Vec<SortedRecordByName> sorted_classes;
  sus::Vec<SortedRecordByName> sorted_unions;
  for (const auto& [key, sub_element] : element.records) {
    if (sub_element.hidden()) continue;

    switch (sub_element.record_type) {
      case RecordType::Class: [[fallthrough]];
      case RecordType::Struct:
        sorted_classes.push(
            sus::tuple(sub_element.name, sub_element.sort_key, key));
        break;
      case RecordType::Union:
        sorted_unions.push(
            sus::tuple(sub_element.name, sub_element.sort_key, key));
        break;
    }
  }
  sorted_classes.sort_unstable_by(
      [](const SortedRecordByName& a, const SortedRecordByName& b) {
        auto ord = a.at<0>() <=> b.at<0>();
        if (ord != 0) return ord;
        return a.at<1>() <=> b.at<1>();
      });
  sorted_unions.sort_unstable_by(
      [](const SortedRecordByName& a, const SortedRecordByName& b) {
        auto ord = a.at<0>() <=> b.at<0>();
        if (ord != 0) return ord;
        return a.at<1>() <=> b.at<1>();
      });

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

  sus::Vec<SortedConceptByName> sorted_concepts;
  for (const auto& [key, sub_element] : element.concepts) {
    if (sub_element.hidden()) continue;

    sorted_concepts.push(
        sus::tuple(sub_element.name, sub_element.sort_key, key));
  }
  sorted_concepts.sort_unstable_by(
      [](const SortedConceptByName& a, const SortedConceptByName& b) {
        auto ord = a.at<0>() <=> b.at<0>();
        if (ord != 0) return ord;
        return a.at<1>() <=> b.at<1>();
      });

  sus::Vec<SidebarLink> sidebar_links;
  if (!sorted_namespaces.is_empty())
    sidebar_links.push(SidebarLink("Namespaces", "#namespaces"));
  if (!sorted_classes.is_empty())
    sidebar_links.push(SidebarLink("Classes", "#classes"));
  if (!sorted_unions.is_empty())
    sidebar_links.push(SidebarLink("Unions", "#unions"));
  if (!sorted_functions.is_empty())
    sidebar_links.push(SidebarLink("Functions", "#functions"));
  if (!sorted_operators.is_empty())
    sidebar_links.push(SidebarLink("Operators", "#operators"));
  if (!sorted_concepts.is_empty())
    sidebar_links.push(SidebarLink("Concepts", "#concepts"));

  auto body = html.open_body();
  if (element.namespace_name == Namespace::Tag::Global)
    generate_sidebar(body, db, "", options.project_name, "TODO: version",
                     sus::move(sidebar_links), options);
  else
    generate_sidebar(body, db, "namespace", element.name, "",
                     sus::move(sidebar_links), options);

  auto main = body.open_main();
  auto namespace_div = main.open_div();
  namespace_div.add_class("namespace");
  generate_namespace_overview(namespace_div, element, ancestors, md_html,
                              options);

  if (!sorted_namespaces.is_empty()) {
    if (auto result = generate_namespace_references(
            namespace_div, element, sorted_namespaces.as_slice(), page_state);
        result.is_err()) {
      return sus::err(sus::move(result).unwrap_err());
    }
  }

  if (!sorted_classes.is_empty()) {
    if (auto result = generate_record_references(namespace_div, element,
                                                 sorted_classes.as_slice(),
                                                 RecordType::Class, page_state);
        result.is_err()) {
      return sus::err(sus::move(result).unwrap_err());
    }
  }
  if (!sorted_unions.is_empty()) {
    if (auto result = generate_record_references(namespace_div, element,
                                                 sorted_unions.as_slice(),
                                                 RecordType::Union, page_state);
        result.is_err()) {
      return sus::err(sus::move(result).unwrap_err());
    }
  }

  if (!sorted_functions.is_empty()) {
    if (auto result = generate_function_references(
            namespace_div, element, sorted_functions, GenerateFunctions,
            page_state);
        result.is_err()) {
      return sus::err(sus::move(result).unwrap_err());
    }
  }
  if (!sorted_operators.is_empty()) {
    if (auto result = generate_function_references(
            namespace_div, element, sorted_operators, GenerateOperators,
            page_state);
        result.is_err()) {
      return sus::err(sus::move(result).unwrap_err());
    }
  }

  if (!sorted_concepts.is_empty()) {
    if (auto result = generate_concept_references(
            namespace_div, element, sorted_concepts.as_slice(), page_state);
        result.is_err()) {
      return sus::err(sus::move(result).unwrap_err());
    }
  }

  // Recurse into namespaces, concepts, records and functions.
  ancestors.push(&element);
  for (const auto& [u, sub_element] : element.namespaces) {
    if (sub_element.hidden()) continue;
    if (auto result =
            generate_namespace(db, sub_element, sus::clone(ancestors), options);
        result.is_err()) {
      return sus::err(sus::move(result).unwrap_err());
    }
  }
  for (const auto& [u, sub_element] : element.concepts) {
    if (sub_element.hidden()) continue;
    if (auto result = generate_concept(db, sub_element, ancestors, options);
        result.is_err()) {
      return sus::err(sus::move(result).unwrap_err());
    }
  }
  for (const auto& [u, sub_element] : element.records) {
    if (sub_element.hidden()) continue;
    if (auto result =
            generate_record(db, sub_element, ancestors, sus::vec(), options);
        result.is_err()) {
      return sus::err(sus::move(result).unwrap_err());
    }
  }
  for (const auto& [u, sub_element] : element.functions) {
    if (sub_element.hidden()) continue;
    if (auto result = generate_function(db, sub_element, ancestors, options);
        result.is_err()) {
      return sus::err(sus::move(result).unwrap_err());
    }
  }

  return sus::ok();
}

sus::Result<void, MarkdownToHtmlError> generate_namespace_reference(
    HtmlWriter::OpenLi& open_li, const NamespaceElement& element,
    ParseMarkdownPageState& page_state) noexcept {
  {
    auto item_div = open_li.open_div();
    item_div.add_class("item-name");

    auto name_link = item_div.open_a();
    name_link.add_class("namespace-name");
    if (!element.hidden()) {
      name_link.add_href(construct_html_url_for_namespace(element));
    } else {
      llvm::errs() << "WARNING: Reference to hidden NamespaceElement "
                   << element.name << " in namespace "
                   << element.namespace_path;
    }
    name_link.write_text(element.name);
  }
  {
    auto desc_div = open_li.open_div();
    desc_div.add_class("description");
    desc_div.add_class("short");
    if (auto comment = element.get_comment(); comment.is_some()) {
      if (auto md_html = markdown_to_html(comment.as_value(), page_state);
          md_html.is_err()) {
        return sus::err(sus::move(md_html).unwrap_err());
      } else {
        desc_div.write_html(sus::move(md_html).unwrap().summary_html);
      }
    }
  }

  return sus::ok();
}

}  // namespace subdoc::gen
