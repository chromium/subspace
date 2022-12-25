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

#include "cir/lib/run.h"

#include <memory>
#include <utility>

#include "cir/lib/visit.h"
#include "subspace/iter/iterator.h"

using sus::result::Result;

namespace cir {

Result<Output, i32> run_test(std::string content,
                             sus::Vec<std::string> args) noexcept {
  auto join_args = std::string();
  for (const auto& a : sus::move(args).into_iter()) {
    join_args += a + "\n";
  }

  auto err = std::string();
  auto compdb = clang::tooling::FixedCompilationDatabase::loadFromBuffer(
      ".", join_args, err);
  if (!err.empty()) {
    llvm::outs() << "error making compdb for tests: " << err << "\n";
    return Result<Output, i32>::with_err(1);
  }

  auto vfs = llvm::IntrusiveRefCntPtr(new llvm::vfs::InMemoryFileSystem());
  vfs->addFile("test.cc", 0, llvm::MemoryBuffer::getMemBuffer(content));

  return run_files(*compdb, {"test.cc"}, std::move(vfs));
}

Result<Output, i32> run_files(
    const clang::tooling::CompilationDatabase& compdb,
    std::vector<std::string> paths,
    llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> fs) noexcept {
  auto tool = clang::tooling::ClangTool(
      compdb, paths, std::make_shared<clang::PCHContainerOperations>(),
      std::move(fs));

  auto adj = [](clang::tooling::CommandLineArguments args, llvm::StringRef Filename) {
    // Clang-cl doesn't understand this argument, but it may appear in the
    // command-line for MSVC in C++20 codebases (like subspace).
    std::erase(args, "/Zc:preprocessor");

    const std::string& tool = args[0];
    if (tool.find("cl.exe") != std::string::npos) {
      // TODO: https://github.com/llvm/llvm-project/issues/59689 clang-cl
      // requires this define in order to use offsetof() from constant
      // expressions, which subspace uses for the never-value optimization.
      args.push_back("/D_CRT_USE_BUILTIN_OFFSETOF");
    }

    return std::move(args);
  };
  tool.appendArgumentsAdjuster(adj);

  auto asts = std::vector<std::unique_ptr<clang::ASTUnit>>();
  if (i32 ret = tool.buildASTs(asts); ret != 0) {
    return Result<Output, i32>::with_err(ret);
  }

  for (auto& ast : asts) {
    for (auto it = ast->top_level_begin(); it != ast->top_level_end(); it++) {
      cir::visit_decl(**it);
    }
  }
  return Result<Output, i32>::with(Output());
}

}  // namespace cir
