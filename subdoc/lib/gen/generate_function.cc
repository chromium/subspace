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

#include "subdoc/lib/gen/generate_function.h"

#include <sstream>

#include "subdoc/lib/database.h"
#include "subdoc/lib/gen/files.h"
#include "subdoc/lib/gen/generate_cpp_path.h"
#include "subdoc/lib/gen/generate_head.h"
#include "subdoc/lib/gen/generate_requires.h"
#include "subdoc/lib/gen/html_writer.h"
#include "subdoc/lib/gen/options.h"
#include "subdoc/lib/parse_comment.h"
#include "sus/prelude.h"

namespace subdoc::gen {

namespace {

enum Style {
  StyleShort,
  StyleLong,
};

void generate_return_type(HtmlWriter::OpenDiv& div,
                          const FunctionOverload& overload) noexcept {
  if (overload.return_type_element.is_some()) {
    auto return_type_link = div.open_a();
    return_type_link.add_class("type-name");
    return_type_link.add_title(overload.return_type_name);
    if (!overload.return_type_element->hidden()) {
      return_type_link.add_href(
          construct_html_file_path(
              std::filesystem::path(),
              overload.return_type_element->namespace_path.as_slice(),
              overload.return_type_element->record_path.as_slice(),
              overload.return_type_element->name)
              .string());
    } else {
      llvm::errs() << "WARNING: Reference to hidden TypeElement "
                   << overload.return_type_element->name << " in namespace "
                   << overload.return_type_element->namespace_path;
    }
    return_type_link.write_text(overload.return_short_type_name);
  } else {
    div.write_text(overload.return_short_type_name);
  }
}

void generate_function_params(HtmlWriter::OpenDiv& div,
                              const FunctionOverload& overload) {
  {
    div.write_text("(");
    for (const auto& [i, p] : overload.parameters.iter().enumerate()) {
      if (i > 0u) div.write_text(", ");

      if (p.type_element.is_some()) {
        auto one_param_link = div.open_a();
        one_param_link.add_class("type-name");
        one_param_link.add_title(p.type_name);
        if (!p.type_element->hidden()) {
          one_param_link.add_href(construct_html_file_path(
                                      std::filesystem::path(),
                                      p.type_element->namespace_path.as_slice(),
                                      p.type_element->record_path.as_slice(),
                                      p.type_element->name)
                                      .string());
        } else {
          llvm::errs() << "WARNING: Reference to hidden TypeElement "
                       << p.type_element->name << " in namespace "
                       << p.type_element->namespace_path;
        }
        one_param_link.write_text(p.short_type_name);
      } else {
        div.write_text(p.short_type_name);
      }

      if (!p.parameter_name.empty()) {
        div.write_text(" ");
        div.write_text(p.parameter_name);
      }

      if (p.default_value.is_some()) {
        div.write_text(" = ");
        div.write_text(p.default_value.as_value());
      }
    }
    div.write_text(")");
  }
  if (overload.method.is_some()) {
    if (overload.method->is_volatile) div.write_text(" volatile");
    {
      switch (overload.method->qualifier) {
        case MethodQualifier::Const: div.write_text(" const"); break;
        case MethodQualifier::ConstLValue: div.write_text(" const&"); break;
        case MethodQualifier::ConstRValue: div.write_text(" const&&"); break;
        case MethodQualifier::Mutable: break;
        case MethodQualifier::MutableLValue: div.write_text(" &"); break;
        case MethodQualifier::MutableRValue: div.write_text(" &&"); break;
      }
    }
  }
}

void generate_function_extras(HtmlWriter::OpenDiv& div,
                              const FunctionOverload& overload) noexcept {
  if (overload.is_deleted) {
    auto extra_div = div.open_div();
    extra_div.add_class("deleted");
    extra_div.write_text("deleted");
  }
  if (overload.method.is_some() && overload.method.as_value().is_virtual) {
    auto extra_div = div.open_div();
    extra_div.add_class("virtual");
    extra_div.write_text("virtual");
  }
}

void generate_overload_set(HtmlWriter::OpenDiv& div,
                           const FunctionElement& element, u32 overload_set,
                           Style style, bool link_to_page) noexcept {
  for (const FunctionOverload& overload : element.overloads) {
    auto overload_div = div.open_div();
    overload_div.add_class("overload");

    bool is_static =
        overload.method.as_ref()
            .map([](const auto& method) { return method.is_static; })
            .unwrap_or(false);
    const bool has_return = overload.method.as_ref()
                                .map([](const auto& method) {
                                  return !method.is_ctor && !method.is_dtor &&
                                         !method.is_conversion;
                                })
                                .unwrap_or(true);

    {
      auto signature_div = overload_div.open_div(HtmlWriter::SingleLine);
      signature_div.add_class("function-signature");

      if (style == StyleLong) {
        if (is_static) {
          auto static_span = signature_div.open_span(HtmlWriter::SingleLine);
          static_span.add_class("static");
          static_span.write_text("static");
        }
        if (has_return) {
          auto auto_span = signature_div.open_span(HtmlWriter::SingleLine);
          auto_span.add_class("function-auto");
          auto_span.write_text("auto");
        }
      }
      {
        auto name_anchor = signature_div.open_a();
        if (link_to_page) {
          if (!element.hidden()) {
            name_anchor.add_href(
                construct_html_file_path_for_function(std::filesystem::path(),
                                                      element, overload_set)
                    .string());
          } else {
            llvm::errs() << "WARNING: Reference to hidden FunctionElement "
                         << element.name << " in namespace "
                         << element.namespace_path;
          }
        } else {
          std::ostringstream anchor;
          if (overload.method.is_some())
            anchor << "method.";
          else
            anchor << "function.";
          anchor << (is_static ? "static." : "");
          anchor << element.name;
          if (overload_set > 0u) anchor << "." << overload_set;
          name_anchor.add_name(anchor.str());
          name_anchor.add_href(std::string("#") + anchor.str());
        }
        name_anchor.add_class("function-name");
        name_anchor.write_text(element.name);
      }
      if (style == StyleLong) {
        generate_function_params(signature_div, overload);
        if (has_return) {
          signature_div.write_text(" -> ");
          generate_return_type(signature_div, overload);
        }
      }
    }

    if (style == StyleLong) {
      if (overload.constraints.is_some()) {
        generate_requires_constraints(overload_div,
                                      overload.constraints.as_value());
      }
      generate_function_extras(overload_div, overload);
    }

    if (style == StyleShort) {
      break;  // Only show one overload/copy of the name in short style.
    }
  }
}

}  // namespace

void generate_function(const FunctionElement& element,
                       sus::Slice<const NamespaceElement*> namespaces,
                       u32 overload_set, const Options& options) noexcept {
  if (element.hidden()) return;

  ParseMarkdownPageState page_state;

  const std::filesystem::path path = construct_html_file_path_for_function(
      options.output_root, element, overload_set);
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
    title << element.name;
    generate_head(html, sus::move(title).str(), options);
  }

