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

#include <regex>

#include "subdoc/llvm.h"
#include "sus/fn/fn.h"
#include "sus/option/option.h"
#include "sus/prelude.h"

namespace subdoc {

/// Options to control the execution of the AST and documentation parsing and
/// collection.
struct RunOptions {
  RunOptions set_show_progress(bool p) && {
    show_progress = p;
    return sus::move(*this);
  }
  RunOptions set_include_path_patterns(std::regex r) && {
    include_path_patterns = sus::move(r);
    return sus::move(*this);
  }
  RunOptions set_exclude_path_patterns(std::regex r) && {
    exclude_path_patterns = sus::move(r);
    return sus::move(*this);
  }
  RunOptions set_on_tu_complete(
      sus::fn::FnBox<void(clang::ASTContext&)> fn) && {
    on_tu_complete = sus::some(sus::move(fn));
    return sus::move(*this);
  }

  /// Whether to print progress while collecting documentation from souce files.
  bool show_progress = true;
  /// Defaults to match everything.
  std::regex include_path_patterns = std::regex("");
  /// Defaults to match nothing.
  std::regex exclude_path_patterns;
  /// A closure to run after parsing each translation unit.
  ///
  /// Used for tests to observe the AST and test subdoc methods that act on
  /// things from the AST.
  sus::Option<sus::fn::FnBox<void(clang::ASTContext&)>> on_tu_complete;
  /// The overview markdown which will be applied as the doc comment to the
  /// global namespace/project overview page.
  sus::Vec<std::string> project_overview_markdown;
};

}  // namespace subdoc
