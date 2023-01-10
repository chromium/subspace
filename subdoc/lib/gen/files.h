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

#include <filesystem>
#include <fstream>
#include <sstream>

#include "subspace/iter/iterator.h"

namespace subdoc::gen {

inline sus::Option<std::ofstream> open_file_for_writing(
    std::filesystem::path path) noexcept {
  sus::Option<std::ofstream> out;

  std::ofstream file;
  file.open(path.c_str());
  if (file.is_open()) {
    out.insert(sus::move(file));
  } else {
    llvm::errs() << "Unable to open file " << path.string() << " for writing\n";
  }

  return out;
}

}  // namespace subdoc::gen
