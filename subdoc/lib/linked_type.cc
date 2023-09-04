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

#include "subdoc/lib/database.h"

namespace subdoc {

LinkedType LinkedType::with_type(Type t, const Database& db) noexcept {
  sus::Vec<sus::Option<TypeRef>> refs = db.collect_type_element_refs(t);
  return LinkedType(CONSTRUCT, sus::move(t), sus::move(refs));
}

}  // namespace subdoc
