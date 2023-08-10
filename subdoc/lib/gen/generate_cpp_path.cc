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
    std::string_view name, const sus::Slice<const NamespaceElement*>& ancestors,
    const sus::Slice<const TypeElement*>& type_ancestors) noexcept {
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
                  // TODO: Project name in options.
                  return std::string("PROJECT NAME");
                case Namespace::Tag::Anonymous:
                  return std::string("(anonymous)");
                case Namespace::Tag::Named: return sus::clone(ancestor.name);
              }
              // SAFETY: No default or fallthrough from the switch above
              // so all cases return.
              sus::unreachable_unchecked(unsafe_fn);
            }(),
        .link_href = construct_html_file_path_for_namespace(
                         std::filesystem::path(), ancestor)
                         .string(),
    });
  }
  for (const TypeElement& ancestor : type_ancestors.iter().map(
           [](const TypeElement* e) -> const TypeElement& { return *e; })) {
    out.push(CppPathElement{
        .name = sus::clone(ancestor.name),
        .link_href =
            construct_html_file_path(
                std::filesystem::path(), ancestor.namespace_path.as_slice(),
                ancestor.record_path.as_slice(), ancestor.name)
                .string(),
    });
  }
  out.push(CppPathElement{
      .name = std::string(name),
      .link_href = "#",
  });
  return out;
}

}  // namespace

sus::Vec<CppPathElement> generate_cpp_path_for_namespace(
    const NamespaceElement& element,
    const sus::Slice<const NamespaceElement*>& ancestors) noexcept {
  sus::Vec<CppPathElement> out;
  switch (element.namespace_name) {
    case Namespace::Tag::Global:
      out.push(CppPathElement{
          // TODO: Project name in options.
          .name = "PROJECT NAME: Subspace",
          .link_href = "#",
      });
      break;
    case Namespace::Tag::Anonymous:
      out.push(CppPathElement{
          .name = "(anonymous)",
          .link_href = "#",
      });
      break;
    case Namespace::Tag::Named: {
      out.extend(generate_with_ancestors(element.name, ancestors,
                                         sus::Slice<const TypeElement*>()));
      break;
    }
  }
  return out;
}

sus::Vec<CppPathElement> generate_cpp_path_for_type(
    const TypeElement& element,
    const sus::Slice<const NamespaceElement*>& namespace_ancestors,
    const sus::Slice<const TypeElement*>& type_ancestors) noexcept {
  return generate_with_ancestors(element.name, namespace_ancestors,
                                 type_ancestors);
}

}  // namespace subdoc::gen