  auto body = html.open_body();

  auto function_div = body.open_div();
  function_div.add_class("function");

  auto section_div = function_div.open_div();
  section_div.add_class("section");
  section_div.add_class("overview");

  {
    auto header_div = section_div.open_div();
    header_div.add_class("section-header");
    {
      auto function_type_span = header_div.open_span();
      function_type_span.write_text("Function");
    }
    for (auto [i, e] :
         generate_cpp_path_for_function(element, namespaces, options)
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
            case CppPathFunction: return "function-name";
          }
          sus::unreachable();
        }());
        ancestor_anchor.add_href(e.link_href);
        ancestor_anchor.write_text(e.name);
      }
    }
  }
  {
    auto overload_set_div = section_div.open_div();
    overload_set_div.add_class("overload-set");
    for (const FunctionOverload& overload : element.overloads) {
      auto overload_div = overload_set_div.open_div();
      overload_div.add_class("overload");

      {
        auto signature_div = overload_div.open_div(HtmlWriter::SingleLine);
        signature_div.add_class("function-signature");

        {
          auto auto_span = signature_div.open_span(HtmlWriter::SingleLine);
          auto_span.add_class("function-auto");
          auto_span.write_text("auto");
        }
        {
          auto name_anchor = signature_div.open_a();
          name_anchor.add_href("#");
          name_anchor.add_class("function-name");
          name_anchor.write_text(element.name);
        }
        generate_function_params(signature_div, overload);
        {
          auto arrow_span = signature_div.open_span(HtmlWriter::SingleLine);
          arrow_span.add_class("return-arrow");
          arrow_span.write_text("->");
        }
        generate_return_type(signature_div, overload);
      }

      if (overload.constraints.is_some()) {
        generate_requires_constraints(overload_div,
                                      overload.constraints.as_value());
      }
      generate_function_extras(overload_div, overload);
    }
  }
  if (element.has_comment()) {
    auto desc_div = section_div.open_div();
    desc_div.add_class("description");
    desc_div.add_class("long");
    desc_div.write_html(element.comment.parsed_full(page_state).unwrap());
  }
}

void generate_function_reference(HtmlWriter::OpenUl& items_list,
                                 const FunctionElement& element,
                                 u32 overload_set,
                                 ParseMarkdownPageState& page_state) noexcept {
  auto item_li = items_list.open_li();
  item_li.add_class("section-item");

  {
    auto overload_set_div = item_li.open_div();
    overload_set_div.add_class("overload-set");
    overload_set_div.add_class("item-name");

    // Operator overloads can all have different parameters and return types, so
    // we display them in long form.
    Style style = element.is_operator ? StyleLong : StyleShort;
    generate_overload_set(overload_set_div, element, overload_set, style,
                          /*link_to_page=*/true);
  }
  {
    auto desc_div = item_li.open_div();
    desc_div.add_class("description");
    desc_div.add_class("short");
    if (element.has_comment())
      desc_div.write_html(element.comment.parsed_summary(page_state).unwrap());
  }
}

void generate_function_long_reference(
    HtmlWriter::OpenDiv& item_div, const FunctionElement& element,
    u32 overload_set, ParseMarkdownPageState& page_state) noexcept {
  {
    auto overload_set_div = item_div.open_div();
    overload_set_div.add_class("overload-set");
    overload_set_div.add_class("item-name");
    generate_overload_set(overload_set_div, element, overload_set, StyleLong,
                          /*link_to_page=*/false);
  }
  {
    auto desc_div = item_div.open_div();
    desc_div.add_class("description");
    desc_div.add_class("long");
    if (element.has_comment())
      desc_div.write_html(element.comment.parsed_full(page_state).unwrap());
  }
}

}  // namespace subdoc::gen
