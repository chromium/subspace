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

#include "subdoc/lib/gen/generate.h"

#include <filesystem>

#include "subdoc/lib/gen/generate_namespace.h"
#include "sus/error/compat_error.h"

namespace subdoc::gen {

sus::result::Result<void, GenerateError> generate(const Database& db,
                                                  const Options& options) {
  if (std::filesystem::exists(options.output_root)) {
    if (std::filesystem::is_directory(options.output_root)) {
      for (auto it = std::filesystem::directory_iterator(options.output_root);
           it != std::filesystem::directory_iterator(); ++it) {
        if (!(it->is_directory() &&
              it->path().filename().string().starts_with("."))) {
          std::error_code ec;
          std::filesystem::remove(*it, ec);
          if (ec) {
            return sus::err(
                GenerateError::with<GenerateError::Tag::DeleteFileError>(
                    GenerateFileError{
                        .path = it->path().filename().string(),
                        .source = sus::move_into(ec),
                    }));
          }
        }
      }
    } else {
      std::error_code ec;
      std::filesystem::remove(options.output_root, ec);
      if (ec) {
        return sus::err(
            GenerateError::with<GenerateError::Tag::DeleteFileError>(
                GenerateFileError{
                    .path = options.output_root.string(),
                    .source = sus::move_into(ec),
                }));
      }
    }
  }
  if (auto result = generate_namespace(db, db.global, sus::vec(), options);
      result.is_err()) {
    return sus::err(GenerateError::with<GenerateError::Tag::MarkdownError>(
        sus::into(sus::move(result).unwrap_err())));
  }

  for (const std::string& s : options.copy_files) {
    if (!std::filesystem::exists(s)) {
      llvm::errs() << "Skipping copy of '" << s << "'. File not found.\n";
    } else {
      std::error_code ec;
      std::filesystem::copy(s, options.output_root, ec);
      if (ec) {
        return sus::err(GenerateError::with<GenerateError::Tag::CopyFileError>(
            GenerateFileError{
                .path = sus::clone(s),
                .source = sus::move_into(ec),
            }));
      }
    }
  }

  return sus::ok();
}

}  // namespace subdoc::gen
