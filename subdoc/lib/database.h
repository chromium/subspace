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

#include "subdoc/lib/namespace.h"
#include "subdoc/lib/unique_symbol.h"
#include "subdoc/llvm.h"
#include "subspace/choice/choice.h"
#include "subspace/prelude.h"

namespace subdoc {

enum class AttachedType {
  Function,
  Class,
  Union,
};
// clang-format off
using Attached = sus::Choice<sus_choice_types(
  (AttachedType::Function, sus::Vec<Namespace>),
  (AttachedType::Class, sus::Vec<Namespace>),
  (AttachedType::Union, sus::Vec<Namespace>),
)>;
// clang-format on

struct Comment {
#if defined(__clang__) && !__has_feature(__cpp_aggregate_paren_init)
  Comment(std::string raw_text, clang::SourceRange source_range,
          Attached attached_to)
      : raw_text(raw_text),
        source_range(source_range),
        attached_to(sus::move(attached_to)) {}
#endif

  std::string raw_text;
  std::string begin_loc;

  Attached attached_to;
};

using NamedCommentMap = std::unordered_map<UniqueSymbol, Comment>;

struct Database {
  NamedCommentMap classes;
  NamedCommentMap unions;
  NamedCommentMap functions;
  NamedCommentMap deductions;
  NamedCommentMap ctors;
  NamedCommentMap dtors;
  NamedCommentMap conversions;
  NamedCommentMap methods;
};

static_assert(sus::mem::Move<Database>);

}  // namespace subdoc
