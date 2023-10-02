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

#include "subdoc/lib/gen/generate_record.h"

#include <sstream>

#include "subdoc/lib/database.h"
#include "subdoc/lib/gen/files.h"
#include "subdoc/lib/gen/generate_cpp_path.h"
#include "subdoc/lib/gen/generate_function.h"
#include "subdoc/lib/gen/generate_head.h"
#include "subdoc/lib/gen/generate_nav.h"
#include "subdoc/lib/gen/generate_requires.h"
#include "subdoc/lib/gen/generate_type.h"
#include "subdoc/lib/gen/html_writer.h"
#include "subdoc/lib/gen/markdown_to_html.h"
#include "subdoc/lib/gen/options.h"
#include "subdoc/lib/parse_comment.h"
#include "sus/assertions/unreachable.h"
#include "sus/prelude.h"

namespace subdoc::gen {

namespace {

using SortedFunctionByName =
    sus::Tuple<std::string_view,
               /* primary sort_key (this should be ordered first) */ u32,
               /* sort_key */ u32, FunctionId>;
using SortedFieldByName =
    sus::Tuple<std::string_view, /* sort_key */ u32, UniqueSymbol>;

enum class MethodType {
  StaticMethods,
  NonStaticMethods,
  Conversions,
  NonStaticOperators,
};

const FunctionElement& function_element_from_sorted(
    const RecordElement& element, MethodType type,
    const SortedFunctionByName& s) noexcept {
  switch (type) {
    case MethodType::StaticMethods: {
      if (element.ctors.count(s.at<3>())) return element.ctors.at(s.at<3>());
      return element.methods.at(s.at<3>());
    }
    case MethodType::NonStaticMethods: return element.methods.at(s.at<3>());
    case MethodType::NonStaticOperators: return element.methods.at(s.at<3>());
    case MethodType::Conversions: return element.conversions.at(s.at<3>());
  }
  sus::unreachable();
}

/// Compares two `SortedFunctionByName` for ordering. It compares by ignoring
/// the `FunctionId` (which is not `Ord`).
constexpr inline std::weak_ordering cmp_functions_by_name(
    const SortedFunctionByName& a, const SortedFunctionByName& b) noexcept {
  // Primary sort key comes first.
  auto ord1 = a.at<1>() <=> b.at<1>();
  if (ord1 != 0) return ord1;
  // Then the name.
  auto ord0 = a.at<0>() <=> b.at<0>();
  if (ord0 != 0) return ord0;
  // Then the item sort key (overload set).
  return a.at<2>() <=> b.at<2>();
}

/// Compares two `SortedFunctionByName` for ordering. It compares by ignoring
/// the `UniqueSymbol` (which is not `Ord`).
constexpr inline std::weak_ordering cmp_fields_by_name(
    const SortedFieldByName& a, const SortedFieldByName& b) noexcept {
  // Name comes first.
  auto ord0 = a.at<0>() <=> b.at<0>();
  if (ord0 != 0) return ord0;
  // Then the item sort key.
  return a.at<1>() <=> b.at<1>();
}

void generate_record_overview(HtmlWriter::OpenDiv& record_div,
                              const RecordElement& element,
                              sus::Slice<const NamespaceElement*> namespaces,
                              sus::Slice<const RecordElement*> type_ancestors,
                              const MarkdownToHtml& comment_html,
                              const Options& options) noexcept {
  auto section_div = record_div.open_div();
  section_div.add_class("section");
  section_div.add_class("overview");

  {
    auto header_div = section_div.open_div();
    header_div.add_class("section-header");
    {
      auto record_type_span = header_div.open_span();
      record_type_span.write_text(
          friendly_record_type_name(element.record_type, true));
    }
    for (auto [i, e] : generate_cpp_path_for_type(element, namespaces,
                                                  type_ancestors, options)
                           .into_iter()
                           .enumerate()) {
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
              break;  // Concept can't be an ancestor of a record.
          }
          sus::unreachable();
        }());
        ancestor_anchor.add_href(e.link_href);
        ancestor_anchor.write_text(e.name);
      }
    }
  }
  {
    auto type_sig_div = section_div.open_div(HtmlWriter::SingleLine);
    type_sig_div.add_class("type-signature");
    if (!element.template_params.is_empty()) {
      auto template_pre = type_sig_div.open_pre();
      template_pre.add_class("template");
      template_pre.write_text("template <");
      for (const auto& [i, s] : element.template_params.iter().enumerate()) {
        if (i > 0u) template_pre.write_text(", ");
        template_pre.write_text(s);
      }
      template_pre.write_text(">");
    }
    {
      auto record_type_span = type_sig_div.open_span();
      std::string record_type_name =
          friendly_record_type_name(element.record_type, false);
      record_type_span.add_class(record_type_name);
      record_type_span.write_text(record_type_name);
    }
    {
      auto name_span = type_sig_div.open_span();
      name_span.add_class("type-name");
      name_span.write_text(element.name);
    }
    if (element.final) {
      auto final_span = type_sig_div.open_span();
      final_span.add_class("final");
      final_span.write_text("final");
    }
    if (element.constraints.is_some()) {
      generate_requires_constraints(type_sig_div,
                                    element.constraints.as_value());
    }
    {
      auto record_body_div = type_sig_div.open_div();
      record_body_div.add_class("record-body");
      record_body_div.write_text("{ ... };");
    }
  }
  {
    auto desc_div = section_div.open_div();
    desc_div.add_class("description");
    desc_div.add_class("long");
    desc_div.write_html(comment_html.full_html);
  }
}

