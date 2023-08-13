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
#include "subdoc/lib/gen/html_writer.h"
#include "subdoc/lib/gen/options.h"
#include "sus/prelude.h"

namespace subdoc::gen {

namespace {

enum Style {
  StyleShort,
  StyleLong,
};

void generate_return_type(HtmlWriter::OpenDiv& div,
                          const FunctionOverload& overload,
                          bool is_static) noexcept {
  if (is_static) {
    auto static_span = div.open_span();
    static_span.add_class("static");
    static_span.write_text("static");
  }
  {
    auto return_type_link = div.open_a();
    return_type_link.add_class("type-name");
    return_type_link.add_title(overload.return_type_name);
    if (overload.return_type_element.is_some()) {
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
    }
    return_type_link.write_text(overload.return_short_type_name);
  }
}

void generate_function_params(HtmlWriter::OpenDiv& div,
                              const FunctionOverload& overload) {
  auto params_span = div.open_span(HtmlWriter::SingleLine);
  params_span.add_class("function-params");
  {
    auto open_paren = params_span.open_span(HtmlWriter::SingleLine);
    open_paren.add_class("paren");
    open_paren.add_class("open-paren");
    open_paren.write_text("(");
  }
  for (const auto& [i, p] : overload.parameters.iter().enumerate()) {
    if (i > 0u) params_span.write_text(", ");

    {
      auto one_param_link = params_span.open_a();
      one_param_link.add_class("type-name");
      one_param_link.add_title(p.type_name);
      if (p.type_element.is_some()) {
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
      }
      one_param_link.write_text(p.short_type_name);
    }

    {
      auto name_span = params_span.open_span(HtmlWriter::SingleLine);
      name_span.add_class("parameter-name");
      name_span.write_text(p.parameter_name);
    }

    if (p.default_value.is_some()) {
      {
        auto default_span = params_span.open_span(HtmlWriter::SingleLine);
        default_span.add_class("parameter-default-eq");
        default_span.write_text("=");
      }
      {
        auto default_span = params_span.open_span(HtmlWriter::SingleLine);
        default_span.add_class("parameter-default-value");
        default_span.write_text(p.default_value.as_value());
      }
    }
  }
  {
    auto close_paren = params_span.open_span(HtmlWriter::SingleLine);
    close_paren.add_class("paren");
    close_paren.add_class("close-paren");
    close_paren.write_text(")");
  }
  if (overload.method.is_some()) {
    if (overload.method->is_volatile) {
      auto volatile_span = div.open_span();
      volatile_span.add_class("volatile");
      volatile_span.write_text("volatile");
    }
    {
      switch (overload.method->qualifier) {
        case MethodQualifier::Const: {
          auto qualifier_span = div.open_span();
          qualifier_span.add_class("const");
          qualifier_span.write_text("const");
          break;
        }
        case MethodQualifier::ConstLValue: {
          auto qualifier_span = div.open_span();
          qualifier_span.add_class("const");
          qualifier_span.add_class("ref");
          qualifier_span.write_text("const&");
          break;
        }
        case MethodQualifier::ConstRValue: {
          auto qualifier_span = div.open_span();
          qualifier_span.add_class("const");
          qualifier_span.add_class("rref");
          qualifier_span.write_text("const&&");
          break;
        }
        case MethodQualifier::Mutable: {
          break;
        }
        case MethodQualifier::MutableLValue: {
          auto qualifier_span = div.open_span();
          qualifier_span.add_class("mutable");
          qualifier_span.add_class("ref");
          qualifier_span.write_text("&");
          break;
        }
        case MethodQualifier::MutableRValue: {
          auto qualifier_span = div.open_span();
          qualifier_span.add_class("mutable");
          qualifier_span.add_class("rref");
          qualifier_span.write_text("&&");
          break;
        }
      }
    }
  }
}

void generate_function_requires(HtmlWriter::OpenDiv& div,
                                const FunctionOverload& overload) {
  if (overload.constraints.is_some() &&
      !overload.constraints->list.is_empty()) {
    auto requires_div = div.open_div();
    requires_div.add_class("requires");
    {
      auto keyword_div = requires_div.open_span(HtmlWriter::SingleLine);
      keyword_div.add_class("requires-keyword");
      keyword_div.add_class("keyword");
      keyword_div.write_text("requires");
    }
    for (const RequiresConstraint& constraint : overload.constraints->list) {
      auto clause_div = requires_div.open_div(HtmlWriter::SingleLine);
      clause_div.add_class("requires-constaint");
      switch (constraint) {
        using enum RequiresConstraint::Tag;
        case Concept:
          clause_div.write_text(constraint.as<Concept>().concept_name);
          clause_div.write_text("<");
          for (const auto& [i, s] :
               constraint.as<Concept>().args.iter().enumerate()) {
            if (i > 0u) clause_div.write_text(", ");
            clause_div.write_text(s);
          }
          clause_div.write_text(">");
          break;
        case Text: clause_div.write_text(constraint.as<Text>()); break;
      }
    }
  }
}

void generate_overload_set(HtmlWriter::OpenDiv& div,
                           const FunctionElement& element, bool is_static,
                           u32 overload_set, Style style,
                           bool link_to_page) noexcept {
  for (const FunctionOverload& overload : element.overloads) {
    auto overload_div = div.open_div();
    overload_div.add_class("overload");

    {
      auto signature_div = overload_div.open_div();
      signature_div.add_class("function-signature");

      if (style == StyleLong) {
        generate_return_type(signature_div, overload, is_static);
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
      }
    }

    if (style == StyleLong) {
      generate_function_requires(overload_div, overload);
    }

    if (style == StyleShort) {
      break;  // Only show one overload/copy of the name in short style.
    }
  }
}

}  // namespace

void generate_function(const FunctionElement& element,
                       const sus::Slice<const NamespaceElement*>& namespaces,
                       u32 overload_set, const Options& options) noexcept {
  if (element.hidden()) return;

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
        auto signature_div = overload_div.open_div();
        signature_div.add_class("function-signature");

        // No need to display the `static` on a free function, so just consider
        // ourselves not.
        generate_return_type(signature_div, overload, /*is_static=*/false);
        {
          auto name_anchor = signature_div.open_a();
          name_anchor.add_href("#");
          name_anchor.add_class("function-name");
          name_anchor.write_text(element.name);
        }
        generate_function_params(signature_div, overload);
      }

