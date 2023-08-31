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

#include "fmt/format.h"
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

/// Removes html tags, leaving behind the text content.
std::string drop_tags(std::string_view html) noexcept {
  if (html.empty()) return std::string(html);

  std::ostringstream s;

  bool inside_tag = false;
  sus::Option<usize> text_start;
  for (auto i : sus::ops::range<usize>(0u, html.size())) {
    if (inside_tag) {
      if (html[i] == '>') {
        inside_tag = false;
        text_start.insert(i + 1u);
      }
    } else {
      if (html.substr(i).starts_with("<")) {
        inside_tag = true;
        if (text_start.is_some()) {
          s << html.substr(*text_start, i - *text_start);
          text_start = sus::none();
        }
      }
    }
  }
  return sus::move(s).str();
}

}  // namespace

sus::Result<MarkdownToHtml, MarkdownToHtmlError> markdown_to_html(
    const Comment& comment, ParseMarkdownPageState& page_state) noexcept {
  std::ostringstream parsed;

  struct UserData {
    std::ostringstream& parsed;
    ParseMarkdownPageState& page_state;
    sus::Option<std::string> error_message;
  };
  UserData data(parsed, page_state, sus::none());

  auto process_output = [](const MD_CHAR* chars, MD_SIZE size, void* v) {
    auto& userdata = *reinterpret_cast<UserData*>(v);
    userdata.parsed << std::string_view(chars, size);
  };
  auto render_self_link = [](const MD_CHAR* chars, MD_SIZE size, void* v,
                             MD_HTML* html,
                             int (*render)(MD_HTML* html, const MD_CHAR* chars,
                                           MD_SIZE size)) -> int {
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

    int result = 0;
    result = render(html, mapped.data(), u32::try_from(mapped.size()).unwrap());
    if (result != 0) return result;
    if (count > 0u) {
      result = render(html, "-", 1u);
      if (result != 0) return result;
      while (count > 0u) {
        static const char NUMS[] = "0123456789";
        result = render(html, NUMS + (count % 10u), 1u);
        if (result != 0) return result;
        count /= 10u;
      }
    }
    return result;
  };
  auto record_self_link = [](const MD_CHAR* chars, MD_SIZE size,
                             void* v) -> int {
    auto& userdata = *reinterpret_cast<UserData*>(v);
    auto mapped = std::string(std::string_view(chars, size));
    if (auto it = userdata.page_state.self_link_counts.find(mapped);
        it != userdata.page_state.self_link_counts.end()) {
      it->second += 1u;
    } else {
      userdata.page_state.self_link_counts.emplace(sus::move(mapped), 1u);
    }
    return 0;
  };
  auto render_code_link = [](const MD_CHAR* chars, MD_SIZE size, void* v,
                             MD_HTML* html,
                             int (*render)(MD_HTML* html, const MD_CHAR* chars,
                                           MD_SIZE size)) -> int {
    auto& userdata = *reinterpret_cast<UserData*>(v);
    auto name = std::string_view(chars, size);
    auto anchor = std::string_view();
    if (auto pos = name.find('#'); pos != std::string_view::npos) {
      anchor = name.substr(pos);
      name = name.substr(0u, pos);
    }
    sus::Option<FoundName> found = userdata.page_state.db.find_name(name);
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
      int r = render(html, href.data(), u32::try_from(href.size()).unwrap());
      if (r != 0) return r;
      if (anchor.size() > 0u)
        r = render(html, anchor.data(), u32::try_from(anchor.size()).unwrap());
      return r;
    } else {
      std::string msg =
          fmt::format("unable to resolve code link '{}' to a C++ name",
                      std::string_view(chars, size));
      if (userdata.page_state.options.ignore_bad_code_links) {
        fmt::println("WARNING: {}", msg);
        return 0;
      } else {
        userdata.error_message = sus::some(sus::move(msg));
        return -1;
      }
    }
  };

  int result = md_html(
      comment.text.data(), u32::try_from(comment.text.size()).unwrap(),
      MD_HTML_CALLBACKS{
          process_output,
          render_self_link,
          record_self_link,
          render_code_link,
      },
      &data,
      MD_FLAG_PERMISSIVEAUTOLINKS | MD_FLAG_TABLES | MD_FLAG_STRIKETHROUGH |
          // Forked extensions.
          MD_FLAG_HEADERSELFLINKS | MD_FLAG_CODELINKS,
      // We enable MD_ASSERT() to catch memory safety bugs, so
      // ensure something gets printed should a problem occur.
      MD_HTML_FLAG_DEBUG);
  if (result != 0) {
    if (data.error_message.is_some()) {
      return sus::err(
          MarkdownToHtmlError{.message = data.error_message.take().unwrap()});
    } else {
      return sus::err(MarkdownToHtmlError{
          .message = fmt::format("unknown parsing error '{}'", result)});
    }
  }
  std::string str = sus::move(parsed).str();
  return sus::ok(MarkdownToHtml{
      .full_html = sus::clone(str),
      .summary_html = summarize_html(str),
      .summary_text = drop_tags(summarize_html(str)),
  });
}

}  // namespace subdoc::gen