sus::Result<void, MarkdownToHtmlError> generate_record_fields(
    HtmlWriter::OpenDiv& record_div, const RecordElement& element,
    bool static_fields, sus::Slice<SortedFieldByName> fields,
    ParseMarkdownPageState& page_state) {
  auto section_div = record_div.open_div();
  section_div.add_class("section");
  section_div.add_class("fields");
  section_div.add_class(static_fields ? "static" : "nonstatic");

  {
    auto fields_header_div = section_div.open_div();
    fields_header_div.add_class("section-header");
    if (static_fields) {
      auto header_name = fields_header_div.open_a();
      header_name.add_name("static-data-members");
      header_name.add_href("#static-data-members");
      header_name.write_text("Static Data Members");
    } else {
      auto header_name = fields_header_div.open_a();
      header_name.add_name("data-members");
      header_name.add_href("#data-members");
      header_name.write_text("Data Members");
    }
  }
  {
    auto items_ul = section_div.open_ul();
    items_ul.add_class("section-items");
    items_ul.add_class("item-table");

    for (auto&& [name, sort_key, field_unique_symbol] : fields) {
      const FieldElement& fe = element.fields.at(field_unique_symbol);
      if (auto result =
              generate_field_reference(items_ul, fe, static_fields, page_state);
          result.is_err()) {
        return sus::err(sus::move(result).unwrap_err());
      }
    }
  }

  return sus::ok();
}

sus::Result<void, MarkdownToHtmlError> generate_record_methods(
    HtmlWriter::OpenDiv& record_div, const RecordElement& element,
    MethodType type, sus::Slice<SortedFunctionByName> methods,
    ParseMarkdownPageState& page_state) {
  auto section_div = record_div.open_div();
  section_div.add_class("section");
  section_div.add_class("methods");
  switch (type) {
    case MethodType::Conversions: section_div.add_class("conversion"); break;
    case MethodType::StaticMethods: section_div.add_class("static"); break;
    case MethodType::NonStaticMethods:
      section_div.add_class("nonstatic");
      break;
    case MethodType::NonStaticOperators:
      section_div.add_class("nonstatic");
      break;
  }

  {
    auto methods_header_div = section_div.open_div();
    methods_header_div.add_class("section-header");
    switch (type) {
      case MethodType::StaticMethods: {
        auto header_name = methods_header_div.open_a();
        header_name.add_name("static-methods");
        header_name.add_href("#static-methods");
        header_name.write_text("Static Methods");
        break;
      }
      case MethodType::NonStaticMethods: {
        auto header_name = methods_header_div.open_a();
        header_name.add_name("methods");
        header_name.add_href("#methods");
        header_name.write_text("Methods");
        break;
      }
      case MethodType::Conversions: {
        auto header_name = methods_header_div.open_a();
        header_name.add_name("conversions");
        header_name.add_href("#conversions");
        header_name.write_text("Conversions");
        break;
      }
      case MethodType::NonStaticOperators: {
        auto header_name = methods_header_div.open_a();
        header_name.add_name("operators");
        header_name.add_href("#operators");
        header_name.write_text("Operators");
        break;
      }
    }
  }
  {
    auto items_div = section_div.open_div();
    items_div.add_class("section-items");

    for (const SortedFunctionByName& sorted_fn : methods) {
      const FunctionElement& func =
          function_element_from_sorted(element, type, sorted_fn);
      if (auto result = generate_function_method_reference(
              items_div, func,
              /*with constraints=*/true, page_state);
          result.is_err()) {
        return sus::err(sus::move(result).unwrap_err());
      }
    }
  }

  return sus::ok();
}

}  // namespace

