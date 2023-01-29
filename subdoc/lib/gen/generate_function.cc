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
#include "subdoc/lib/gen/html_writer.h"
#include "subdoc/lib/gen/options.h"
#include "subspace/prelude.h"

namespace subdoc::gen {

void generate_function(HtmlWriter::OpenDiv& section_div,
                       const FunctionElement& element,
                       bool is_static) noexcept {
  auto item_div = section_div.open_div();
  item_div.add_class("section-item");

  for (const FunctionOverload& overload : element.overloads) {
    auto overload_div = item_div.open_div();
    overload_div.add_class("overload");

    if (is_static) {
      auto static_span = overload_div.open_span();
      static_span.add_class("static");
      static_span.write_text("static");
    }
    {
      auto return_type_span = overload_div.open_span();
      return_type_span.add_class("type-name");
      return_type_span.add_title(element.return_type_name);
      return_type_span.write_text(element.return_short_type_name);
    }
    {
      auto name_anchor = overload_div.open_a(HtmlWriter::SingleLine);
      std::ostringstream anchor;
      if (overload.method.is_some())
        anchor << "method.";
      else
        anchor << "function.";
      anchor << (is_static ? "static." : "");
      anchor << element.name;
      name_anchor.add_name(anchor.str());
      name_anchor.add_href(std::string("#") + anchor.str());
      name_anchor.add_class("function-name");
      name_anchor.write_text(element.name);
    }
    {
      auto params_span = overload_div.open_span();
      params_span.add_class("function-params");
      // TODO: Write params.
      params_span.write_text("()");
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
  if (element.has_comment()) {
    auto desc_div = item_div.open_div();
    desc_div.add_class("description");
    desc_div.write_html(element.comment.raw_text);
  }
}

}  // namespace subdoc::gen