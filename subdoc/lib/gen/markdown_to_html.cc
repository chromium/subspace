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

#include "subdoc/lib/gen/markdown_to_html.h"

#include <sstream>

#include "subdoc/lib/gen/files.h"
#include "sus/ops/range.h"

#pragma warning(push)
#pragma warning(disable : 4244)
#include "third_party/md4c/src/md4c-html.h"
#include "third_party/md4c/src/md4c.h"
#pragma warning(pop)

namespace subdoc::gen {

namespace {

/// Grabs the contents of the first non-empty html tag as the summary.
std::string summarize_html(std::string_view html) noexcept {
  if (html.empty()) return std::string(html);

  bool inside_tag = false;
  i32 tag_depth = 0;
  sus::Option<size_t> start_non_empty;
  for (auto i : sus::ops::range<usize>(0u, html.size())) {
    if (inside_tag) {
      if (html[i] == '>') {
        inside_tag = false;
      }
    } else {
      if (html.substr(i).starts_with("<")) {
        inside_tag = true;
        if (start_non_empty.is_some()) {
          if (html.substr(i).starts_with("</")) {
            if (start_non_empty.is_some() && tag_depth == 0) {
              std::ostringstream summary;
              summary << "<p>";
              summary << html.substr(start_non_empty.as_value(),
                                     i - start_non_empty.as_value());
              summary << "</p>";
              return summary.str();
            }

            tag_depth -= 1;
          } else {
            tag_depth += 1;
          }
        }
      } else {
        // We see a character that's not part of an html tag.
        start_non_empty = start_non_empty.or_that(sus::some(i));
      }
    }
  }
  llvm::errs() << "WARNING: Html summary could not find non-empty tag pair.\n";
  llvm::errs() << html << "\n";
  return std::string(html);
}

}  // namespace

sus::Result<std::string, MarkdownToHtmlError> markdown_to_html_full(
    const Comment& comment, ParseMarkdownPageState& page_state) noexcept {
  std::ostringstream parsed;

  struct UserData {
    std::ostringstream& parsed;
    ParseMarkdownPageState& page_state;
  };
  UserData data(parsed, page_state);

  auto process_output = [](const MD_CHAR* chars, MD_SIZE size, void* v) {
    auto& userdata = *reinterpret_cast<UserData*>(v);
    userdata.parsed << std::string_view(chars, size);
  };
  auto render_self_link =
      [](const MD_CHAR* chars, MD_SIZE size, void* v, MD_HTML* html,
         void (*render)(MD_HTML* html, const MD_CHAR* chars, MD_SIZE size)) {
        auto& userdata = *reinterpret_cast<UserData*>(v);

        std::string mapped(std::string_view(chars, size));
        auto count = 0_u32;
        if (auto it = userdata.page_state.self_link_counts.find(mapped);
            it != userdata.page_state.self_link_counts.end()) {
          count = it->second;
        }

        for (char& c : mapped) {
          if (c == ' ') {
            c = '-';
          } else {
            // SAFETY: The input is a char, so tolower will give a value in the
            // range of char.
            c = sus::mog<char>(std::tolower(c));
          }
        }

        render(html, mapped.data(), u32::try_from(mapped.size()).unwrap());
        if (count > 0u) {
          render(html, "-", 1u);
          while (count > 0u) {
            static const char NUMS[] = "0123456789";
            render(html, NUMS + (count % 10u), 1u);
            count /= 10u;
          }
        }
      };
  auto record_self_link = [](const MD_CHAR* chars, MD_SIZE size, void* v) {
    auto& userdata = *reinterpret_cast<UserData*>(v);
    auto mapped = std::string(std::string_view(chars, size));
    if (auto it = userdata.page_state.self_link_counts.find(mapped);
        it != userdata.page_state.self_link_counts.end()) {
      it->second += 1u;
    } else {
      userdata.page_state.self_link_counts.emplace(sus::move(mapped), 1u);
    }
  };
  auto render_code_link =
      [](const MD_CHAR* chars, MD_SIZE size, void* v, MD_HTML* html,
         void (*render)(MD_HTML* html, const MD_CHAR* chars, MD_SIZE size)) {
        auto& userdata = *reinterpret_cast<UserData*>(v);
        sus::Option<FoundName> found =
            userdata.page_state.db.find_name(std::string_view(chars, size));
        if (found.is_some()) {
          std::string href;
          switch (found.as_value()) {
            case FoundName::Tag::Namespace:
              href = construct_html_url_for_namespace(
                  found.as_value().as<FoundName::Tag::Namespace>());
              break;
            case FoundName::Tag::Function: {
              href = construct_html_url_for_function(
                  found.as_value().as<FoundName::Tag::Function>());
              break;
            }
            case FoundName::Tag::Type:
              href = construct_html_url_for_type(
                  found.as_value().as<FoundName::Tag::Type>());
              break;

            case FoundName::Tag::Concept:
              href = construct_html_url_for_concept(
                  found.as_value().as<FoundName::Tag::Concept>());
              break;
            case FoundName::Tag::Field:
              href = construct_html_url_for_field(
                  found.as_value().as<FoundName::Tag::Field>());
              break;
          }
          render(html, href.data(), u32::try_from(href.size()).unwrap());
        } else {
          fmt::println("ERROR: missing code link {}",
                       std::string_view(chars, size));
          render(html, chars, size);
        }
      };

  int result =
      md_html(comment.text.data(), u32::try_from(comment.text.size()).unwrap(),
              MD_HTML_CALLBACKS{
                  process_output,
                  render_self_link,
                  record_self_link,
                  render_code_link,
              },
              &data,
              MD_FLAG_PERMISSIVEAUTOLINKS | MD_FLAG_NOHTMLBLOCKS |
                  MD_FLAG_NOHTMLSPANS | MD_FLAG_TABLES | MD_FLAG_STRIKETHROUGH |
                  // Forked extensions.
                  MD_FLAG_HEADERSELFLINKS | MD_FLAG_CODELINKS,
              0);
  if (result != 0)
    return sus::err(MarkdownToHtmlError{.message = "Failed to parse markdown"});
  return sus::ok(sus::move(parsed).str());
}

sus::Result<std::string, MarkdownToHtmlError> markdown_to_html_summary(
    const Comment& comment, ParseMarkdownPageState& page_state) noexcept {
  return markdown_to_html_full(comment, page_state)
      .and_then(
          [](std::string s) -> sus::Result<std::string, MarkdownToHtmlError> {
            return sus::ok(summarize_html(sus::move(s)));
          });
}

}  // namespace subdoc::gen
