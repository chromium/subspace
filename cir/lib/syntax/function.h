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

struct Function {
  u32 id;
  std::string name;
  SourceSpan span;
  Option<syntax::Let> return_var;

  const clang::FunctionDecl& decl;

  std::string to_string() const& noexcept {
    // TODO: Use fmt library (or add such to subspace).
    std::ostringstream s;
    s << "fn " << name << "@" << id.primitive_value << "(";
    // TODO: args
    s << ") ";
    if (return_var.is_some()) {
      s << "-> " << return_var.unwrap_ref().type.to_string() << " ";
    }
    s << "{\n";
    // TODO: body
    s << "}";
    return s.str();
  }
};

}  // namespace cir::syntax
