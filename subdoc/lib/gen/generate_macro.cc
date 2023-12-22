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

#include "subdoc/lib/gen/generate_macro.h"

#include <sstream>

#include "subdoc/lib/database.h"
#include "subdoc/lib/gen/files.h"
#include "subdoc/lib/gen/generate_cpp_path.h"
#include "subdoc/lib/gen/generate_head.h"
#include "subdoc/lib/gen/generate_nav.h"
#include "subdoc/lib/gen/generate_search.h"
#include "subdoc/lib/gen/generate_source_link.h"
#include "subdoc/lib/gen/html_writer.h"
#include "subdoc/lib/gen/markdown_to_html.h"
#include "subdoc/lib/gen/options.h"
#include "sus/prelude.h"

namespace subdoc::gen {

sus::Result<void, MarkdownToHtmlError> generate_macro(
    const Database& db, const MacroElement& element,
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
      construct_html_file_path_for_macro(options.output_root, element);
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
  generate_nav(body, db, "macro", element.name, "",
               // TODO: links to what? same question as for functions.
               sus::empty, options);

  auto main = body.open_main();
  auto function_div = main.open_div();
  function_div.add_class("macro");

  auto section_div = function_div.open_div();
  section_div.add_class("section");
  section_div.add_class("overview");

  generate_search_title(
      section_div, generate_cpp_path_for_macro(element, namespaces, options));
  {
    auto header_div = section_div.open_div();
    header_div.add_class("section-header");
    {
      auto function_type_span = header_div.open_span();
      function_type_span.write_text("Macro");
    }
    for (auto [i, e] : generate_cpp_path_for_macro(element, namespaces, options)
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
        ancestor_anchor.add_search_weight(e.search_weight);
        ancestor_anchor.add_class([&e]() {
          switch (e.type) {
            case CppPathProject: return "project-name";
            case CppPathMacro: return "macro-name";
            default:
              sus_unreachable();  // Macros are only in the global namespace.
          }
        }());
        ancestor_anchor.add_href(e.link_href);
        ancestor_anchor.write_text(e.name);
      }
    }
  }
  {
    auto overload_set_div = section_div.open_div();
    overload_set_div.add_class("overload-set");
    {
      auto overload_div = overload_set_div.open_div();
      overload_div.add_class("overload");

      {
        auto signature_div = overload_div.open_div(HtmlWriter::SingleLine);
        signature_div.add_class("macro-signature");

        generate_source_link(signature_div, element);

        {
          auto auto_span = signature_div.open_span(HtmlWriter::SingleLine);
          auto_span.add_class("macro-define");
          auto_span.write_text("#define");
        }
        signature_div.write_text(" ");
        {
          auto name_anchor = signature_div.open_a();
          name_anchor.add_href("#");
          name_anchor.add_class("macro-name");
          name_anchor.write_text(element.name);
        }
        if (element.parameters.is_some()) {
          signature_div.write_text("(");
          for (const auto& [i, p] :
               element.parameters.as_value().iter().enumerate()) {
            if (i > 0u) signature_div.write_text(", ");
            signature_div.write_text(p);
          }
          signature_div.write_text(")");
        }
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

sus::Result<void, MarkdownToHtmlError> generate_macro_reference(
    HtmlWriter::OpenUl& items_list, const MacroElement& element,
    ParseMarkdownPageState& page_state) noexcept {
  auto item_li = items_list.open_li();
  item_li.add_class("section-item");

  {
    auto overload_set_div = item_li.open_div();
    overload_set_div.add_class("overload-set");
    overload_set_div.add_class("item-name");

    {
      auto overload_div = overload_set_div.open_div();
      overload_div.add_class("overload");

      {
        auto signature_div = overload_div.open_div(HtmlWriter::SingleLine);
        signature_div.add_class("macro-signature");
        signature_div.write_text(" ");
        {
          auto link_anchor = signature_div.open_a();
          if (!element.hidden()) {
            link_anchor.add_href(construct_html_url_for_macro(element));
          } else {
            llvm::errs() << "WARNING: Reference to hidden MacroElement "
                         << element.name << " in namespace "
                         << element.namespace_path;
          }
          link_anchor.add_class("macro-name");
          link_anchor.write_text(element.name);
        }
      }
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

}  // namespace subdoc::gen