sus::Result<void, MarkdownToHtmlError> generate_record(
    const Database& db, const RecordElement& element,
    sus::Slice<const NamespaceElement*> namespaces,
    sus::Vec<const RecordElement*> type_ancestors,
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

  const std::filesystem::path path = construct_html_file_path(
      options.output_root, element.namespace_path.as_slice(),
      element.record_path.as_slice(), element.name);
  std::filesystem::create_directories(path.parent_path());
  auto html = HtmlWriter(open_file_for_writing(path).unwrap());

  {
    std::ostringstream title;
    for (const Namespace& n : element.namespace_path.iter().rev()) {
      switch (n) {
        case Namespace::Tag::Global: break;
        case Namespace::Tag::Anonymous:
          title << "(anonymous)";
          title << "::";
          break;
        case Namespace::Tag::Named:
          title << n.as<Namespace::Tag::Named>();
          title << "::";
          break;
      }
    }
    for (std::string_view record_name : element.record_path.iter().rev()) {
      title << record_name;
      title << "::";
    }
    title << element.name;
    generate_head(html, sus::move(title).str(), md_html.summary_text, options);
  }

  sus::Vec<SortedFieldByName> sorted_static_fields;
  sus::Vec<SortedFieldByName> sorted_fields;
  for (const auto& [symbol, field_element] : element.fields) {
    if (field_element.hidden()) continue;

    switch (field_element.is_static) {
      case FieldElement::Static:
        sorted_static_fields.push(
            sus::tuple(field_element.name, field_element.sort_key, symbol));
        break;
      case FieldElement::NonStatic:
        sorted_fields.push(
            sus::tuple(field_element.name, field_element.sort_key, symbol));
        break;
    }
  }
  sorted_static_fields.sort_unstable_by(cmp_fields_by_name);
  sorted_fields.sort_unstable_by(cmp_fields_by_name);

  sus::Vec<SortedFunctionByName> sorted_static_methods;
  sus::Vec<SortedFunctionByName> sorted_methods;
  sus::Vec<SortedFunctionByName> sorted_conversions;
  sus::Vec<SortedFunctionByName> sorted_operators;
  for (const auto& [method_id, method_element] : element.ctors) {
    if (method_element.hidden()) continue;

    sorted_static_methods.push(sus::tuple(method_element.name, 0u,
                                          method_element.sort_key, method_id));
  }
  for (const auto& [method_id, method_element] : element.methods) {
    if (method_element.hidden()) continue;

    if (method_id.is_static) {
      sorted_static_methods.push(sus::tuple(method_element.name,
                                            1u,  // After ctors.
                                            method_element.sort_key,
                                            method_id));
    } else if (method_element.is_operator) {
      sorted_operators.push(sus::tuple(method_element.name, 0u,
                                       method_element.sort_key, method_id));
    } else {
      sorted_methods.push(sus::tuple(method_element.name, 0u,
                                     method_element.sort_key, method_id));
    }
  }
  for (const auto& [method_id, method_element] : element.conversions) {
    if (method_element.hidden()) continue;

    sorted_conversions.push(sus::tuple(method_element.name, 0u,
                                       method_element.sort_key, method_id));
  }
  sorted_static_methods.sort_unstable_by(cmp_functions_by_name);
  sorted_methods.sort_unstable_by(cmp_functions_by_name);
  sorted_conversions.sort_unstable_by(cmp_functions_by_name);
  sorted_operators.sort_unstable_by(cmp_functions_by_name);

  sus::Vec<SidebarLink> sidebar_links;
  if (!sorted_static_fields.is_empty()) {
    sidebar_links.push(SidebarLink(SidebarLinkStyle::GroupHeader,
                                   "Static Data Members",
                                   "#static-data-members"));
    for (auto&& [name, sort_key, field_unique_symbol] : sorted_static_fields) {
      const FieldElement& fe = element.fields.at(field_unique_symbol);
      sidebar_links.push(SidebarLink(SidebarLinkStyle::Item, std::string(name),
                                     construct_html_url_for_field(fe)));
    }
  }
  if (!sorted_static_methods.is_empty()) {
    sidebar_links.push(SidebarLink(SidebarLinkStyle::GroupHeader,
                                   "Static Methods", "#static-methods"));
    for (const SortedFunctionByName& sorted_fn : sorted_static_methods) {
      const FunctionElement& fe = function_element_from_sorted(
          element, MethodType::StaticMethods, sorted_fn);
      sidebar_links.push(SidebarLink(SidebarLinkStyle::Item,
                                     sus::clone(fe.name),
                                     construct_html_url_for_function(fe)));
    }
  }
  if (!sorted_methods.is_empty()) {
    sidebar_links.push(
        SidebarLink(SidebarLinkStyle::GroupHeader, "Methods", "#methods"));
    for (const SortedFunctionByName& sorted_fn : sorted_methods) {
      const FunctionElement& fe = function_element_from_sorted(
          element, MethodType::NonStaticMethods, sorted_fn);
      sidebar_links.push(SidebarLink(SidebarLinkStyle::Item,
                                     sus::clone(fe.name),
                                     construct_html_url_for_function(fe)));
    }
  }
  if (!sorted_conversions.is_empty()) {
    sidebar_links.push(SidebarLink(SidebarLinkStyle::GroupHeader, "Conversions",
                                   "#conversions"));
    for (const SortedFunctionByName& sorted_fn : sorted_conversions) {
      const FunctionElement& fe = function_element_from_sorted(
          element, MethodType::Conversions, sorted_fn);
      sidebar_links.push(SidebarLink(SidebarLinkStyle::Item,
                                     sus::clone(fe.name),
                                     construct_html_url_for_function(fe)));
    }
  }
  if (!sorted_operators.is_empty()) {
    sidebar_links.push(
        SidebarLink(SidebarLinkStyle::GroupHeader, "Operators", "#operators"));
    for (const SortedFunctionByName& sorted_fn : sorted_operators) {
      const FunctionElement& fe = function_element_from_sorted(
          element, MethodType::NonStaticOperators, sorted_fn);
      sidebar_links.push(SidebarLink(SidebarLinkStyle::Item,
                                     sus::clone(fe.name),
                                     construct_html_url_for_function(fe)));
    }
  }
  if (!sorted_fields.is_empty()) {
    sidebar_links.push(SidebarLink(SidebarLinkStyle::GroupHeader,
                                   "Data Members", "#data-members"));
    for (auto&& [name, sort_key, field_unique_symbol] : sorted_fields) {
      const FieldElement& fe = element.fields.at(field_unique_symbol);
      sidebar_links.push(SidebarLink(SidebarLinkStyle::Item, std::string(name),
                                     construct_html_url_for_field(fe)));
    }
  }

  auto body = html.open_body();
  generate_nav(body, db, friendly_record_type_name(element.record_type, false),
               element.name, "", sus::move(sidebar_links), options);

  auto main = body.open_main();
  auto record_div = main.open_div();
  record_div.add_class("type");
  record_div.add_class("record");
  record_div.add_class(friendly_record_type_name(element.record_type, false));
  generate_record_overview(record_div, element, namespaces, type_ancestors,
                           md_html, options);

  if (!sorted_static_fields.is_empty()) {
    if (auto result =
            generate_record_fields(record_div, element, true,
                                   sorted_static_fields.as_slice(), page_state);
        result.is_err()) {
      return sus::err(sus::move(result).unwrap_err());
    }
  }

  if (!sorted_static_methods.is_empty()) {
    if (auto result = generate_record_methods(
            record_div, element, MethodType::StaticMethods,
            sorted_static_methods.as_slice(), page_state);
        result.is_err()) {
      return sus::err(sus::move(result).unwrap_err());
    }
  }
  if (!sorted_methods.is_empty()) {
    if (auto result = generate_record_methods(
            record_div, element, MethodType::NonStaticMethods,
            sorted_methods.as_slice(), page_state);
        result.is_err()) {
      return sus::err(sus::move(result).unwrap_err());
    }
  }
  if (!sorted_conversions.is_empty()) {
    if (auto result = generate_record_methods(
            record_div, element, MethodType::Conversions,
            sorted_conversions.as_slice(), page_state);
        result.is_err()) {
      return sus::err(sus::move(result).unwrap_err());
    }
  }
  if (!sorted_operators.is_empty()) {
    if (auto result = generate_record_methods(
            record_div, element, MethodType::NonStaticOperators,
            sorted_operators.as_slice(), page_state);
        result.is_err()) {
      return sus::err(sus::move(result).unwrap_err());
    }
  }

  if (!sorted_fields.is_empty()) {
    if (auto result = generate_record_fields(
            record_div, element, false, sorted_fields.as_slice(), page_state);
        result.is_err()) {
      return sus::err(sus::move(result).unwrap_err());
    }
  }

  type_ancestors.push(&element);
  for (const auto& [key, subrecord] : element.records) {
    if (auto result = generate_record(db, subrecord, namespaces,
                                      sus::clone(type_ancestors), options);
        result.is_err()) {
      return sus::err(sus::move(result).unwrap_err());
    }
  }

  return sus::ok();
}

