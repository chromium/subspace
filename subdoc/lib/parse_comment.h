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
#include "subspace/result/result.h"

namespace subdoc {

struct ParsedComment {
  DocAttributes attributes;
  std::string comment;
};

struct ParseCommentError {
  std::string message;
};

inline sus::result::Result<ParsedComment, ParseCommentError> parse_comment(
    clang::ASTContext& ast_cx, const clang::RawComment& raw) noexcept {
  auto& src_manager = ast_cx.getSourceManager();
  const llvm::StringRef text = raw.getRawText(src_manager);
  const llvm::StringRef eol = text.detectEOL();

  DocAttributes attrs;
  std::ostringstream parsed;

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
      attrs.location = raw.getBeginLoc();

      std::vector<clang::RawComment::CommentLine> lines =
          raw.getFormattedLines(src_manager, ast_cx.getDiagnostics());
      bool add_newline = false;
      for (const auto& line : lines) {
        // TODO: Better and more robust parser and error messages.
        if (line.Text.starts_with("#[doc.") &&
            line.Text.rfind("]") != std::string::npos) {
          llvm::StringRef v =
              llvm::StringRef(line.Text).substr(6u, line.Text.rfind("]") - 6u);
          if (v.starts_with("overloads=")) {
            llvm::StringRef name = v.substr(strlen("overloads="));
            attrs.overload_set = sus::some(
                std::hash<std::string_view>()(std::string_view(name.data())));
          } else if (v.starts_with("inherit=")) {
            llvm::StringRef name = v.substr(strlen("inherit="));
            auto vec = sus::Vec<InheritPathElement>();
            while (name != "") {
              auto [element, remainder] = name.split("::");
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
                m << line.Text;
                return sus::result::err(ParseCommentError{.message = m.str()});
              }
            }
            attrs.inherit = sus::some(sus::move(vec));
          } else {
            std::ostringstream m;
            m << "Unknown doc attribute " << std::string_view(v) << " in: ";
            m << line.Text;
            return sus::result::err(ParseCommentError{.message = m.str()});
          }
        } else if (line.Text.find("#[doc") != std::string::npos) {
          std::ostringstream m;
          m << "Unused doc comment in: ";
          m << line.Text;
          return sus::result::err(ParseCommentError{.message = m.str()});
        } else {
          if (add_newline) parsed << "\n";
          parsed << line.Text;
          add_newline = true;
        }
      }
      break;
    }
    case clang::RawComment::CommentKind::RCK_OrdinaryBCPL:  // `// Foo`
      // TODO: Conditionally collect these?
      break;
    case clang::RawComment::CommentKind::RCK_OrdinaryC:  // `/* Foo`
      // TODO: Conditionally collect these?
      break;
    case clang::RawComment::CommentKind::RCK_Qt:  // `/*! Foo`
      return sus::result::err(
          ParseCommentError{.message = "Invalid comment format `/*!`"});
      break;
    case clang::RawComment::CommentKind::RCK_BCPLExcl:  // `//! Foo`
      return sus::result::err(
          ParseCommentError{.message = "Invalid comment format `//!`"});
    case clang::RawComment::CommentKind::RCK_Invalid:  // `/* Foo`
      return sus::result::err(ParseCommentError{
          .message = "Invalid comment format, unable to parse"});
    case clang::RawComment::CommentKind::RCK_Merged:  // More than one.
      return sus::result::err(
          ParseCommentError{.message = "Merged comment format?"});
      break;
  }

  return sus::result::ok(
      ParsedComment(sus::move(attrs), sus::move(parsed).str()));
}

}  // namespace subdoc
