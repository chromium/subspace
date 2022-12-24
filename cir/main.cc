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

#include <unordered_set>

#include "lib/tool.h"
#include "subspace/prelude.h"

#pragma warning(push)
#pragma warning(disable : 4624)
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/CompilationDatabase.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/WithColor.h"
#pragma warning(pop)

int main(int argc, const char** argv) {
  llvm::InitLLVM init(argc, argv);
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();

  llvm::cl::OptionCategory option_category("CIR options");
  llvm::Expected<clang::tooling::CommonOptionsParser> options_parser =
      clang::tooling::CommonOptionsParser::create(argc, argv, option_category,
                                                  llvm::cl::ZeroOrMore);
  if (!options_parser) {
    llvm::WithColor::error() << llvm::toString(options_parser.takeError());
    return 1;
  }

  auto& compdb = options_parser->getCompilations();

  std::vector<std::string> run_against_files;

  for (const auto& input_path : options_parser->getSourcePathList()) {
    bool found = false;
    for (auto s : compdb.getAllFiles()) {
      if (s.find(input_path) == std::string::npos) {
        continue;
      }
      llvm::outs() << s << " :\n";
      run_against_files.push_back(s);
      found = true;
    }
    if (!found) {
      llvm::outs() << "Unknown file, not in compiledb: " << input_path << "\n";
    }
  }

  auto fs = llvm::vfs::getRealFileSystem();
  return cir::tool::run_files(run_against_files, *fs);
}
