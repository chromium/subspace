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

#include "subdoc/lib/type.h"
#include "sus/choice/choice.h"
#include "sus/collections/vec.h"
#include "sus/option/option.h"

namespace subdoc {

struct ConceptElement;
struct Database;
struct RecordElement;

enum class TypeRefTag {
  Concept,
  Record,
};
/// A reference to a type in the `Database`.
using TypeRef =
    sus::Choice<sus_choice_types((TypeRefTag::Concept, const ConceptElement&),
                                 (TypeRefTag::Record, const RecordElement&))>;

/// A fully described and printable type, with all its sub-types linked to the
/// database when they exist there and are not marked hidden.
struct LinkedType {
  static LinkedType with_type(Type t, const Database& db) noexcept;

  Type type;
  /// References into the database for every type that makes up `type`.
  sus::Vec<sus::Option<TypeRef>> type_element_refs;

 private:
 enum Construct { CONSTRUCT };
  LinkedType(Construct, Type t, sus::Vec<sus::Option<TypeRef>> v)
      : type(sus::move(t)), type_element_refs(sus::move(v)) {}
};

}  // namespace subdoc
