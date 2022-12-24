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
#include <vector>

#pragma warning(push)
#pragma warning(disable : 4624)
#include "llvm/Support/VirtualFileSystem.h"
#pragma warning(pop)

namespace cir::tool {

int run_test(std::string content);

int run_files(std::vector<std::string> paths, const llvm::vfs::FileSystem& fs);

}  // namespace cir::tool
