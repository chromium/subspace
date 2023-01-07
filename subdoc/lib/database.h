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

#include <unordered_map>

#include "subdoc/llvm.h"
#include "subspace/prelude.h"

namespace subdoc {

struct Comment {
#if defined(__clang__) && !__has_feature(__cpp_aggregate_paren_init)
  Comment(std::string raw_text, clang::SourceRange source_range)
      : raw_text(raw_text), source_range(source_range) {}
#endif

  std::string raw_text;
  std::string begin_loc;
};

using NamedComment =
    std::unordered_map<std::string /* fully qualified name */, Comment>;

struct Database {
  NamedComment functions;
  NamedComment deductions;
  NamedComment ctors;
  NamedComment dtors;
  NamedComment conversions;
  NamedComment methods;
};

static_assert(sus::mem::Move<Database>);

}  // namespace subdoc
