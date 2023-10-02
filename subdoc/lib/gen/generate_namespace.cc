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
#include "subdoc/lib/gen/generate_alias.h"
#include "subdoc/lib/gen/generate_concept.h"
#include "subdoc/lib/gen/generate_cpp_path.h"
#include "subdoc/lib/gen/generate_function.h"
#include "subdoc/lib/gen/generate_head.h"
#include "subdoc/lib/gen/generate_nav.h"
#include "subdoc/lib/gen/generate_record.h"
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
using SortedAliasByName = sus::Tuple<std::string_view, u32, AliasId>;
using SortedVariableByName = sus::Tuple<std::string_view, u32, UniqueSymbol>;

const NamespaceElement& namespace_element_from_sorted(
    const NamespaceElement& element, const SortedNamespaceByName& s) noexcept {
  return element.namespaces.at(s.at<2>());
}

const ConceptElement& concept_element_from_sorted(
    const NamespaceElement& element, const SortedConceptByName& s) noexcept {
  return element.concepts.at(s.at<2>());
}

const FunctionElement& function_element_from_sorted(
    const NamespaceElement& element, const SortedFunctionByName& s) noexcept {
  return element.functions.at(s.at<2>());
}

const RecordElement& record_element_from_sorted(
    const NamespaceElement& element, const SortedRecordByName& s) noexcept {
  return element.records.at(s.at<2>());
}

const AliasElement& alias_element_from_sorted(
    const NamespaceElement& element, const SortedAliasByName& s) noexcept {
  return element.aliases.at(s.at<2>());
}

const FieldElement& field_element_from_sorted(
    const NamespaceElement& element, const SortedVariableByName& s) noexcept {
  return element.variables.at(s.at<2>());
}

constexpr inline std::weak_ordering cmp_namespaces_by_name(
    const SortedNamespaceByName& a, const SortedNamespaceByName& b) noexcept {
  auto ord = a.at<0>() <=> b.at<0>();
  if (ord != 0) return ord;
  return a.at<1>() <=> b.at<1>();
}

constexpr inline std::weak_ordering cmp_concepts_by_name(
    const SortedConceptByName& a, const SortedConceptByName& b) noexcept {
  auto ord = a.at<0>() <=> b.at<0>();
  if (ord != 0) return ord;
  return a.at<1>() <=> b.at<1>();
}

constexpr inline std::weak_ordering cmp_functions_by_name(
    const SortedFunctionByName& a, const SortedFunctionByName& b) noexcept {
  auto ord = a.at<0>() <=> b.at<0>();
  if (ord != 0) return ord;
  return a.at<1>() <=> b.at<1>();
}

constexpr inline std::weak_ordering cmp_records_by_name(
    const SortedRecordByName& a, const SortedRecordByName& b) noexcept {
  auto ord = a.at<0>() <=> b.at<0>();
  if (ord != 0) return ord;
  return a.at<1>() <=> b.at<1>();
}

constexpr inline std::weak_ordering cmp_aliases_by_name(
    const SortedAliasByName& a, const SortedAliasByName& b) noexcept {
  auto ord = a.at<0>() <=> b.at<0>();
  if (ord != 0) return ord;
  return a.at<1>() <=> b.at<1>();
}

