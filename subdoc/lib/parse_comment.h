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

#include "subdoc/llvm.h"
#include "subspace/result/result.h"

namespace subdoc {

struct ParseCommentError {
  std::string message;
};

inline sus::result::Result<std::string, ParseCommentError> parse_comment(
    clang::ASTContext& ast_cx, const clang::RawComment& raw) noexcept {
  auto& src_manager = ast_cx.getSourceManager();
  const llvm::StringRef text = raw.getRawText(src_manager);
  const llvm::StringRef eol = text.detectEOL();

  std::ostringstream parsed;

  switch (raw.getKind()) {
    case clang::RawComment::CommentKind::RCK_BCPLSlash:  // `/// Foo`
      [[fallthrough]];
    case clang::RawComment::CommentKind::RCK_JavaDoc: {  // `/** Foo`
      std::vector<clang::RawComment::CommentLine> lines =
          raw.getFormattedLines(src_manager, ast_cx.getDiagnostics());
      bool add_newline = false;
      for (const auto& line : lines) {
        if (add_newline) parsed << "\n";
        parsed << std::string(line.Text);
        add_newline = true;
      }
      break;
    }
    case clang::RawComment::CommentKind::RCK_OrdinaryBCPL:  // `// Foo`
      // TODO: Conditionally collect these?
      break;
    case clang::RawComment::CommentKind::RCK_OrdinaryC:  // `/* Foo`
      // TODO: Conditionally collect these?
      break;
    case clang::RawComment::CommentKind::RCK_Merged:  // More than one.
      return sus::result::err(
          ParseCommentError{.message = "Merged comment format?"});
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
  }

  return sus::result::ok(parsed.str());
}

}  // namespace subdoc
