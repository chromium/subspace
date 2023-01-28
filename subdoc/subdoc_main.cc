// Copyright 2022 Google LLC
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

#include <filesystem>
#include <unordered_set>

#include "lib/gen/generate.h"
#include "lib/run.h"
#include "subdoc/llvm.h"
#include "subspace/prelude.h"

int main(int argc, const char** argv) {
  llvm::InitLLVM init(argc, argv);
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();

  llvm::cl::OptionCategory option_category("SubDoc options");

  llvm::cl::list<std::string> option_css(
      "css",
      llvm::cl::desc(
          "A list of CSS files to include in the generated HTML header.\n"
          "\n"
          "When rendering the HTML, a <link> tag will be added\n"
          "with each path to a CSS file that is specified. For\n"
          "example: \"../main.css,other.css,/top.css\"."),
      llvm::cl::cat(option_category));

  llvm::cl::list<std::string> option_copy_files(
      "copy-files",
      llvm::cl::desc("A list (comma separated) of files to be copied into the "
                     "output directory."),
      llvm::cl::cat(option_category));

  llvm::Expected<clang::tooling::CommonOptionsParser> options_parser =
      clang::tooling::CommonOptionsParser::create(argc, argv, option_category,
                                                  llvm::cl::ZeroOrMore);
  if (!options_parser) {
    llvm::WithColor::error() << llvm::toString(options_parser.takeError());
    return 1;
  }

  auto& comp_db = options_parser->getCompilations();

  std::vector<std::string> paths = options_parser->getSourcePathList();
  if (paths.empty()) {
    llvm::errs() << "Error: no input files specified.\n";
    llvm::cl::PrintHelpMessage(/*Hidden=*/false, /*Categorized=*/true);
    return 1;
  }

  // These are the files available to run the tool against.
  std::vector<std::string> comp_db_files = comp_db.getAllFiles();
  // These are the files we choose to run the tool against. We use fuzzy
  // matching on the input arguments to pick them.
  sus::Vec<std::string> run_against_files;

  for (const std::string& input_path : paths) {
    bool found = false;
    for (const std::string& s : comp_db_files) {
      if (s.find(input_path) == std::string::npos) {
        continue;
      }
      run_against_files.push(s);
      found = true;
    }
    if (!found) {
      llvm::errs() << "Unknown file, not in compiledb: " << input_path << "\n";
      return 1;
    }
  }

  auto fs = llvm::vfs::getRealFileSystem();

  auto result =
      subdoc::run_files(comp_db, sus::move(run_against_files), sus::move(fs));
  if (result.is_err()) {
    llvm::errs() << "Error occurred. Exiting.\n";
    return 1;
  }

  subdoc::Database docs_db = sus::move(result).unwrap();

  auto options = subdoc::gen::Options{
      // TODO: Make this configurable.
      .output_root = "out/docs",
  };
  if (option_css.empty() && option_copy_files.empty()) {
    // Defaults to pull the test stylesheet, assuming subdoc is being run from the
    // source root directory.
    options.copy_files.push("subdoc/gen_tests/subdoc-test-style.css");
    options.stylesheets.push("subdoc-test-style.css");
  } else {
    for (std::string s : sus::move(option_css))
      options.stylesheets.push(sus::move(s));
    for (std::string s : sus::move(option_copy_files))
      options.copy_files.push(sus::move(s));
  }

  subdoc::gen::generate(docs_db, options);
  return 0;
}
