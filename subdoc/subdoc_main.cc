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
      .output_root = "docs",
      .default_stylesheet_path = "../subdoc/subdoc-default-style.css",
  };
  subdoc::gen::generate(docs_db, options);
  return 0;
}
