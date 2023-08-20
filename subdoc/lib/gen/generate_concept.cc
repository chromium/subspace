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

#include "subdoc/lib/gen/generate_concept.h"

#include <sstream>

#include "subdoc/lib/database.h"
#include "subdoc/lib/gen/files.h"
#include "subdoc/lib/gen/generate_cpp_path.h"
#include "subdoc/lib/gen/generate_head.h"
#include "subdoc/lib/gen/generate_requires.h"
#include "subdoc/lib/gen/html_writer.h"
#include "subdoc/lib/gen/markdown_to_html.h"
#include "subdoc/lib/gen/options.h"
#include "subdoc/lib/parse_comment.h"
#include "sus/assertions/unreachable.h"
#include "sus/prelude.h"

namespace subdoc::gen {

namespace {

void generate_concept_overview(HtmlWriter::OpenDiv& record_div,
                               const ConceptElement& element,
                               sus::Slice<const NamespaceElement*> namespaces,
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
      record_type_span.write_text("Concept");
    }
    for (auto [i, e] :
         generate_cpp_path_for_concept(element, namespaces, options)
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
            case CppPathRecord:
              break;  // Reccord can't be an ancesor of a concept.
            case CppPathFunction:
              break;  // Function can't be an ancesor of a concept.
            case CppPathConcept: return "concept-name";
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
      auto concept_keyword_span = type_sig_div.open_span();
      concept_keyword_span.add_class("concept");
      concept_keyword_span.write_text("concept");
    }
    {
      auto name_span = type_sig_div.open_span();
      name_span.add_class("type-name");
      name_span.write_text(element.name);
    }
    {
      auto concept_body_div = type_sig_div.open_div();
      generate_requires_constraints(concept_body_div, element.constraints);
    }
  }
  if (element.has_comment()) {
    auto desc_div = section_div.open_div();
    desc_div.add_class("description");
    desc_div.add_class("long");
    desc_div.write_html(
        markdown_to_html_full(element.comment, page_state).unwrap());
  }
}

}  // namespace

void generate_concept(const Database& db, const ConceptElement& element,
                      sus::Slice<const NamespaceElement*> namespaces,
                      const Options& options) noexcept {
  if (element.hidden()) return;

  ParseMarkdownPageState page_state(db);

  const std::filesystem::path path =
      construct_html_file_path_for_concept(options.output_root, element);
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

  auto record_div = body.open_div();
  record_div.add_class("concept");
  generate_concept_overview(record_div, element, namespaces, page_state,
                            options);
}

void generate_concept_reference(HtmlWriter::OpenUl& items_list,
                                const ConceptElement& element,
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
        name_link.add_href(construct_html_url_for_concept(element));
      } else {
        llvm::errs() << "WARNING: Reference to hidden ConceptElement "
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
      desc_div.write_html(
          markdown_to_html_summary(element.comment, page_state).unwrap());
  }
}

}  // namespace subdoc::gen
