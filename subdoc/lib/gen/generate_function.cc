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
#include "subdoc/lib/gen/generate_nav.h"
#include "subdoc/lib/gen/generate_requires.h"
#include "subdoc/lib/gen/generate_type.h"
#include "subdoc/lib/gen/html_writer.h"
#include "subdoc/lib/gen/markdown_to_html.h"
#include "subdoc/lib/gen/options.h"
#include "subdoc/lib/parse_comment.h"
#include "sus/prelude.h"

namespace subdoc::gen {

namespace {

enum Style {
  StyleShort,
  StyleLong,
  StyleLongWithConstraints,
};

void generate_function_params(HtmlWriter::OpenDiv& div,
                              const FunctionOverload& overload) {
  {
    div.write_text("(");
    for (const auto& [i, p] : overload.parameters.iter().enumerate()) {
      if (i > 0u) div.write_text(", ");

      if (p.parameter_name.empty()) {
        generate_type(div, p.type, sus::none());
      } else {
        generate_type(
            div, p.type,
            sus::some(sus::dyn<sus::fn::DynFnMut<void(HtmlWriter::OpenDiv&)>>(
                [&](HtmlWriter::OpenDiv& div) {
                  div.write_text(p.parameter_name);
                })));
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
                           const FunctionElement& element, Style style,
                           bool link_to_page) noexcept {
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
      if (!link_to_page) {
        // Only methods are not given their own page, and are just a named
        // anchor on the Record's page.
        sus::check(overload.method.is_some());
        auto name_anchor = signature_div.open_a();
        name_anchor.add_name(construct_html_url_anchor_for_method(element));
      }
      if (style == StyleLong || style == StyleLongWithConstraints) {
        if (!overload.template_params.is_empty()) {
          auto template_div = signature_div.open_div(HtmlWriter::SingleLine);
          template_div.add_class("template");
          template_div.write_text("template <");
          for (const auto& [i, s] :
               overload.template_params.iter().enumerate()) {
            if (i > 0u) template_div.write_text(", ");
            template_div.write_text(s);
          }
          template_div.write_text(">");
        }
        if (is_static) {
          {
            auto static_span = signature_div.open_span(HtmlWriter::SingleLine);
            static_span.add_class("static");
            static_span.write_text("static");
          }
          signature_div.write_text(" ");
        }
        if (has_return) {
          {
            auto auto_span = signature_div.open_span(HtmlWriter::SingleLine);
            auto_span.add_class("function-auto");
            auto_span.write_text("auto");
          }
          signature_div.write_text(" ");
        }
      }
      {
        auto link_anchor = signature_div.open_a();
        if (link_to_page) {
          if (!element.hidden()) {
            link_anchor.add_href(construct_html_url_for_function(element));
          } else {
            llvm::errs() << "WARNING: Reference to hidden FunctionElement "
                         << element.name << " in namespace "
                         << element.namespace_path;
          }
        } else {
          // Only methods are not given their own page, and are just a named
          // anchor on the Record's page.
          sus::check(overload.method.is_some());
          link_anchor.add_href(construct_html_url_for_function(element));
        }
        link_anchor.add_class("function-name");
        link_anchor.write_text(element.name);
      }
      if (style == StyleLong || style == StyleLongWithConstraints) {
        generate_function_params(signature_div, overload);
        if (has_return) {
          signature_div.write_text(" -> ");
          generate_type(signature_div, overload.return_type,
                        sus::none() /* no variable name */);
        }
      }

      if (style == StyleLongWithConstraints) {
        if (overload.constraints.is_some()) {
          generate_requires_constraints(signature_div,
                                        overload.constraints.as_value());
        }
        generate_function_extras(signature_div, overload);
      }
    }

    if (style == StyleShort) {
      break;  // Only show one overload/copy of the name in short style.
    }
  }
}

}  // namespace

sus::Result<void, MarkdownToHtmlError> generate_function(
    const Database& db, const FunctionElement& element,
    sus::Slice<const NamespaceElement*> namespaces,
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
      construct_html_file_path_for_function(options.output_root, element);
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
    generate_head(html, sus::move(title).str(), md_html.summary_text, options);
  }

  auto body = html.open_body();
  generate_nav(body, db, "function", element.name, "",
               // TODO: links to what?
               sus::vec(), options);

  auto main = body.open_main();
  auto function_div = main.open_div();
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
            case CppPathConcept:
              break;  // Concept can't be an ancestor of a function.
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
        if (!overload.template_params.is_empty()) {
          auto template_div = signature_div.open_div(HtmlWriter::SingleLine);
          template_div.add_class("template");
          template_div.write_text("template <");
          for (const auto& [i, s] :
               overload.template_params.iter().enumerate()) {
            if (i > 0u) template_div.write_text(", ");
            template_div.write_text(s);
          }
          template_div.write_text(">");
        }
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
        // This is generating a function that is not a method, so there's always
        // some return type (ie. it can't be a special method like a ctor/dtor).
        signature_div.write_text(" -> ");
        generate_type(signature_div, overload.return_type,
                      sus::none() /* no variable name */);

        if (overload.constraints.is_some()) {
          generate_requires_constraints(signature_div,
                                        overload.constraints.as_value());
        }
        generate_function_extras(signature_div, overload);
      }
    }
  }
  {
    auto desc_div = section_div.open_div();
    desc_div.add_class("description");
    desc_div.add_class("long");
    desc_div.write_html(md_html.full_html);
  }

  return sus::ok();
}

sus::Result<void, MarkdownToHtmlError> generate_function_reference(
    HtmlWriter::OpenUl& items_list, const FunctionElement& element,
    ParseMarkdownPageState& page_state) noexcept {
  auto item_li = items_list.open_li();
  item_li.add_class("section-item");

  {
    auto overload_set_div = item_li.open_div();
    overload_set_div.add_class("overload-set");
    overload_set_div.add_class("item-name");

    // Operator overloads can all have different parameters and return types, so
    // we display them in long form.
    generate_overload_set(overload_set_div, element, StyleShort,
                          /*link_to_page=*/true);
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

sus::Result<void, MarkdownToHtmlError> generate_function_method_reference(
    HtmlWriter::OpenDiv& item_div, const FunctionElement& element,
    bool with_constraints, ParseMarkdownPageState& page_state) noexcept {
  {
    auto overload_set_div = item_div.open_div();
    overload_set_div.add_class("overload-set");
    overload_set_div.add_class("item-name");
    generate_overload_set(
        overload_set_div, element,
        with_constraints ? StyleLongWithConstraints : StyleLong,
        /*link_to_page=*/false);
  }
  {
    auto desc_div = item_div.open_div();
    desc_div.add_class("description");
    desc_div.add_class("long");
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
