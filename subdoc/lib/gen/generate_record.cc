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
#include "subdoc/lib/gen/generate_requires.h"
#include "subdoc/lib/gen/html_writer.h"
#include "subdoc/lib/gen/options.h"
#include "subdoc/lib/parse_comment.h"
#include "sus/assertions/unreachable.h"
#include "sus/prelude.h"

namespace subdoc::gen {

namespace {

using SortedFunctionByName = sus::Tuple<std::string_view, u32, u32, FunctionId>;
using SortedFieldByName = sus::Tuple<std::string_view, u32, UniqueSymbol>;

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
                              ParseMarkdownPageState& page_state,
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
            case CppPathFunction: break;  // Function can't be an ancesor of a record.
            case CppPathConcept: break;  // Concept can't be an ancestor of a record.
          }
          sus::unreachable();
        }());
        ancestor_anchor.add_href(e.link_href);
        ancestor_anchor.write_text(e.name);
      }
    }
  }
  {
    auto type_sig_div = section_div.open_div();
    type_sig_div.add_class("type-signature");
    if (!element.template_params.is_empty()) {
      auto template_div = type_sig_div.open_div(HtmlWriter::SingleLine);
      template_div.add_class("template");
      template_div.write_text("template <");
      for (const auto& [i, s] : element.template_params.iter().enumerate()) {
        if (i > 0u) template_div.write_text(", ");
        template_div.write_text(s);
      }
      template_div.write_text(">");
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
  if (element.has_comment()) {
    auto desc_div = section_div.open_div();
    desc_div.add_class("description");
    desc_div.add_class("long");
    desc_div.write_html(element.comment.parsed_full(page_state).unwrap());
  }
}

void generate_record_fields(HtmlWriter::OpenDiv& record_div,
                            const RecordElement& element, bool static_fields,
                            sus::Slice<SortedFieldByName> fields,
                            ParseMarkdownPageState& page_state) {
  if (fields.is_empty()) return;

  auto section_div = record_div.open_div();
  section_div.add_class("section");
  section_div.add_class("fields");
  section_div.add_class(static_fields ? "static" : "nonstatic");

  {
    auto fields_header_div = section_div.open_div();
    fields_header_div.add_class("section-header");
    fields_header_div.write_text(static_fields ? "Static Data Members"
                                               : "Data Members");
  }
  {
    auto items_div = section_div.open_div();
    items_div.add_class("section-items");

    for (auto&& [name, sort_key, field_unique_symbol] : fields) {
      const FieldElement& fe = element.fields.at(field_unique_symbol);

      auto field_div = items_div.open_div();
      field_div.add_class("section-item");

      {
        auto name_div = field_div.open_div();
        name_div.add_class("item-name");
        name_div.add_class("member-signature");

        if (!fe.template_params.is_empty()) {
          auto template_div = name_div.open_div(HtmlWriter::SingleLine);
          template_div.add_class("template");
          template_div.write_text("template <");
          for (const auto& [i, s] : fe.template_params.iter().enumerate()) {
            if (i > 0u) template_div.write_text(", ");
            template_div.write_text(s);
          }
          template_div.write_text(">");
        }
        if (static_fields) {
          auto static_span = name_div.open_span();
          static_span.add_class("static");
          static_span.write_text("static");
        }
        if (fe.is_const) {
          auto field_type_span = name_div.open_span();
          field_type_span.add_class("const");
          field_type_span.write_text("const");
        }
        if (fe.is_volatile) {
          auto field_type_span = name_div.open_span();
          field_type_span.add_class("volatile");
          field_type_span.write_text("volatile");
        }
        if (fe.type_element.is_some()) {
          auto field_type_link = name_div.open_a();
          field_type_link.add_class("type-name");
          field_type_link.add_title(fe.type_name);
          if (!fe.hidden()) {
            field_type_link.add_href(
                construct_html_file_path(
                    std::filesystem::path(),
                    fe.type_element->namespace_path.as_slice(),
                    fe.type_element->record_path.as_slice(),
                    fe.type_element->name)
                    .string());
          } else {
            llvm::errs() << "WARNING: Reference to hidden FieldElement "
                         << fe.name << " in record " << element.name
                         << " in namespace " << element.namespace_path;
          }
          field_type_link.write_text(fe.short_type_name);
        } else {
          name_div.write_text(fe.short_type_name);
        }
        {
          auto field_name_anchor = name_div.open_a();
          std::ostringstream anchor;
          anchor << "field.";
          anchor << fe.name;
          field_name_anchor.add_name(anchor.str());
          field_name_anchor.add_href(std::string("#") + anchor.str());
          field_name_anchor.add_class("field-name");
          field_name_anchor.write_text(fe.name);
        }
      }
      if (fe.has_comment()) {
        auto desc_div = field_div.open_div();
        desc_div.add_class("description");
        desc_div.add_class("long");
        desc_div.write_html(fe.comment.parsed_full(page_state).unwrap());
      }
    }
  }
}

enum MethodType {
  StaticMethods,
  NonStaticMethods,
  Conversions,
  NonStaticOperators,
};

void generate_record_methods(HtmlWriter::OpenDiv& record_div,
                             const RecordElement& element, MethodType type,
                             sus::Slice<SortedFunctionByName> methods,
                             ParseMarkdownPageState& page_state) {
  if (methods.is_empty()) return;

  auto section_div = record_div.open_div();
  section_div.add_class("section");
  section_div.add_class("methods");
  switch (type) {
    case Conversions: section_div.add_class("conversion"); break;
    case StaticMethods: section_div.add_class("static"); break;
    case NonStaticMethods: section_div.add_class("nonstatic"); break;
    case NonStaticOperators: section_div.add_class("nonstatic"); break;
  }

  {
    auto methods_header_div = section_div.open_div();
    methods_header_div.add_class("section-header");
    switch (type) {
      case StaticMethods:
        methods_header_div.write_text("Static Methods");
        break;
      case NonStaticMethods: methods_header_div.write_text("Methods"); break;
      case Conversions: methods_header_div.write_text("Conversions"); break;
      case NonStaticOperators:
        methods_header_div.write_text("Operators");
        break;
    }
  }
  {
    auto items_div = section_div.open_div();
    items_div.add_class("section-items");

    u32 overload_set;
    std::string_view prev_name;
    for (auto&& [name, sort_key1, sort_key2, function_id] : methods) {
      if (name == prev_name)
        overload_set += 1u;
      else
        overload_set = 0u;
      prev_name = name;
      const FunctionElement& func = [&]() -> decltype(auto) {
        switch (type) {
          case StaticMethods: {
            if (element.ctors.count(function_id))
              return element.ctors.at(function_id);
            return element.methods.at(function_id);
          }
          case NonStaticMethods: return element.methods.at(function_id);
          case NonStaticOperators: return element.methods.at(function_id);
          case Conversions: return element.conversions.at(function_id);
        }
        sus::unreachable();
      }();
      generate_function_long_reference(items_div, func, overload_set,
                                       /*with constraints=*/false, page_state);
    }
  }
}

}  // namespace

