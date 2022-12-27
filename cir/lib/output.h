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

#include <sstream>

#include "cir/lib/syntax/function.h"
#include "subspace/containers/vec.h"
#include "subspace/iter/iterator.h"

namespace cir {

struct Output {
  std::unordered_map<syntax::FunctionId, syntax::Function> functions;
};

inline std::string to_string(const Output& out) noexcept {
  // TODO: Use fmt library (or add such to subspace).
  std::ostringstream s;
  bool saw_fn = false;
  for (const auto& [id, f] : out.functions) {
    if (saw_fn) s << "\n\n";
    saw_fn = true;
    s << cir::to_string(f);
  }
  return s.str();
}

}  // namespace cir
