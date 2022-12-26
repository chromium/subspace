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

#include "cir/lib/output.h"
#include "cir/llvm.h"
#include "subspace/containers/vec.h"
#include "subspace/option/option.h"
#include "subspace/prelude.h"

namespace cir {

sus::Option<Output> run_test(std::string content,
                             sus::Vec<std::string> args) noexcept;

sus::Option<Output> run_file(
    const clang::tooling::CompilationDatabase& compdb, std::string path,
    llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> fs) noexcept;

}  // namespace cir
