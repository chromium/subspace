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

#include <filesystem>
#include <memory>
#include <utility>

#include "subdoc/lib/clang_resource_dir.h"
#include "subdoc/lib/visit.h"
#include "sus/collections/compat_vector.h"
#include "sus/iter/iterator.h"

namespace subdoc {

sus::Result<Database, DiagnosticResults> run_test(
    std::string pretend_file_name, std::string content,
    sus::Slice<std::string> command_line_args,
    const RunOptions& options) noexcept {
  auto join_args = std::string();
  for (const std::string& a : command_line_args) {
    join_args += a + "\n";
  }

  auto err = std::string();
  auto comp_db = clang::tooling::FixedCompilationDatabase::loadFromBuffer(
      ".", join_args, err);
  if (!err.empty()) {
    llvm::errs() << "error making comp_db for tests: " << err << "\n";
    return sus::err(DiagnosticResults());
  }

  auto vfs = llvm::IntrusiveRefCntPtr(new llvm::vfs::InMemoryFileSystem());
  vfs->addFile(pretend_file_name, 0, llvm::MemoryBuffer::getMemBuffer(content));

  return run_files(*comp_db, Vec<std::string>(sus::move(pretend_file_name)),
                   std::move(vfs), options);
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

sus::Result<Database, DiagnosticResults> run_files(
    const clang::tooling::CompilationDatabase& comp_db, Vec<std::string> paths,
    llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> fs,
    const RunOptions& options) noexcept {
  // Clang DiagnoticsConsumer that prints out the full error and context, which
  // is what the default one does, but by making it we have a pointer from which
  // we can see if an error occured.
  auto diags = std::make_unique<DiagnosticTracker>(
      llvm::errs(), new clang::DiagnosticOptions());

  usize num_files = paths.len();
  auto tool = clang::tooling::ClangTool(
      comp_db, sus::move(paths).into_iter().collect<std::vector<std::string>>(),
      std::make_shared<clang::PCHContainerOperations>(), std::move(fs));
  tool.setDiagnosticConsumer(&*diags);

  ClangResourceDir resource_dir;

  auto adj = [&resource_dir](clang::tooling::CommandLineArguments args,
                             llvm::StringRef) {
    // Clang-cl doesn't understand this argument, but it may appear in the
    // command-line for MSVC in C++20 codebases (like subspace).
    std::erase(args, "/Zc:preprocessor");

    if (std::filesystem::path(args[0]).filename().string() == "cl.exe") {
      // TODO: https://github.com/llvm/llvm-project/issues/59689 clang-cl
      // requires this define in order to use offsetof() from constant
      // expressions, which subspace uses for the never-value optimization.
      args.push_back("/D_CRT_USE_BUILTIN_OFFSETOF");
#if CLANG_VERSION_MAJOR <= 16
      // TODO: https://github.com/llvm/llvm-project/issues/60347 the
      // source_location header on windows requires this to be defined. As
      // Clang's C++20 support includes consteval, let's define it.
      args.push_back("/D__cpp_consteval");
#endif
      // Turn off warnings in code, clang-cl finds a lot of warnings that we
      // don't get when building with regular clang.
      args.push_back("/w");
      // Subdoc sets a SUBDOC define when executing, allowing code to be
      // changed while generating docs if needed.
      args.push_back("/DSUBDOC");
    } else {
      // Subdoc sets a SUBDOC define when executing, allowing code to be
      // changed while generating docs if needed.
      args.push_back("-DSUBDOC");
    }

    // As of CMake version 3.28, the compile_commands.json includes "C++ 20
    // modules support". Unfortuantely, this means that CMake injects a command-
    // file into the compile command which is supposed to provide the mapping
    // from module name to BMI file. However the mapping file does not actually
    // exist outside of CMake doing compilation, so the compile_commands.json is
    // a bit of a lie, and running the command contained within it will fail if
    // subdoc tries to executate it verbatim, due to the mapping file not
    // existing.
    //
    // So we drop the `@foo.modmap` argument from each command line for now to
    // work around this problem introduced by CMake. Once code actually uses
    // modules however, the mappings provided by the modmap file will be
    // required to executate the compilation. And we will need to figure out
    // some other way to provide them or force CMake to write the modmap files
    // outside of compilation.
    //
    // See https://github.com/chromium/subspace/issues/437.
    std::erase_if(args, [](const auto& a) {
      return a.starts_with("@") && a.ends_with(".modmap");
    });

    if (Option<std::string> dir = resource_dir.find_resource_dir(args[0]);
        dir.is_some()) {
      args.push_back("-resource-dir");
      args.push_back(dir.as_value());
    }

    return std::move(args);
  };
  tool.appendArgumentsAdjuster(adj);

  auto cx = VisitCx(options);
  auto docs_db = Database(
      Comment(sus::clone(options.project_overview_text), "", DocAttributes()));
  auto visitor_factory = VisitorFactory(cx, docs_db, num_files);

  i32 run_value = sus::move(tool).run(&visitor_factory);
  if (run_value == 1) {
    return sus::err(sus::move(sus::move(diags)->results));
  }
  if (diags->getNumErrors() > 0u) {
    return sus::err(sus::move(sus::move(diags)->results));
  }

  auto r = docs_db.resolve_inherited_comments();
  if (r.is_err()) {
    // TODO: forward the message location into a diagnostic.
    llvm::errs() << sus::move(sus::move(r).unwrap_err()) << "\n";
    return sus::err(sus::move(sus::move(diags)->results));
  }

  return sus::ok(sus::move(docs_db));
}

}  // namespace subdoc
