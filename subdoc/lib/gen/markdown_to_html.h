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

#include "subdoc/lib/database.h"
#include "subdoc/lib/gen/options.h"
#include "sus/error/error.h"
#include "sus/result/result.h"

namespace subdoc::gen {

struct ParseMarkdownPageState {
  const Database& db;
  const Options& options;
  std::unordered_map<std::string, u32> self_link_counts;
};

struct MarkdownToHtmlError {
  std::string message;
};

struct MarkdownToHtml {
  std::string full_html;
  std::string summary_html;
  std::string summary_text;
};

sus::Result<MarkdownToHtml, MarkdownToHtmlError> markdown_to_html(
    const Comment& comment, ParseMarkdownPageState& page_state) noexcept;

}  // namespace subdoc::gen

// `sus::error::Error` implementation.
template <>
struct sus::error::ErrorImpl<subdoc::gen::MarkdownToHtmlError> {
  using MarkdownToHtmlError = subdoc::gen::MarkdownToHtmlError;
  static std::string display(const MarkdownToHtmlError& e) noexcept {
    return fmt::format("markdown parsing failed: {}", e.message);
  }
};