constexpr inline std::weak_ordering cmp_variables_by_name(
    const SortedVariableByName& a, const SortedVariableByName& b) noexcept {
  auto ord = a.at<0>() <=> b.at<0>();
  if (ord != 0) return ord;
  return a.at<1>() <=> b.at<1>();
}

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

    for (const SortedNamespaceByName& sorted_ns : namespaces) {
      const NamespaceElement& ne =
          namespace_element_from_sorted(element, sorted_ns);
      {
        auto item_li = items_list.open_li();
        item_li.add_class("section-item");

        if (auto result = generate_namespace_reference(item_li, ne, page_state);
            result.is_err()) {
          return sus::err(sus::move(result).unwrap_err());
        }
      }

      {
        sus::Vec<SortedNamespaceByName> sorted;
        for (const auto& [key, sub_element] : ne.namespaces) {
          if (sub_element.hidden()) continue;
          if (sub_element.is_empty()) continue;

          sorted.push(sus::tuple(sub_element.name, sub_element.sort_key, key));
        }
        sorted.sort_unstable_by(cmp_namespaces_by_name);

        for (const SortedNamespaceByName& sub_sorted_ns : sorted) {
          const NamespaceElement& sub_ne =
              namespace_element_from_sorted(ne, sub_sorted_ns);
          auto item_li = items_list.open_li();
          item_li.add_class("nested");
          item_li.add_class("section-item");
          if (auto result =
                  generate_namespace_reference(item_li, sub_ne, page_state);
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

    for (const SortedConceptByName& sorted_concept : concepts) {
      const ConceptElement& ce =
          concept_element_from_sorted(element, sorted_concept);
      if (auto result = generate_concept_reference(items_list, ce, page_state);
          result.is_err()) {
        return sus::err(sus::move(result).unwrap_err());
      }
    }
  }

  return sus::ok();
}

enum class AliasesOf { Types, Concepts, Functions, Variables };

sus::Result<void, MarkdownToHtmlError> generate_alias_references(
    HtmlWriter::OpenDiv& namespace_div, AliasesOf aliases_of,
    const NamespaceElement& element, sus::Slice<SortedAliasByName> aliases,
    ParseMarkdownPageState& page_state) {
  if (aliases.is_empty()) return sus::ok();

  auto section_div = namespace_div.open_div();
  section_div.add_class("section");
  section_div.add_class("aliases");
  switch (aliases_of) {
    case AliasesOf::Types: section_div.add_class("types"); break;
    case AliasesOf::Concepts: section_div.add_class("concepts"); break;
    case AliasesOf::Functions: section_div.add_class("functions"); break;
    case AliasesOf::Variables: section_div.add_class("variables"); break;
  }

  {
    auto header_div = section_div.open_div();
    header_div.add_class("section-header");
    auto header_name = header_div.open_a();
    switch (aliases_of) {
      case AliasesOf::Types:
        header_name.add_name("aliases-types");
        header_name.add_href("#aliases-types");
        header_name.write_text("Type Aliases");
        break;
      case AliasesOf::Concepts:
        header_name.add_name("aliases-concepts");
        header_name.add_href("#aliases-concepts");
        header_name.write_text("Concept Aliases");
        break;
      case AliasesOf::Functions:
        header_name.add_name("aliases-functions");
        header_name.add_href("#aliases-functions");
        header_name.write_text("Function Aliases");
        break;
      case AliasesOf::Variables:
        header_name.add_name("aliases-variables");
        header_name.add_href("#aliases-variables");
        header_name.write_text("Variable Aliases");
        break;
    }
  }
  {
    auto items_list = section_div.open_ul();
    items_list.add_class("section-items");
    items_list.add_class("item-table");

    for (const SortedAliasByName& sorted_alias : aliases) {
      const AliasElement& ae = alias_element_from_sorted(element, sorted_alias);
      if (auto result = generate_alias_reference(items_list, ae, page_state);
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

    for (const SortedRecordByName& sorted_rec : records) {
      const RecordElement& re = record_element_from_sorted(element, sorted_rec);
      if (auto result = generate_record_reference(items_list, re, page_state);
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

    for (const SortedFunctionByName& sorted_fn : functions) {
      const FunctionElement& fe =
          function_element_from_sorted(element, sorted_fn);
      if (auto result = generate_function_reference(items_list, fe, page_state);
          result.is_err()) {
        return sus::err(sus::move(result).unwrap_err());
      }
    }
  }

  return sus::ok();
}

sus::Result<void, MarkdownToHtmlError> generate_variable_references(
    HtmlWriter::OpenDiv& namespace_div, const NamespaceElement& element,
    sus::Slice<SortedVariableByName> variables,
    ParseMarkdownPageState& page_state) {
  if (variables.is_empty()) return sus::ok();

  auto section_div = namespace_div.open_div();
  section_div.add_class("section");
  section_div.add_class("variables");

  {
    auto header_div = section_div.open_div();
    header_div.add_class("section-header");

    auto header_name = header_div.open_a();
    header_name.add_name("variables");
    header_name.add_href("#variables");
    header_name.write_text("Variables");
  }
  {
    auto items_list = section_div.open_ul();
    items_list.add_class("section-items");
    items_list.add_class("item-table");

    for (const SortedVariableByName& sorted_var : variables) {
      const FieldElement& fe = field_element_from_sorted(element, sorted_var);
      if (auto result = generate_field_reference(
              items_list, fe, /*static_fields=*/false, page_state);
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
  sorted_namespaces.sort_unstable_by(cmp_namespaces_by_name);

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
  sorted_classes.sort_unstable_by(cmp_records_by_name);
  sorted_unions.sort_unstable_by(cmp_records_by_name);

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
  sorted_functions.sort_unstable_by(cmp_functions_by_name);
  sorted_operators.sort_unstable_by(cmp_functions_by_name);

  sus::Vec<SortedVariableByName> sorted_variables;
  for (const auto& [function_id, sub_element] : element.variables) {
    if (sub_element.hidden()) continue;

    sorted_variables.push(
        sus::tuple(sub_element.name, sub_element.sort_key, function_id));
  }
  sorted_variables.sort_unstable_by(cmp_variables_by_name);

  sus::Vec<SortedConceptByName> sorted_concepts;
  for (const auto& [key, sub_element] : element.concepts) {
    if (sub_element.hidden()) continue;

    sorted_concepts.push(
        sus::tuple(sub_element.name, sub_element.sort_key, key));
  }
  sorted_concepts.sort_unstable_by(cmp_concepts_by_name);

  sus::Vec<SortedAliasByName> sorted_aliases_of_types;
  sus::Vec<SortedAliasByName> sorted_aliases_of_functions;
  sus::Vec<SortedAliasByName> sorted_aliases_of_variables;
  sus::Vec<SortedAliasByName> sorted_aliases_of_concepts;
  // TODO: Methods, enum values, variables.
  for (const auto& [key, sub_element] : element.aliases) {
    if (sub_element.hidden()) continue;

    switch (sub_element.target) {
      case AliasTarget::Tag::AliasOfType:
        sorted_aliases_of_types.push(
            sus::tuple(sub_element.name, sub_element.sort_key, key));
        break;
      case AliasTarget::Tag::AliasOfConcept:
        sorted_aliases_of_concepts.push(
            sus::tuple(sub_element.name, sub_element.sort_key, key));
        break;
      case AliasTarget::Tag::AliasOfFunction:
        sorted_aliases_of_functions.push(
            sus::tuple(sub_element.name, sub_element.sort_key, key));
        break;
      case AliasTarget::Tag::AliasOfMethod: break;
      case AliasTarget::Tag::AliasOfEnumConstant: break;
      case AliasTarget::Tag::AliasOfVariable:
        sorted_aliases_of_variables.push(
            sus::tuple(sub_element.name, sub_element.sort_key, key));
        break;
    }
  }
  sorted_aliases_of_types.sort_unstable_by(cmp_aliases_by_name);
  sorted_aliases_of_functions.sort_unstable_by(cmp_aliases_by_name);
  sorted_aliases_of_variables.sort_unstable_by(cmp_aliases_by_name);
  sorted_aliases_of_concepts.sort_unstable_by(cmp_aliases_by_name);

  sus::Vec<SidebarLink> sidebar_links;
  if (!sorted_namespaces.is_empty()) {
    sidebar_links.push(SidebarLink(SidebarLinkStyle::GroupHeader, "Namespaces",
                                   "#namespaces"));
    for (const SortedNamespaceByName& sorted_ns : sorted_namespaces) {
      const NamespaceElement& ne =
          namespace_element_from_sorted(element, sorted_ns);
      sidebar_links.push(SidebarLink(SidebarLinkStyle::Item, ne.name,
                                     construct_html_url_for_namespace(ne)));
    }
  }
  if (!sorted_classes.is_empty()) {
    sidebar_links.push(
        SidebarLink(SidebarLinkStyle::GroupHeader, "Classes", "#classes"));
    for (const SortedRecordByName& sorted_rec : sorted_classes) {
      const RecordElement& re = record_element_from_sorted(element, sorted_rec);
      sidebar_links.push(SidebarLink(SidebarLinkStyle::Item, re.name,
                                     construct_html_url_for_type(re)));
    }
  }
  if (!sorted_unions.is_empty()) {
    sidebar_links.push(
        SidebarLink(SidebarLinkStyle::GroupHeader, "Unions", "#unions"));
    for (const SortedRecordByName& sorted_rec : sorted_unions) {
      const RecordElement& re = record_element_from_sorted(element, sorted_rec);
      sidebar_links.push(SidebarLink(SidebarLinkStyle::Item, re.name,
                                     construct_html_url_for_type(re)));
    }
  }
  if (!sorted_functions.is_empty()) {
    sidebar_links.push(
        SidebarLink(SidebarLinkStyle::GroupHeader, "Functions", "#functions"));
    for (const SortedFunctionByName& sorted_fn : sorted_functions) {
      const FunctionElement& fe =
          function_element_from_sorted(element, sorted_fn);
      sidebar_links.push(SidebarLink(SidebarLinkStyle::Item, fe.name,
                                     construct_html_url_for_function(fe)));
    }
  }
  if (!sorted_operators.is_empty()) {
    sidebar_links.push(
        SidebarLink(SidebarLinkStyle::GroupHeader, "Operators", "#operators"));
    for (const SortedFunctionByName& sorted_fn : sorted_operators) {
      const FunctionElement& fe =
          function_element_from_sorted(element, sorted_fn);
      sidebar_links.push(SidebarLink(SidebarLinkStyle::Item, fe.name,
                                     construct_html_url_for_function(fe)));
    }
  }
  if (!sorted_variables.is_empty()) {
    sidebar_links.push(
        SidebarLink(SidebarLinkStyle::GroupHeader, "Variables", "#variables"));
    for (const SortedVariableByName& sorted_var : sorted_variables) {
      const FieldElement& fe = field_element_from_sorted(element, sorted_var);
      sidebar_links.push(SidebarLink(SidebarLinkStyle::Item, fe.name,
                                     construct_html_url_for_field(fe)));
    }
  }
  if (!sorted_concepts.is_empty()) {
    sidebar_links.push(
        SidebarLink(SidebarLinkStyle::GroupHeader, "Concepts", "#concepts"));
    for (const SortedConceptByName& sorted_concept : sorted_concepts) {
      const ConceptElement& ce =
          concept_element_from_sorted(element, sorted_concept);
      sidebar_links.push(SidebarLink(SidebarLinkStyle::Item, ce.name,
                                     construct_html_url_for_concept(ce)));
    }
  }
  if (!sorted_aliases_of_types.is_empty()) {
    sidebar_links.push(SidebarLink(SidebarLinkStyle::GroupHeader,
                                   "Type Aliases", "#aliases-types"));
    for (const SortedAliasByName& sorted_alias : sorted_aliases_of_types) {
      const AliasElement& ae = alias_element_from_sorted(element, sorted_alias);
      if (sus::Option<std::string> url = construct_html_url_for_alias(ae);
          url.is_some()) {
        // Link to where the alias links to.
        sidebar_links.push(SidebarLink(SidebarLinkStyle::Item, ae.name,
                                       sus::move(url).unwrap()));
      } else {
        // Link to the alias in this page since the alias itself doesn't connect
        // to anything in the database.
        std::ostringstream doc_url;
        doc_url << "#";
        doc_url << construct_html_url_anchor_for_alias(ae);
        sidebar_links.push(SidebarLink(SidebarLinkStyle::Item, ae.name,
                                       sus::move(doc_url).str()));
      }
    }
  }
  if (!sorted_aliases_of_functions.is_empty()) {
    sidebar_links.push(SidebarLink(SidebarLinkStyle::GroupHeader,
                                   "Function Aliases", "#aliases-functions"));
    for (const SortedAliasByName& sorted_alias : sorted_aliases_of_functions) {
      const AliasElement& ae = alias_element_from_sorted(element, sorted_alias);
      if (sus::Option<std::string> url = construct_html_url_for_alias(ae);
          url.is_some()) {
        // Link to where the alias links to.
        sidebar_links.push(SidebarLink(SidebarLinkStyle::Item, ae.name,
                                       sus::move(url).unwrap()));
      } else {
        // Link to the alias in this page since the alias itself doesn't connect
        // to anything in the database.
        std::ostringstream doc_url;
        doc_url << "#";
        doc_url << construct_html_url_anchor_for_alias(ae);
        sidebar_links.push(SidebarLink(SidebarLinkStyle::Item, ae.name,
                                       sus::move(doc_url).str()));
      }
    }
  }
  if (!sorted_aliases_of_variables.is_empty()) {
    sidebar_links.push(SidebarLink(SidebarLinkStyle::GroupHeader,
                                   "Variable Aliases", "#aliases-variables"));
    for (const SortedAliasByName& sorted_alias : sorted_aliases_of_variables) {
      const AliasElement& ae = alias_element_from_sorted(element, sorted_alias);
      if (sus::Option<std::string> url = construct_html_url_for_alias(ae);
          url.is_some()) {
        // Link to where the alias links to.
        sidebar_links.push(SidebarLink(SidebarLinkStyle::Item, ae.name,
                                       sus::move(url).unwrap()));
      } else {
        // Link to the alias in this page since the alias itself doesn't connect
        // to anything in the database.
        std::ostringstream doc_url;
        doc_url << "#";
        doc_url << construct_html_url_anchor_for_alias(ae);
        sidebar_links.push(SidebarLink(SidebarLinkStyle::Item, ae.name,
                                       sus::move(doc_url).str()));
      }
    }
  }
  if (!sorted_aliases_of_concepts.is_empty()) {
    sidebar_links.push(SidebarLink(SidebarLinkStyle::GroupHeader,
                                   "Concept Aliases", "#aliases-concepts"));
    for (const SortedAliasByName& sorted_alias : sorted_aliases_of_concepts) {
      const AliasElement& ae = alias_element_from_sorted(element, sorted_alias);
      if (sus::Option<std::string> url = construct_html_url_for_alias(ae);
          url.is_some()) {
        // Link to where the alias links to.
        sidebar_links.push(SidebarLink(SidebarLinkStyle::Item, ae.name,
                                       sus::move(url).unwrap()));
      } else {
        // Link to the alias in this page since the alias itself doesn't connect
        // to anything in the database.
        std::ostringstream doc_url;
        doc_url << "#";
        doc_url << construct_html_url_anchor_for_alias(ae);
        sidebar_links.push(SidebarLink(SidebarLinkStyle::Item, ae.name,
                                       sus::move(doc_url).str()));
      }
    }
  }

  auto body = html.open_body();
  if (element.namespace_name == Namespace::Tag::Global)
    generate_nav(body, db, "", options.project_name, "TODO: version",
                 sus::move(sidebar_links), options);
  else
    generate_nav(body, db, "namespace", element.name, "",
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

  if (!sorted_variables.is_empty()) {
    if (auto result = generate_variable_references(
            namespace_div, element, sorted_variables, page_state);
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

  if (!sorted_aliases_of_types.is_empty()) {
    if (auto result = generate_alias_references(
            namespace_div, AliasesOf::Types, element,
            sorted_aliases_of_types.as_slice(), page_state);
        result.is_err()) {
      return sus::err(sus::move(result).unwrap_err());
    }
  }

  if (!sorted_aliases_of_functions.is_empty()) {
    if (auto result = generate_alias_references(
            namespace_div, AliasesOf::Functions, element,
            sorted_aliases_of_functions.as_slice(), page_state);
        result.is_err()) {
      return sus::err(sus::move(result).unwrap_err());
    }
  }

  if (!sorted_aliases_of_variables.is_empty()) {
    if (auto result = generate_alias_references(
            namespace_div, AliasesOf::Variables, element,
            sorted_aliases_of_variables.as_slice(), page_state);
        result.is_err()) {
      return sus::err(sus::move(result).unwrap_err());
    }
  }

  if (!sorted_aliases_of_concepts.is_empty()) {
    if (auto result = generate_alias_references(
            namespace_div, AliasesOf::Concepts, element,
            sorted_aliases_of_concepts.as_slice(), page_state);
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
            generate_record(db, sub_element, ancestors, sus::empty, options);
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
