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
#include "subdoc/lib/gen/options.h"
#include "sus/prelude.h"

namespace subdoc::gen {

enum CppPathElementType {
  CppPathProject,
  CppPathNamespace,
  CppPathRecord,
  CppPathConcept,
  CppPathFunction,
  CppPathMacro,
};

struct CppPathElement {
  std::string name;
  std::string link_href;
  CppPathElementType type;
  f32 search_weight;
};

Vec<CppPathElement> generate_cpp_path_for_namespace(
    const NamespaceElement& element,
    sus::Slice<const NamespaceElement*> ancestors,
    const Options& options) noexcept;

Vec<CppPathElement> generate_cpp_path_for_type(
    const TypeElement& element,
    sus::Slice<const NamespaceElement*> namespace_ancestors,
    sus::Slice<const RecordElement*> type_ancestors,
    const Options& options) noexcept;

Vec<CppPathElement> generate_cpp_path_for_concept(
    const ConceptElement& element,
    sus::Slice<const NamespaceElement*> namespace_ancestors,
    const Options& options) noexcept;

Vec<CppPathElement> generate_cpp_path_for_function(
    const FunctionElement& element,
    sus::Slice<const NamespaceElement*> namespace_ancestors,
    const Options& options) noexcept;

Vec<CppPathElement> generate_cpp_path_for_macro(
    const MacroElement& element,
    sus::Slice<const NamespaceElement*> namespace_ancestors,
    const Options& options) noexcept;

}  // namespace subdoc::gen
