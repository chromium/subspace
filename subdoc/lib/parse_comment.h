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

#include <string>

#include "subdoc/lib/doc_attributes.h"
#include "subdoc/llvm.h"
#include "sus/ops/range.h"
#include "sus/result/result.h"

namespace subdoc {

struct ParseCommentError {
  std::string message;
};

struct ParseMarkdownPageState {
  std::unordered_map<std::string, u32> self_link_counts;
};

sus::Result<std::string, ParseCommentError> parse_comment_markdown_to_html(
    std::string_view text, ParseMarkdownPageState& page_state) noexcept;

/// Grabs the contents of the first non-empty html tag as the summary.
std::string summarize_html(std::string_view html) noexcept;

struct ParsedComment {
  DocAttributes attributes;
  std::string text;
};

sus::Result<ParsedComment, ParseCommentError> parse_comment(
    clang::ASTContext& ast_cx, const clang::RawComment& raw,
    std::string_view self_name) noexcept;

}  // namespace subdoc
