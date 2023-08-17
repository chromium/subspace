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

#include "subdoc/lib/parse_comment.h"

#include <sstream>

#include "sus/ops/range.h"

#pragma warning(push)
#pragma warning(disable : 4244)
#include "third_party/md4c/src/md4c-html.h"
#include "third_party/md4c/src/md4c.h"
#pragma warning(pop)

namespace subdoc {

namespace {

/// Grabs the contents of the first non-empty html tag as the summary.
inline std::string summarize_html(std::string_view html) {
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

sus::Result<std::string, ParseCommentError> parse_comment_markdown_to_html(
    sus::SliceMut<std::string> lines) noexcept {
  std::ostringstream parsed;

  struct UserData {
    std::ostringstream& parsed;
  };
  UserData data(parsed);

  auto process_output = [](const MD_CHAR* chars, MD_SIZE size, void* v) {
    auto& data = *reinterpret_cast<UserData*>(v);
    data.parsed << std::string_view(chars, size);
  };

  std::string mdtext = [&]() {
    std::ostringstream s;
    for (const auto& line : lines) {
      s << line << "\n";
    }
    return sus::move(s).str();
  }();
  int result =
      md_html(mdtext.c_str(), u32::try_from(mdtext.size()).unwrap(),
              process_output, &data,
              MD_FLAG_PERMISSIVEAUTOLINKS | MD_FLAG_NOHTMLBLOCKS |
                  MD_FLAG_NOHTMLSPANS | MD_FLAG_TABLES | MD_FLAG_STRIKETHROUGH,
              0);
  if (result != 0)
    return sus::err(ParseCommentError{.message = "Failed to parse markdown"});
  return sus::ok(sus::move(parsed).str());
}

sus::Result<ParsedComment, ParseCommentError> parse_comment(
    clang::ASTContext& ast_cx, const clang::RawComment& raw,
    std::string_view self_name) noexcept {
  auto& src_manager = ast_cx.getSourceManager();
  const llvm::StringRef text = raw.getRawText(src_manager);
  const llvm::StringRef eol = text.detectEOL();

  std::string html;
  DocAttributes attrs;

  clang::RawComment::CommentKind kind = raw.getKind();
  if (kind == clang::RawComment::CommentKind::RCK_Merged) {
    // We get RCK_Merged in a lot of cases though the comment is all `///`
    // format (RCK_BCPLSlash).
    if (text.starts_with("/// "))
      kind = clang::RawComment::CommentKind::RCK_BCPLSlash;
    if (text.starts_with("/** "))
      kind = clang::RawComment::CommentKind::RCK_JavaDoc;
  }

  switch (kind) {
    case clang::RawComment::CommentKind::RCK_BCPLSlash:  // `/// Foo`
      [[fallthrough]];
    case clang::RawComment::CommentKind::RCK_JavaDoc: {  // `/** Foo`
      // Ignore `///////...` or `/******...`.
      if (text.starts_with("////") || text.starts_with("/***")) break;
      attrs.location = raw.getBeginLoc();

      std::vector<clang::RawComment::CommentLine> lines =
          raw.getFormattedLines(src_manager, ast_cx.getDiagnostics());
      auto parsed_lines = sus::Vec<std::string>::with_capacity(lines.size());

      for (const auto& line_ref : lines) {
        auto line = std::string(line_ref.Text);

        // Substitute @doc.self with the name of the type. This also gets
        // applied inside subdoc attributes.
        while (true) {
          auto pos = line.find("@doc.self");
          if (pos == std::string::npos) break;
          line.replace(pos, strlen("@doc.self"), self_name);
        }

        // TODO: Better and more robust parser and error messages.
        if (line.starts_with("#[doc.") &&
            line.rfind("]") != std::string::npos) {
          std::string_view v =
              std::string_view(line).substr(6u, line.rfind("]") - 6u);
          if (v.starts_with("overloads=")) {
            std::string_view name = v.substr(strlen("overloads="));
            attrs.overload_set =
                sus::some(std::hash<std::string_view>()(name.data()));
          } else if (v == "hidden") {
            attrs.hidden = true;
          } else if (v.starts_with("inherit=")) {
            std::string_view name = v.substr(strlen("inherit="));
            auto vec = sus::Vec<InheritPathElement>();
            while (name != "") {
              auto [element, remainder] = llvm::StringRef(name).split("::");
              name = remainder;
              // TODO: This syntax sucks, and it's expensive to look up later.
              // Should we just have a (globally-unique?
              // top-level-namepace-unique?) identifier that a comment can set
              // on itself, and then you inherit from that identifier?
              if (element.starts_with("[n]")) {
                vec.push(InheritPathElement::with<InheritPathNamespace>(
                    std::string(element.substr(3u))));
              } else if (element.starts_with("[r]")) {
                vec.push(InheritPathElement::with<InheritPathRecord>(
                    std::string(element.substr(3u))));
              } else if (element.starts_with("[f]")) {
                // TODO: We should be able to name if the function is static or
                // not, and its documentation overload set name.
                vec.push(InheritPathElement::with<InheritPathFunction>(
                    std::string(element.substr(3u))));
              } else {
                std::ostringstream m;
                m << "Invalid path element '" << std::string_view(element)
                  << "' in doc.inherit: ";
                m << sus::move(line);
                return sus::err(ParseCommentError{.message = m.str()});
              }
            }
            attrs.inherit = sus::some(sus::move(vec));
          } else {
            std::ostringstream m;
            m << "Unknown doc attribute " << std::string_view(v) << " in: ";
            m << sus::move(line);
            return sus::err(ParseCommentError{.message = m.str()});
          }
        } else if (line.find("#[doc") != std::string::npos) {
          std::ostringstream m;
          m << "Unused doc comment in: ";
          m << sus::move(line);
          return sus::err(ParseCommentError{.message = m.str()});
        } else {
          // Drop the trailing ' +\' suffix.
          if (line.ends_with("\\")) line.pop_back();
          while (line.ends_with(" ")) line.pop_back();
          parsed_lines.push(sus::move(line));
        }
      }

      auto parsed = parse_comment_markdown_to_html(parsed_lines);
      if (parsed.is_err()) return sus::err(sus::move(parsed).unwrap_err());
      html = sus::move(parsed).unwrap();
      break;
    }
    case clang::RawComment::CommentKind::RCK_OrdinaryBCPL:  // `// Foo`
      // TODO: Conditionally collect these?
      break;
    case clang::RawComment::CommentKind::RCK_OrdinaryC:  // `/* Foo`
      // TODO: Conditionally collect these?
      break;
    case clang::RawComment::CommentKind::RCK_Qt:  // `/*! Foo`
      return sus::err(
          ParseCommentError{.message = "Invalid comment format `/*!`"});
      break;
    case clang::RawComment::CommentKind::RCK_BCPLExcl:  // `//! Foo`
      return sus::err(
          ParseCommentError{.message = "Invalid comment format `//!`"});
    case clang::RawComment::CommentKind::RCK_Invalid:  // `/* Foo`
      return sus::err(ParseCommentError{
          .message = "Invalid comment format, unable to parse"});
    case clang::RawComment::CommentKind::RCK_Merged:  // More than one.
      return sus::err(ParseCommentError{.message = "Merged comment format?"});
  }

  auto summary = summarize_html(html);
  return sus::ok(
      ParsedComment(sus::move(attrs), sus::move(html), sus::move(summary)));
}

}  // namespace subdoc
