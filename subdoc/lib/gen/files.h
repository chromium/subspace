// Copyright 2023 Google LLC
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

#include <fstream>

namespace subdoc::gen {

static bool write_file(std::string_view path,
                       std::string_view content) noexcept {
  std::ofstream file;
  file.open(path.data());
  if (!file.is_open()) {
    llvm::errs() << "Unable to open file " << path << " for writing";
    return false;
  }
  file << content;
  file.close();
  return true;
}

}  // namespace subdoc::gen
