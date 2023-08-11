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

#include <string>

#include "subdoc/lib/database.h"
#include "sus/prelude.h"

namespace subdoc::gen {

enum CppPathElementType {
    CppPathNamespace,
    CppPathRecord,
};

struct CppPathElement {
  std::string name;
  std::string link_href;
  CppPathElementType type;
};

sus::Vec<CppPathElement> generate_cpp_path_for_namespace(
    const NamespaceElement& element,
    const sus::Slice<const NamespaceElement*>& ancestors) noexcept;

sus::Vec<CppPathElement> generate_cpp_path_for_type(
    const TypeElement& element,
    const sus::Slice<const NamespaceElement*>& namespace_ancestors,
    const sus::Slice<const RecordElement*>& type_ancestors) noexcept;

}  // namespace subdoc::gen
