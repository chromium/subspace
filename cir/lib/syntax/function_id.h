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
#include <string>

#include "cir/llvm.h"
#include "subspace/prelude.h"

namespace cir::syntax {

struct FunctionId {
  u32 num;
};

}  // namespace cir::syntax

namespace cir {

struct Output;

inline std::string to_string(const syntax::FunctionId& id,
                             const Output&) noexcept {
  std::ostringstream s;
  s << id.num.primitive_value;
  return s.str();
}

}  // namespace cir

namespace std {
template <>
struct hash<cir::syntax::FunctionId> {
  auto operator()(const cir::syntax::FunctionId& i) const {
    return std::hash<decltype(i.num)>()(i.num);
  }
};
template <>
struct equal_to<cir::syntax::FunctionId> {
  auto operator()(const cir::syntax::FunctionId& l,
                  const cir::syntax::FunctionId& r) const {
    return l.num == r.num;
  }
};

}  // namespace std
