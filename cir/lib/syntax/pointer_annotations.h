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

namespace cir::syntax {

struct PointerAnnotations {
  bool is_const;
  bool is_nullable;
  // TODO: lifetimes.
};

}  // namespace cir::syntax

namespace cir {

inline std::string to_string(const syntax::PointerAnnotations& anno) noexcept {
  // TODO: Use fmt library (or add such to subspace).
  std::ostringstream s;
  bool space = false;
  if (anno.is_const) {
    if (space) s << " ";
    s << "const";
    space = true;
  }
  if (anno.is_nullable) {
    if (space) s << " ";
    s << "nullable";
    space = true;
  }
  return s.str();
}

}  // namespace cir
