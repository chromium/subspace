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

#include "subdoc/lib/clang_resource_dir.h"

#include <filesystem>
#include <fstream>

#include "subdoc/llvm.h"

namespace subdoc {

Option<std::string> ClangResourceDir::find_resource_dir(std::string_view tool) {
  if (auto it = cache_.find(tool); it != cache_.end()) {
    return sus::some(it->second);
  }

  std::filesystem::path tool_path(tool);
  std::string stem = tool_path.stem().string();
  if (!stem.starts_with("clang")) return sus::none();

  if (!std::filesystem::exists(tool_path)) {
    fmt::println(stderr, "WARNING: can't find clang compiler at '{}'", tool);
    return sus::none();
  }

  std::array<llvm::StringRef, 2> args = {tool, "-print-resource-dir"};
  if (stem.starts_with("clang-cl")) {
    args[1] = "/clang:-print-resource-dir";
  }

  llvm::SmallString<100> out;
  {
    std::error_code code =
        llvm::sys::fs::createTemporaryFile("stdout", "txt", out);
    if (code) {
      fmt::println(stderr, "WARNING: unable to make temp file: {}",
                   code.message());
      return sus::none();
    }
  }
  std::array<std::optional<llvm::StringRef>, 3> redirects = {std::nullopt, out,
                                                             std::nullopt};
  {
    i32 code =
        llvm::sys::ExecuteAndWait(tool, args, /*env=*/std::nullopt, redirects);
    if (code != 0) {
      fmt::println(
          stderr, "WARNING: failed to run clang compiler at '{}', exit code {}",
          tool, code);
      return sus::none();
    }
  }

  auto out_file = std::ifstream(out.c_str());
  std::string resource_dir;
  if (!std::getline(out_file, resource_dir)) {
    fmt::println(stderr,
                 "WARNING: 'clang -print-resource-dir' did not return anything "
                 "for clang compiler at '{}'",
                 tool);
    return sus::none();
  }

  auto trimmed_resource_dir = llvm::StringRef(resource_dir).trim();

  cache_.emplace(tool, std::string(trimmed_resource_dir));
  return sus::some(std::string(trimmed_resource_dir));
}

}  // namespace subdoc