sus::Result<void, MarkdownToHtmlError> generate_record_reference(
    HtmlWriter::OpenUl& items_list, const RecordElement& element,
    ParseMarkdownPageState& page_state) noexcept {
  auto item_li = items_list.open_li();
  item_li.add_class("section-item");

  {
    auto item_div = item_li.open_div();
    item_div.add_class("item-name");

    auto type_sig_div = item_div.open_div(HtmlWriter::SingleLine);
    type_sig_div.add_class("type-signature");

    {
      auto name_link = type_sig_div.open_a();
      name_link.add_class("type-name");
      if (!element.hidden()) {
        name_link.add_href(construct_html_url_for_type(element));
      } else {
        llvm::errs() << "WARNING: Reference to hidden RecordElement "
                     << element.name << " in namespace "
                     << element.namespace_path;
      }
      name_link.write_text(element.name);
    }
  }
  {
    auto desc_div = item_li.open_div();
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

sus::Result<void, MarkdownToHtmlError> generate_field_reference(
    HtmlWriter::OpenUl& ul, const FieldElement& element, bool static_fields,
    ParseMarkdownPageState& page_state) noexcept {
  auto li = ul.open_li();
  li.add_class("section-item");

  {
    auto name_div = li.open_div(HtmlWriter::SingleLine);
    name_div.add_class("item-name");

    auto sig_div = name_div.open_div(HtmlWriter::SingleLine);
    sig_div.add_class("member-signature");

    {
      auto anchor = sig_div.open_a();
      anchor.add_name(construct_html_url_anchor_for_field(element));
    }
    if (!element.template_params.is_empty()) {
      auto template_div = sig_div.open_div(HtmlWriter::SingleLine);
      template_div.add_class("template");
      template_div.write_text("template <");
      for (const auto& [i, s] : element.template_params.iter().enumerate()) {
        if (i > 0u) template_div.write_text(", ");
        template_div.write_text(s);
      }
      template_div.write_text(">");
    }
    if (static_fields) {
      {
        auto static_span = sig_div.open_span(HtmlWriter::SingleLine);
        static_span.add_class("static");
        static_span.write_text("static");
      }
      sig_div.write_text(" ");
    }
    generate_type(
        sig_div, element.type,
        sus::some(sus::dyn<sus::fn::DynFnMut<void(HtmlWriter::OpenDiv&)>>(
            [&](HtmlWriter::OpenDiv& div) {
              auto anchor = div.open_a();
              anchor.add_href(construct_html_url_for_field(element));
              anchor.add_class("field-name");
              anchor.write_text(element.name);
            })));
  }
  {
    auto desc_div = li.open_div();
    desc_div.add_class("description");
    desc_div.add_class("short");
    if (auto comment = element.get_comment(); comment.is_some()) {
      if (auto md_html = markdown_to_html(comment.as_value(), page_state);
          md_html.is_err()) {
        return sus::err(sus::move(md_html).unwrap_err());
      } else {
        desc_div.write_html(sus::move(md_html).unwrap().full_html);
      }
    }
  }
  return sus::ok();
}

}  // namespace subdoc::gen
