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
#include "subspace/prelude.h"

#pragma warning(push)
#pragma warning(disable : 4624)
#pragma warning(disable : 4291)
#include "clang/Frontend/ASTUnit.h"
#include "clang/Serialization/PCHContainerOperations.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/MemoryBuffer.h"
#pragma warning(pop)

namespace cir {

int run_test(std::string content, std::vector<std::string> args) noexcept {
  std::string join_args;
  for (const auto& a : args) {
    join_args += a + "\n";
  }

  std::string err;
  auto compdb = clang::tooling::FixedCompilationDatabase::loadFromBuffer(
      ".", join_args, err);
  if (!err.empty()) {
    llvm::outs() << "error making compdb for tests: " << err << "\n";
    return 1;
  }

  auto vfs = llvm::IntrusiveRefCntPtr(new llvm::vfs::InMemoryFileSystem());
  vfs->addFile("test.cc", 0, llvm::MemoryBuffer::getMemBuffer(content));

  return run_files(*compdb, {"test.cc"}, std::move(vfs));
}

int run_files(const clang::tooling::CompilationDatabase& compdb,
              std::vector<std::string> paths,
              llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> fs) noexcept {
  auto tool = clang::tooling::ClangTool(
      compdb, paths, std::make_shared<clang::PCHContainerOperations>(),
      std::move(fs));

  std::vector<std::unique_ptr<clang::ASTUnit>> asts;
  int ret = tool.buildASTs(asts);
  if (ret == 0) {
    for (auto& ast : asts) {
      for (auto it = ast->top_level_begin(); it != ast->top_level_end(); it++) {
        cir::visit_decl(**it);
      }
    }
  }
  return ret;
}

}  // namespace cir