void generate_record(const RecordElement& element,
                     sus::Slice<const NamespaceElement*> namespaces,
                     sus::Vec<const RecordElement*> type_ancestors,
                     const Options& options) noexcept {
  if (element.hidden()) return;

  ParseMarkdownPageState page_state;

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
    generate_head(html, sus::move(title).str(), options);
  }

  auto body = html.open_body();

  auto record_div = body.open_div();
  record_div.add_class("type");
  record_div.add_class("record");
  record_div.add_class(friendly_record_type_name(element.record_type, false));
  generate_record_overview(record_div, element, namespaces, type_ancestors,
                           page_state, options);

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

  generate_record_fields(record_div, element, true,
                         sorted_static_fields.as_slice(), page_state);

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

  generate_record_methods(record_div, element, StaticMethods,
                          sorted_static_methods.as_slice(), page_state);
  generate_record_methods(record_div, element, NonStaticMethods,
                          sorted_methods.as_slice(), page_state);
  generate_record_methods(record_div, element, Conversions,
                          sorted_conversions.as_slice(), page_state);
  generate_record_methods(record_div, element, NonStaticOperators,
                          sorted_operators.as_slice(), page_state);

  generate_record_fields(record_div, element, false, sorted_fields.as_slice(),
                         page_state);

  type_ancestors.push(&element);
  for (const auto& [key, subrecord] : element.records) {
    generate_record(subrecord, namespaces, sus::clone(type_ancestors), options);
  }
}

void generate_record_reference(HtmlWriter::OpenUl& items_list,
                               const RecordElement& element,
                               ParseMarkdownPageState& page_state) noexcept {
  auto item_li = items_list.open_li();
  item_li.add_class("section-item");

  {
    auto item_div = item_li.open_div();
    item_div.add_class("item-name");

    auto type_sig_div = item_div.open_div();
    type_sig_div.add_class("type-signature");

    {
      auto name_link = type_sig_div.open_a();
      name_link.add_class("type-name");
      if (!element.hidden()) {
        name_link.add_href(construct_html_file_path(
                               std::filesystem::path(),
                               element.namespace_path.as_slice(),
                               element.record_path.as_slice(), element.name)
                               .string());
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
    if (element.has_comment())
      desc_div.write_html(element.comment.parsed_summary(page_state).unwrap());
  }
}

}  // namespace subdoc::gen
