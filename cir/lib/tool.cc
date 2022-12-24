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

#include "cir/lib/tool.h"

#include <memory>
#include <utility>

#include "subspace/prelude.h"

#pragma warning(push)
#pragma warning(disable : 4624)
#pragma warning(disable : 4291)
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/MemoryBuffer.h"
#pragma warning(pop)

namespace cir::tool {

int run_test(std::string content) {
  auto vfs = llvm::vfs::InMemoryFileSystem();
  vfs.addFile("test.cc", 0, llvm::MemoryBuffer::getMemBuffer(content));

  return run_files({"test.cc"}, vfs);
}

int run_files(std::vector<std::string> paths, const llvm::vfs::FileSystem& fs) {
  llvm::outs() << "hi\n";
  return 1;
}

}  // namespace cir::tool
