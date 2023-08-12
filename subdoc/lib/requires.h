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

#include "subdoc/llvm.h"
#include "sus/choice/choice.h"
#include "sus/prelude.h"

namespace subdoc {

struct RequiresConceptConstraint {
  // TODO: Make this a reference to a Concept in the Database when it's present
  // there.
  std::string concept_name;
  // TODO: These can be types that are in the Database, so they could be linked?
  sus::Vec<std::string> args;
};

enum class RequiresConstraintTag {
  Concept,
  Text,
};
// clang-format off
using RequiresConstraint = sus::choice_type::Choice<sus_choice_types(
    (RequiresConstraintTag::Concept, RequiresConceptConstraint),
    (RequiresConstraintTag::Text, std::string),
)>;
// clang-format on

struct RequiresConstraints {
  sus::Vec<RequiresConstraint> list;
};

void requires_constraints_add_expr(RequiresConstraints& constaints,
                                   const clang::ASTContext& context,
                                   const clang::Expr* e) noexcept;

}  // namespace subdoc
