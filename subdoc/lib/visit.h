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

#pragma once

#include <string>
#include <unordered_set>

#include "subdoc/lib/database.h"
#include "subdoc/lib/run_options.h"
#include "subdoc/llvm.h"
#include "subspace/option/option.h"
#include "subspace/prelude.h"

namespace subdoc {

struct VisitedLocation {
  std::string location_as_string;

  friend bool operator==(const VisitedLocation&,
                         const VisitedLocation&) = default;

  struct Hash {
    size_t operator()(const VisitedLocation& v) const {
      return std::hash<std::string>()(v.location_as_string);
    }
  };
};

struct VisitedPath {
  bool included = true;
};

struct VisitCx {
 public:
  const RunOptions& options;
  std::unordered_set<VisitedLocation, VisitedLocation::Hash> visited_locations_;
  std::map<std::string, VisitedPath, std::less<>> visited_paths_;
};

struct LineStats {
  usize cur_file = 1_usize;
  usize num_files;
  usize last_line_len = 0_usize;
  std::string cur_file_name;
};

struct VisitorFactory : public clang::tooling::FrontendActionFactory {
 public:
  explicit VisitorFactory(VisitCx& cx, Database& docs_db, usize num_files)
      : cx(cx), docs_db(docs_db) {
    line_stats.num_files = num_files;
  }

  // Returns a VisitorAction.
  std::unique_ptr<clang::FrontendAction> create() noexcept override;

  VisitCx& cx;
  Database& docs_db;
  LineStats line_stats;
};

struct VisitorAction : public clang::ASTFrontendAction {
  explicit VisitorAction(VisitCx& cx, Database& docs_db, LineStats& line_stats)
      : cx(cx), docs_db(docs_db), line_stats(line_stats) {}

  bool PrepareToExecuteAction(clang::CompilerInstance& inst) noexcept override;

  // Returns a Visitor.
  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
      clang::CompilerInstance& compiler,
      llvm::StringRef file) noexcept override;

  VisitCx& cx;
  Database& docs_db;
  LineStats& line_stats;
};

}  // namespace subdoc
