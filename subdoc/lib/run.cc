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

#include "subdoc/lib/run.h"

#include <memory>
#include <utility>

#include "subdoc/lib/visit.h"
#include "subspace/iter/iterator.h"

namespace subdoc {

sus::result::Result<Database, DiagnosticResults> run_test(
    std::string content, sus::Vec<std::string> args) noexcept {
  auto join_args = std::string();
  for (const auto& a : sus::move(args).into_iter()) {
    join_args += a + "\n";
  }

  auto err = std::string();
  auto comp_db = clang::tooling::FixedCompilationDatabase::loadFromBuffer(
      ".", join_args, err);
  if (!err.empty()) {
    llvm::errs() << "error making comp_db for tests: " << err << "\n";
    return sus::result::err(DiagnosticResults());
  }

  auto vfs = llvm::IntrusiveRefCntPtr(new llvm::vfs::InMemoryFileSystem());
  vfs->addFile("test.cc", 0, llvm::MemoryBuffer::getMemBuffer(content));

  return run_files(*comp_db, sus::vec("test.cc"), std::move(vfs));
}

struct DiagnosticTracker : public clang::TextDiagnosticPrinter {
  explicit DiagnosticTracker(clang::raw_ostream& os,
                             clang::DiagnosticOptions* diags)
      : clang::TextDiagnosticPrinter(os, diags) {}

  void HandleDiagnostic(clang::DiagnosticsEngine::Level level,
                        const clang::Diagnostic& diag) override {
    auto& source_manager = diag.getSourceManager();
    results.locations.push(diag.getLocation().printToString(source_manager));
    clang::TextDiagnosticPrinter::HandleDiagnostic(level, diag);
  }

  DiagnosticResults results;
};

sus::result::Result<Database, DiagnosticResults> run_files(
    const clang::tooling::CompilationDatabase& comp_db,
    sus::Vec<std::string> paths,
    llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> fs) noexcept {
  // Clang DiagnoticsConsumer that prints out the full error and context, which
  // is what the default one does, but by making it we have a pointer from which
  // we can see if an error occured.
  auto diags = std::make_unique<DiagnosticTracker>(
      llvm::errs(), new clang::DiagnosticOptions());

  // TODO: Implement this efficiently in subspace.
  auto to_std_vec = []<class T>(sus::Vec<T> v) {
    std::vector<T> s;
    s.reserve(size_t{v.len()});
    for (auto&& e : sus::move(v).into_iter()) s.push_back(sus::move(e));
    return s;
  };

  auto tool = clang::tooling::ClangTool(
      comp_db, to_std_vec(sus::move(paths)),
      std::make_shared<clang::PCHContainerOperations>(), std::move(fs));
  tool.setDiagnosticConsumer(&*diags);

  auto adj = [](clang::tooling::CommandLineArguments args, llvm::StringRef) {
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

  auto cx = VisitCx();
  auto docs_db = Database();
  auto visitor_factory = VisitorFactory(cx, docs_db);
  if (tool.run(&visitor_factory) == 1) {
    return sus::result::err(sus::move(sus::move(diags)->results));
  }
  if (diags->getNumErrors() > 0) {
    return sus::result::err(sus::move(sus::move(diags)->results));
  }
  return sus::result::ok(sus::move(docs_db));
}

}  // namespace subdoc
