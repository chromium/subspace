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

#include "subdoc/lib/database.h"
#include "subdoc/llvm.h"
#include "subspace/containers/vec.h"
#include "subspace/prelude.h"
#include "subdoc/lib/run_options.h"
#include "subspace/result/result.h"

namespace subdoc {

struct DiagnosticResults {
  sus::Vec<std::string> locations;
};
static_assert(sus::mem::Move<DiagnosticResults>);

sus::Result<Database, DiagnosticResults> run_test(
    std::string content, sus::Slice<const std::string> command_line_args,
    const RunOptions& options) noexcept;

sus::Result<Database, DiagnosticResults> run_files(
    const clang::tooling::CompilationDatabase& compdb,
    sus::Vec<std::string> paths,
    llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> fs,
    const RunOptions& options) noexcept;

}  // namespace subdoc
