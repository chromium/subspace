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
#include "sus/boxed/box.h"
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
      sus::Box<sus::fn::DynFn<void(clang::ASTContext&, clang::Preprocessor&)>>
          fn) && {
    on_tu_complete = sus::some(sus::move(fn));
    return sus::move(*this);
  }
  RunOptions set_macro_prefixes(Vec<std::string> prefixes) && {
    macro_prefixes = sus::move(prefixes);
    return sus::move(*this);
  }
  RunOptions set_generate_source_links(bool gen) && {
    generate_source_links = gen;
    return sus::move(*this);
  }
  RunOptions set_remove_path_prefix(Option<std::string> prefix) && {
    remove_path_prefix = sus::move(prefix);
    return sus::move(*this);
  }
  RunOptions set_add_path_prefix(Option<std::string> prefix) && {
    add_path_prefix = sus::move(prefix);
    return sus::move(*this);
  }
  RunOptions set_source_line_prefix(Option<std::string> prefix) && {
    source_line_prefix = sus::move(prefix);
    return sus::move(*this);
  }

  /// Whether to print progress while collecting documentation from souce files.
  bool show_progress = true;
  /// Defaults to match everything.
  std::regex include_path_patterns = std::regex("");
  /// Defaults to match nothing.
  std::regex exclude_path_patterns;
  /// Prefixes of macros to be included in docs.
  Vec<std::string> macro_prefixes;
  /// A closure to run after parsing each translation unit.
  ///
  /// Used for tests to observe the AST and test subdoc methods that act on
  /// things from the AST.
  Option<
      sus::Box<sus::fn::DynFn<void(clang::ASTContext&, clang::Preprocessor&)>>>
      on_tu_complete;
  /// The overview markdown which will be applied as the doc comment to the
  /// global namespace/project overview page. This is the raw markdown text, not
  /// parsed to html yet.
  std::string project_overview_text;
  /// Whether to generate links to source code.
  bool generate_source_links = true;
  /// A prefix to remove from all paths in source links.
  Option<std::string> remove_path_prefix;
  /// A prefix to add to all paths in source links, after removing the prefix
  /// specified by `remove_path_prefix`.
  Option<std::string> add_path_prefix;
  /// A prefix to add to the source code line number html fragment. Github uses
  /// an `L` as its prefix.
  Option<std::string> source_line_prefix;
};

}  // namespace subdoc