      generate_function_requires(overload_div, overload);
    }
  }
  if (element.has_comment()) {
    auto desc_div = section_div.open_div();
    desc_div.add_class("description");
    desc_div.add_class("long");
    desc_div.write_html(element.comment.full());
  }
}

void generate_function_reference(HtmlWriter::OpenUl& items_list,
                                 const FunctionElement& element, bool is_static,
                                 u32 overload_set) noexcept {
  auto item_li = items_list.open_li();
  item_li.add_class("section-item");

  {
    auto overload_set_div = item_li.open_div();
    overload_set_div.add_class("overload-set");
    overload_set_div.add_class("item-name");

    // Operator overloads can all have different parameters and return types, so
    // we display them in long form.
    Style style = element.is_operator ? StyleLong : StyleShort;
    generate_overload_set(overload_set_div, element, is_static, overload_set,
                          style,
                          /*link_to_page=*/true);
  }
  {
    auto desc_div = item_li.open_div();
    desc_div.add_class("description");
    desc_div.add_class("short");
    if (element.has_comment()) desc_div.write_html(element.comment.summary());
  }
}

void generate_function_long_reference(HtmlWriter::OpenDiv& item_div,
                                      const FunctionElement& element,
                                      bool is_static,
                                      u32 overload_set) noexcept {
  {
    auto overload_set_div = item_div.open_div();
    overload_set_div.add_class("overload-set");
    overload_set_div.add_class("item-name");
    generate_overload_set(overload_set_div, element, is_static, overload_set,
                          StyleLong,
                          /*link_to_page=*/false);
  }
  {
    auto desc_div = item_div.open_div();
    desc_div.add_class("description");
    desc_div.add_class("long");
    if (element.has_comment()) desc_div.write_html(element.comment.full());
  }
}

}  // namespace subdoc::gen
