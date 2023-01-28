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

#include <regex>

#include "subspace/prelude.h"

namespace subdoc {

struct RunOptions {
  RunOptions set_show_progress(bool p) && {
    show_progress = p;
    return sus::move(*this);
  }
  RunOptions set_include_path_patterns(std::regex r) && {
    include_path_patterns = sus::move(r);
    return sus::move(*this);
  }
  RunOptions set_exclude_path_patterns(std::regex r) && {
    exclude_path_patterns = sus::move(r);
    return sus::move(*this);
  }  

  // Whether to print progress while collecting documentation from souce files.
  bool show_progress = true;
  // Defaults to match everything.
  std::regex include_path_patterns = std::regex("");
  // Defaults to match nothing.
  std::regex exclude_path_patterns;
};

}  // namespace subdoc
