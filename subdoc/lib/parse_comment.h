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

#pragma once

#include <sstream>
#include <string>

#include "subdoc/lib/doc_attributes.h"
#include "subdoc/llvm.h"
#include "sus/ops/range.h"
#include "sus/result/result.h"

namespace subdoc {

struct ParsedComment {
  DocAttributes attributes;
  std::string full_comment;
  std::string summary;
};

struct ParseCommentError {
  std::string message;
};

/// Grabs the contents of the first non-empty html tag as the summary.
inline std::string summarize_html(std::string_view html) {
  if (html.empty()) return std::string(html);

  bool inside_tag = false;
  i32 tag_depth = 0;
  sus::Option<size_t> start_non_empty;
  for (auto i : sus::ops::range(size_t{0u}, html.size())) {
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

inline sus::result::Result<std::string, ParseCommentError>
parse_comment_markdown_to_html(sus::SliceMut<std::string> lines) noexcept {
  std::ostringstream parsed;
  while (lines.first().is_some() && lines.first().as_value().empty()) {
    lines = lines["1.."_r];
  }
  while (lines.last().is_some() && lines.last().as_value().empty()) {
    lines = lines[".."_r.end_at(lines.len() - 1u)];
  }

  if (!lines.is_empty()) {
    parsed << "<p>";
    bool add_space = false;
    bool inside_pre = false;
    bool inside_code_snippet = false;
    for (std::string& s : lines.iter_mut()) {
      // Quote any <>.
      while (true) {
        size_t pos = s.find_first_of("<>");
        if (pos == std::string::npos) break;
        if (s[pos] == '<')
          s.replace(pos, 1u, "&lt;");
        else
          s.replace(pos, 1u, "&gt;");
      }

      if (s.empty()) {
        // Empty line, preserve the paragraph break.
        parsed << "</p><p>";
        add_space = false;
      } else if (s.starts_with("#") && !(inside_pre || inside_code_snippet)) {
        // Markdown header.
        if (s.starts_with("##### "))
          parsed << "</p><h5>" << sus::move(s).substr(6) << "</h5><p>";
        else if (s.starts_with("#### "))
          parsed << "</p><h4>" << sus::move(s).substr(5) << "</h4><p>";
        else if (s.starts_with("### "))
          parsed << "</p><h3>" << sus::move(s).substr(4) << "</h3><p>";
        else if (s.starts_with("## "))
          parsed << "</p><h2>" << sus::move(s).substr(3) << "</h2><p>";
        else if (s.starts_with("# "))
          parsed << "</p><h1>" << sus::move(s).substr(2) << "</h1><p>";
        else
          parsed << "</p><h6>" << sus::move(s) << "</h6><p>";
        add_space = false;
      } else if (s.starts_with("```")) {
        // Markdown code blocks with ``` at the start and end.
        inside_pre = !inside_pre;
        if (inside_pre) {
          if (inside_code_snippet) {
            return sus::err(ParseCommentError{
                .message = "Invalid markdown, found ``` inside `"});
          }
          // TODO: After the opening ``` there can be a language for syntax
          // highlighting...
          parsed << "</p><pre><code>";
        } else {
          parsed << "</code></pre><p>";
          add_space = false;
        }
      } else if (inside_pre) {
        parsed << sus::move(s) << "\n";
      } else {
        // Markdown code snippets with `foo` format.
        while (true) {
          size_t start = s.find("`");
          if (start == std::string::npos) break;
          inside_code_snippet = !inside_code_snippet;
          if (inside_code_snippet) {
            size_t end = s.find("`", start + 1u);
            if (end != std::string::npos) {
              s.replace(end, 1, "</code>");
              inside_code_snippet = false;
            }
            s.replace(start, 1, "<code>");
          } else {
            s.replace(start, 1, "</code>");
          }
        }

        // Finally add the text!
        if (add_space) parsed << " ";
        parsed << sus::move(s);
        add_space = true;
      }
    }
    parsed << "</p>";

    // A `snippet` was not terminated.
    if (inside_code_snippet) {
      return sus::err(
          ParseCommentError{.message = "Unterminated code ` snippet"});
    }
    if (inside_pre) {
      return sus::err(
          ParseCommentError{.message = "Unterminated code ``` block"});
    }
  }
  return sus::ok(sus::move(parsed).str());
}

inline sus::Result<ParsedComment, ParseCommentError> parse_comment(
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
          // TODO: Use the name of the type!
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
