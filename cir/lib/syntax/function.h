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

#include "cir/lib/source_span.h"
#include "cir/lib/syntax/statements/let.h"
#include "cir/llvm.h"
#include "subspace/prelude.h"

namespace cir::syntax {

struct FunctionId {
  u32 num;
};

struct Function {
  FunctionId id;
  std::string name;
  SourceSpan span;
  Option<syntax::Let> return_var;

  const clang::FunctionDecl& decl;
};

}  // namespace cir::syntax

namespace cir {

inline std::string to_string(const syntax::FunctionId& id) noexcept {
  std::ostringstream s;
  s << id.num.primitive_value;
  return s.str();
}

inline std::string to_string(const syntax::Function& fn) noexcept {
  // TODO: Use fmt library (or add such to subspace).
  std::ostringstream s;
  s << "fn " << fn.name << "@" << cir::to_string(fn.id) << "(";
  // TODO: args
  s << ") ";
  if (fn.return_var.is_some()) {
    s << "-> " << cir::to_string(fn.return_var.unwrap_ref().type) << " ";
  }
  s << "{\n";
  // TODO: body
  s << "}";
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
