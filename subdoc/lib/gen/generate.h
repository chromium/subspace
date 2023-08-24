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

#include <system_error>

#include "subdoc/lib/database.h"
#include "subdoc/lib/gen/markdown_to_html.h"
#include "subdoc/lib/gen/options.h"
#include "sus/boxed/box.h"
#include "sus/choice/choice.h"
#include "sus/error/error.h"
#include "sus/prelude.h"
#include "sus/result/result.h"

namespace subdoc::gen {

struct GenerateFileError {
  std::string path;
  sus::Box<sus::error::DynError> source;
};

enum class GenerateErrorTag {
  CopyFileError,
  DeleteFileError,
  MarkdownError,
};
/// GenerateError is an error type that always comes with an
/// [`error_source`]($sus::error::error_source).
using GenerateError = sus::Choice<sus_choice_types(
    (GenerateErrorTag::CopyFileError, GenerateFileError),
    (GenerateErrorTag::DeleteFileError, GenerateFileError),
    (GenerateErrorTag::MarkdownError, sus::Box<sus::error::DynError>))>;

/// Generate the html output from the database.
///
/// Returns an error if generation fails.
sus::result::Result<void, GenerateError> generate(const Database& db,
                                                  const Options& options);

}  // namespace subdoc::gen

// `sus::error::Error` implementation.
template <>
struct sus::error::ErrorImpl<subdoc::gen::GenerateError> {
  using GenerateError = subdoc::gen::GenerateError;
  static std::string display(const GenerateError& e) noexcept {
    switch (e) {
      case GenerateError::Tag::CopyFileError: {
        const auto& p = e.as<GenerateError::Tag::CopyFileError>();
        return fmt::format("copying file '{}' failed", p.path);
      }
      case GenerateError::Tag::DeleteFileError: {
        const auto& p = e.as<GenerateError::Tag::DeleteFileError>();
        return fmt::format("deleting file '{}' failed", p.path);
      }
      case GenerateError::Tag::MarkdownError: {
        return fmt::format("parsing doc comment markdown");
      }
    }
    sus::unreachable();
  }
  static sus::Option<const sus::error::DynError&> source(
      const GenerateError& e) noexcept {
    switch (e) {
      case GenerateError::Tag::CopyFileError: {
        const auto& p = e.as<GenerateError::Tag::CopyFileError>();
        return sus::some(*p.source);
      }
      case GenerateError::Tag::DeleteFileError: {
        const auto& p = e.as<GenerateError::Tag::DeleteFileError>();
        return sus::some(*p.source);
      }
      case GenerateError::Tag::MarkdownError: {
        const auto& p = e.as<GenerateError::Tag::MarkdownError>();
        return sus::some(*p);
      }
    }
    sus::unreachable();
  }
};
