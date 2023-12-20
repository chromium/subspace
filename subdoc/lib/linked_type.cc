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

#include "subdoc/lib/linked_type.h"

#include <sstream>

#include "subdoc/lib/database.h"

namespace subdoc {

LinkedType LinkedType::with_type(Type t, const Database& db) noexcept {
  Vec<Option<TypeRef>> refs = db.collect_type_element_refs(t);
  return LinkedType(CONSTRUCT, sus::move(t), sus::move(refs));
}

LinkedConcept LinkedConcept::with_concept(sus::Slice<Namespace> namespace_path,
                                          std::string name,
                                          const Database& db) noexcept {
  Option<FoundName> found =
      db.find_name_in_namespace_path(namespace_path, name);
  if (found.is_some() && *found == FoundName::Tag::Concept) {
    return LinkedConcept{
        ConceptRefOrName::with<ConceptRefOrName::Tag::Ref>(
            found->as<FoundName::Tag::Concept>()),
    };
  } else {
    std::ostringstream s;
    s << namespace_path_to_string(namespace_path.iter());
    s << sus::move(name);
    return LinkedConcept{
        ConceptRefOrName::with<ConceptRefOrName::Tag::Name>(sus::move(s).str()),
    };
  }
}

LinkedFunction LinkedFunction::with_function(
    sus::Slice<Namespace> namespace_path, std::string name,
    const Database& db) noexcept {
  // TODO: The alias has to pick an overload set to link to, which one? Should
  // all function overload sets be on one html page?
  Option<FoundName> found =
      db.find_name_in_namespace_path(namespace_path, name);
  if (found.is_some() && *found == FoundName::Tag::Function) {
    return LinkedFunction{
        FunctionRefOrName::with<FunctionRefOrName::Tag::Ref>(
            found->as<FoundName::Tag::Function>()),
    };
  } else {
    std::ostringstream s;
    s << namespace_path_to_string(namespace_path.iter());
    s << sus::move(name);
    return LinkedFunction{
        FunctionRefOrName::with<FunctionRefOrName::Tag::Name>(
            sus::move(s).str()),
    };
  }
}

LinkedVariable LinkedVariable::with_variable(
    sus::Slice<Namespace> namespace_path, std::string name,
    const Database& db) noexcept {
  // TODO: The alias has to pick an overload set to link to, which one? Should
  // all function overload sets be on one html page?
  Option<FoundName> found =
      db.find_name_in_namespace_path(namespace_path, name);
  if (found.is_some() && *found == FoundName::Tag::Field) {
    return LinkedVariable{
        VariableRefOrName::with<VariableRefOrName::Tag::Ref>(
            found->as<FoundName::Tag::Field>()),
    };
  } else {
    std::ostringstream s;
    s << namespace_path_to_string(namespace_path.iter());
    s << sus::move(name);
    return LinkedVariable{
        VariableRefOrName::with<VariableRefOrName::Tag::Name>(
            sus::move(s).str()),
    };
  }
}

}  // namespace subdoc
