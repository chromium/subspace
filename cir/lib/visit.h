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

#include "cir/lib/output.h"
#include "cir/llvm.h"
#include "subspace/prelude.h"

namespace cir {

struct VisitCtx {
 public:
  u32 make_function_id() noexcept {
    u32 id = next_function_id;
    next_function_id += 1u;
    return id;
  }

  u32 make_local_varname() noexcept {
    u32 id = next_local_varname;
    next_local_varname += 1u;
    return id;
  }

  // The back of this Vec is the function whose body is being parsed.
  Vec<syntax::Function> in_functions = Vec<syntax::Function>::with_default();

private:
  u32 next_function_id = 0u;
  u32 next_local_varname = 0u;
};

void visit_decl(VisitCtx& ctx, clang::Decl& decl, Output& output) noexcept;

}  // namespace cir
