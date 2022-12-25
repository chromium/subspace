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

#include "cir/lib/syntax/type_reference.h"
#include "cir/llvm.h"
#include "subspace/prelude.h"
#include "cir/lib/source_span.h"

namespace cir::syntax {

struct Let {
  u32 name;
  TypeReference type;
  SourceSpan span;
};

}  // namespace cir::syntax
