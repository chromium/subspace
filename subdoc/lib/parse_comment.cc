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

namespace subdoc {

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

      std::ostringstream stream;
      for (const clang::RawComment::CommentLine& line_ref :
           raw.getFormattedLines(src_manager, ast_cx.getDiagnostics())) {
        auto line = std::string(line_ref.Text);

        // Substitute @doc.self with the name of the type. This also gets
        // applied inside subdoc attributes.
        while (true) {
          auto pos = line.find("@doc.self");
          if (pos == std::string::npos) break;
          if (self_name.empty()) {
            return sus::err(ParseCommentError{
                .message = "@doc.self used outside of a class"});
          }
          line.replace(pos, strlen("@doc.self"), self_name);
        }

        // TODO: Better and more robust parser and error messages.
        if (line.starts_with("#[doc.") &&
            line.rfind("]") != std::string::npos) {
          std::string_view v =
              std::string_view(line).substr(6u, line.rfind("]") - 6u);
          if (v.starts_with("overloads=")) {
            attrs.overload_set =
                sus::some(std::string(v.substr(strlen("overloads="))));
          } else if (v == "hidden") {
            attrs.hidden = true;
          } else if (v.starts_with("inherit=")) {
            std::string_view name = v.substr(strlen("inherit="));
            auto vec = Vec<InheritPathElement>();
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
          auto v = std::string_view(line);
          if (v.ends_with("\\")) v = v.substr(0u, v.size() - 1u);
          while (v.ends_with(" ")) v = v.substr(0u, v.size() - 1u);
          stream << v << "\n";
        }
      }

      html = sus::move(stream).str();
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
    case clang::RawComment::CommentKind::RCK_BCPLExcl:  // `//! Foo`
      return sus::err(
          ParseCommentError{.message = "Invalid comment format `//!`"});
    case clang::RawComment::CommentKind::RCK_Invalid:  // `/* Foo`
      return sus::err(ParseCommentError{
          .message = "Invalid comment format, unable to parse"});
    case clang::RawComment::CommentKind::RCK_Merged:  // More than one.
      return sus::err(ParseCommentError{.message = "Merged comment format?"});
  }

  return sus::ok(ParsedComment(sus::move(attrs), sus::move(html)));
}

}  // namespace subdoc
