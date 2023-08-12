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
#include "subdoc/lib/gen/html_writer.h"
#include "subdoc/lib/gen/options.h"
#include "sus/prelude.h"

namespace subdoc::gen {

namespace {

void generate_overload_set(HtmlWriter::OpenDiv& div,
                           const FunctionElement& element, bool is_static,
                           u32 overload_set) noexcept {
  for (const FunctionOverload& overload : element.overloads) {
    auto overload_div = div.open_div();
    overload_div.add_class("overload");

    if (is_static) {
      auto static_span = overload_div.open_span();
      static_span.add_class("static");
      static_span.write_text("static");
    }
    {
      auto return_type_link = overload_div.open_a();
      return_type_link.add_class("type-name");
      return_type_link.add_title(overload.return_type_name);
      if (overload.return_type_element.is_some()) {
        return_type_link.add_href(
            construct_html_file_path(
                std::filesystem::path(),
                overload.return_type_element->namespace_path.as_slice(),
                overload.return_type_element->record_path.as_slice(),
                overload.return_type_element->name)
                .string());
      }
      return_type_link.write_text(overload.return_short_type_name);
    }
    {
      auto name_anchor = overload_div.open_a();
      std::ostringstream anchor;
      if (overload.method.is_some())
        anchor << "method.";
      else
        anchor << "function.";
      anchor << (is_static ? "static." : "");
      anchor << element.name;
      if (overload_set > 0u) anchor << "." << overload_set.primitive_value;
      name_anchor.add_name(anchor.str());
      name_anchor.add_href(std::string("#") + anchor.str());
      name_anchor.add_class("function-name");
      name_anchor.write_text(element.name);
    }
    {
      auto params_span = overload_div.open_span(HtmlWriter::SingleLine);
      params_span.add_class("function-params");
      params_span.write_text("(");
      bool write_comma = false;
      for (const FunctionParameter& p : overload.parameters) {
        if (write_comma) params_span.write_text(", ");
        auto one_param_link = params_span.open_a();
        one_param_link.add_class("type-name");
        one_param_link.add_title(p.type_name);
        if (p.type_element.is_some()) {
          one_param_link.add_href(construct_html_file_path(
                                      std::filesystem::path(),
                                      p.type_element->namespace_path.as_slice(),
                                      p.type_element->record_path.as_slice(),
                                      p.type_element->name)
                                      .string());
        }
        one_param_link.write_text(p.short_type_name);
        write_comma = true;
      }
      params_span.write_text(")");
    }
    if (overload.method.is_some()) {
      if (overload.method->is_volatile) {
        auto volatile_span = overload_div.open_span();
        volatile_span.add_class("volatile");
        volatile_span.write_text("volatile");
      }
      {
        switch (overload.method->qualifier) {
          case MethodQualifier::Const: {
            auto qualifier_span = overload_div.open_span();
            qualifier_span.add_class("const");
            qualifier_span.write_text("const");
            break;
          }
          case MethodQualifier::ConstLValue: {
            auto qualifier_span = overload_div.open_span();
            qualifier_span.add_class("const");
            qualifier_span.add_class("ref");
            qualifier_span.write_text("const&");
            break;
          }
          case MethodQualifier::ConstRValue: {
            auto qualifier_span = overload_div.open_span();
            qualifier_span.add_class("const");
            qualifier_span.add_class("rref");
            qualifier_span.write_text("const&&");
            break;
          }
          case MethodQualifier::Mutable: {
            break;
          }
          case MethodQualifier::MutableLValue: {
            auto qualifier_span = overload_div.open_span();
            qualifier_span.add_class("mutable");
            qualifier_span.add_class("ref");
            qualifier_span.write_text("&");
            break;
          }
          case MethodQualifier::MutableRValue: {
            auto qualifier_span = overload_div.open_span();
            qualifier_span.add_class("mutable");
            qualifier_span.add_class("rref");
            qualifier_span.write_text("&&");
            break;
          }
        }
      }
    }
  }
}

}  // namespace

void generate_function_reference(HtmlWriter::OpenUl& items_list,
                                 const FunctionElement& element, bool is_static,
                                 u32 overload_set) noexcept {
  auto item_li = items_list.open_li();
  item_li.add_class("section-item");

  {
    auto overload_set_div = item_li.open_div();
    overload_set_div.add_class("overload-set");
    overload_set_div.add_class("item-name");
    generate_overload_set(overload_set_div, element, is_static, overload_set);
  }
  if (element.has_comment()) {
    auto desc_div = item_li.open_div();
    desc_div.add_class("description");
    desc_div.add_class("short");
    desc_div.write_html(element.comment.summary());
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
    generate_overload_set(overload_set_div, element, is_static, overload_set);
  }
  if (element.has_comment()) {
    auto desc_div = item_div.open_div();
    desc_div.add_class("description");
    desc_div.add_class("long");
    desc_div.write_html(element.comment.full());
  }
}

}  // namespace subdoc::gen
