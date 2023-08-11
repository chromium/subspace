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

#include "subdoc/lib/gen/generate_cpp_path.h"

#include "subdoc/lib/gen/files.h"
#include "sus/assertions/unreachable.h"

namespace subdoc::gen {

namespace {

sus::Vec<CppPathElement> generate_with_ancestors(
    std::string_view name, CppPathElementType self_type,
    const sus::Slice<const NamespaceElement*>& ancestors,
    const sus::Slice<const RecordElement*>& type_ancestors,
    const Options& options) noexcept {
  sus::Vec<CppPathElement> out;
  for (const NamespaceElement& ancestor : ancestors.iter().map(
           [](const NamespaceElement* e) -> const NamespaceElement& {
             return *e;
           })) {
    out.push(CppPathElement{
        .name =
            [&]() {
              switch (ancestor.namespace_name) {
                case Namespace::Tag::Global:
                  return sus::clone(options.project_name);
                case Namespace::Tag::Anonymous:
                  return std::string("(anonymous)");
                case Namespace::Tag::Named: return sus::clone(ancestor.name);
              }
              sus::unreachable();
            }(),
        .link_href = construct_html_file_path_for_namespace(
                         std::filesystem::path(), ancestor)
                         .string(),
        .type =
            [&]() {
              switch (ancestor.namespace_name) {
                case Namespace::Tag::Global: return CppPathProject;
                case Namespace::Tag::Anonymous: return CppPathNamespace;
                case Namespace::Tag::Named: return CppPathNamespace;
              }
              sus::unreachable();
            }(),
    });
  }
  for (const RecordElement& ancestor : type_ancestors.iter().map(
           [](const RecordElement* e) -> const RecordElement& { return *e; })) {
    out.push(CppPathElement{
        .name = sus::clone(ancestor.name),
        .link_href =
            construct_html_file_path(
                std::filesystem::path(), ancestor.namespace_path.as_slice(),
                ancestor.record_path.as_slice(), ancestor.name)
                .string(),
        .type = CppPathRecord,
    });
  }
  out.push(CppPathElement{
      .name = std::string(name),
      .link_href = "#",
      .type = self_type,
  });
  return out;
}

}  // namespace

sus::Vec<CppPathElement> generate_cpp_path_for_namespace(
    const NamespaceElement& element,
    const sus::Slice<const NamespaceElement*>& ancestors,
    const Options& options) noexcept {
  sus::Vec<CppPathElement> out;
  switch (element.namespace_name) {
    case Namespace::Tag::Global:
      out.push(CppPathElement{
          .name = sus::clone(options.project_name),
          .link_href = "#",
          .type = CppPathProject,
      });
      break;
    case Namespace::Tag::Anonymous:
      out.push(CppPathElement{
          .name = "(anonymous)",
          .link_href = "#",
          .type = CppPathNamespace,
      });
      break;
    case Namespace::Tag::Named: {
      out.extend(
          generate_with_ancestors(element.name, CppPathNamespace, ancestors,
                                  sus::Slice<const RecordElement*>(), options));
      break;
    }
  }
  return out;
}

sus::Vec<CppPathElement> generate_cpp_path_for_type(
    const TypeElement& element,
    const sus::Slice<const NamespaceElement*>& namespace_ancestors,
    const sus::Slice<const RecordElement*>& type_ancestors,
    const Options& options) noexcept {
  return generate_with_ancestors(element.name, CppPathRecord,
                                 namespace_ancestors, type_ancestors, options);
}

}  // namespace subdoc::gen
