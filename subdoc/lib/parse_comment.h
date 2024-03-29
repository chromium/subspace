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
#include "sus/error/error.h"
#include "sus/result/result.h"

namespace subdoc {

struct ParsedComment {
  DocAttributes attributes;
  std::string text;
};

struct ParseCommentError {
  std::string message;
};

sus::Result<ParsedComment, ParseCommentError> parse_comment(
    clang::ASTContext& ast_cx, const clang::RawComment& raw,
    std::string_view self_name) noexcept;

}  // namespace subdoc

// `sus::error::Error` implementation.
template <>
struct sus::error::ErrorImpl<subdoc::ParseCommentError> {
  using ParseCommentError = subdoc::ParseCommentError;
  static std::string display(const ParseCommentError& e) noexcept {
    return fmt::format("doc comment was invalid: {}", e.message);
  }
};
