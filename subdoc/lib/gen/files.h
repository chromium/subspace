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

#include <filesystem>
#include <fstream>
#include <sstream>

#include "subdoc/lib/database.h"
#include "sus/collections/slice.h"
#include "sus/iter/iterator.h"
#include "sus/ops/range.h"

namespace subdoc::gen {

inline sus::Option<std::ofstream> open_file_for_writing(
    std::filesystem::path path) noexcept {
  std::ofstream file;
  file.open(path, std::ios::binary);
  if (file.is_open()) {
    return sus::some(sus::move(file));
  } else {
    llvm::errs() << "Unable to open file " << path.string() << " for writing\n";
    return sus::none();
  }
}

inline std::filesystem::path construct_html_file_path(
    std::filesystem::path root, sus::Slice<Namespace> namespace_path,
    sus::Slice<std::string> record_path, std::string_view name) noexcept {
  std::filesystem::path p = sus::move(root);

  std::ostringstream fname;
  for (const Namespace& n : namespace_path.iter().rev()) {
    switch (n) {
      case Namespace::Tag::Global: break;
      case Namespace::Tag::Anonymous:
        fname << "anonymous";
        fname << "-";
        break;
      case Namespace::Tag::Named:
        fname << n.as<Namespace::Tag::Named>();
        fname << "-";
        break;
    }
  }
  for (std::string_view n : record_path.iter().rev()) {
    fname << n << "-";
  }
  fname << name;
  fname << ".html";
  p.append(sus::move(fname).str());
  return p;
}

inline std::filesystem::path construct_html_namespace_file_path(
    std::filesystem::path root, sus::Slice<Namespace> namespace_path) noexcept {
  std::filesystem::path p = sus::move(root);

  std::ostringstream fname;
  // The namespace path includes the namespace element itself, so drop
  // that one.
  for (const Namespace& n : namespace_path.iter().skip(1u).rev()) {
    switch (n) {
      case Namespace::Tag::Global: break;
      case Namespace::Tag::Anonymous:
        fname << "anonymous";
        fname << "-";
        break;
      case Namespace::Tag::Named:
        fname << n.as<Namespace::Tag::Named>();
        fname << "-";
        break;
    }
  }

  std::string name = [&]() {
    switch (namespace_path[0u]) {
      case Namespace::Tag::Global:
        // We're generating the global namespace, which will go in `index.html`.
        return std::string("index");
      case Namespace::Tag::Anonymous: return std::string("anonymous");
      case Namespace::Tag::Named:
        // Otherwise, use `namespace.${name}` for the file name of the namespace,
        // which prevents collisions with other html files that have the same name
        // as a top level namespace, such as a top level namespace named "index".
        return fmt::format("namespace.{}",
                           namespace_path[0u].as<Namespace::Tag::Named>());
    }
    sus_unreachable();
  }();

  fname << name;
  fname << ".html";
  p.append(sus::move(fname).str());
  return p;
}

inline std::filesystem::path construct_html_file_path_for_concept(
    std::filesystem::path root, const ConceptElement& element) noexcept {
  return construct_html_file_path(sus::move(root), element.namespace_path,
                                  sus::Slice<std::string>(), element.name);
}

inline std::string construct_html_url_for_concept(
    const ConceptElement& element) noexcept {
  return construct_html_file_path_for_concept(std::filesystem::path(), element)
      .string();
}

inline std::filesystem::path construct_html_file_path_for_type(
    std::filesystem::path root, const TypeElement& element) noexcept {
  return construct_html_file_path(sus::move(root), element.namespace_path,
                                  element.record_path, element.name);
}

inline std::string construct_html_url_for_type(
    const TypeElement& element) noexcept {
  return construct_html_file_path_for_type(std::filesystem::path(), element)
      .string();
}

inline std::string construct_html_url_for_field(
    const FieldElement& element) noexcept {
  std::ostringstream p;
  if (element.record_path.is_empty()) {
    // A variable.
    p << construct_html_namespace_file_path(std::filesystem::path(),
                                            element.namespace_path)
             .string();
    p << "#variable.";
  } else {
    // A class field.
    p << construct_html_file_path(
             std::filesystem::path(), element.namespace_path,
             element.record_path[sus::ops::range(
                 0_usize, element.record_path.len() - 1u)],
             element.record_path.last().expect("Field has no record parent?"))
             .string();
    p << "#field.";
  }
  p << element.name;
  return sus::move(p).str();
}

inline std::string construct_html_url_anchor_for_field(
    const FieldElement& element) noexcept {
  std::ostringstream url;
  if (element.record_path.is_empty()) {
    url << "variable.";
  } else {
    url << "field.";
  }
  url << element.name;
  return sus::move(url).str();
}

inline std::filesystem::path construct_html_file_path_for_function(
    std::filesystem::path root, const FunctionElement& element) noexcept {
  std::ostringstream s;
  s << "fn.";
  s << element.name;
  // TODO: Should collect all overload sets together on one html page with
  // #anchors to each set?
  if (element.overload_set.is_some())
    s << "." << element.overload_set.as_value();
  std::string name = sus::move(s).str();

  // Escaping for operator symbols in the file name.
  // TODO: base64 encode it?
  while (true) {
    if (auto pos = name.find("<<"); pos != std::string::npos) {
      name.replace(pos, 2, "_leftshift");
    } else if (pos = name.find(">>"); pos != std::string::npos) {
      name.replace(pos, 2, "_rightshift");
    } else if (pos = name.find("+"); pos != std::string::npos) {
      name.replace(pos, 1, "_plus");
    } else if (pos = name.find("-"); pos != std::string::npos) {
      name.replace(pos, 1, "_sub");
    } else if (pos = name.find("*"); pos != std::string::npos) {
      name.replace(pos, 1, "_mul");
    } else if (pos = name.find("/"); pos != std::string::npos) {
      name.replace(pos, 1, "_div");
    } else if (pos = name.find("%"); pos != std::string::npos) {
      name.replace(pos, 1, "_rem");
    } else if (pos = name.find("<=>"); pos != std::string::npos) {
      name.replace(pos, 3, "_spaceship");
    } else if (pos = name.find("=="); pos != std::string::npos) {
      name.replace(pos, 2, "_eq");
    } else if (pos = name.find("!="); pos != std::string::npos) {
      name.replace(pos, 2, "_ne");
    } else if (pos = name.find(">="); pos != std::string::npos) {
      name.replace(pos, 2, "_ge");
    } else if (pos = name.find("<="); pos != std::string::npos) {
      name.replace(pos, 2, "_le");
    } else if (pos = name.find(">"); pos != std::string::npos) {
      name.replace(pos, 1, "_gt");
    } else if (pos = name.find("<"); pos != std::string::npos) {
      name.replace(pos, 1, "_lt");
    } else if (pos = name.find("|"); pos != std::string::npos) {
      name.replace(pos, 1, "_or");
    } else if (pos = name.find("&"); pos != std::string::npos) {
      name.replace(pos, 1, "_and");
    } else if (pos = name.find("^"); pos != std::string::npos) {
      name.replace(pos, 1, "_xor");
    } else if (pos = name.find("\"\""); pos != std::string::npos) {
      name.replace(pos, 2, "_literal");
    } else if (pos = name.find("\""); pos != std::string::npos) {
      name.replace(pos, 1, "_quote");
    } else {
      break;
    }
  }

  return construct_html_file_path(sus::move(root),
                                  element.namespace_path.as_slice(),
                                  sus::Slice<std::string>(), name);
}

inline std::string construct_html_url_anchor_for_method(
    const FunctionElement& element) noexcept {
  sus_check(!element.record_path.is_empty());
  // There's no escaping that happens for anchors on the record page, unlike
  // for file paths. So we don't use construct_html_file_path_for_function()
  // here which escapes.
  std::ostringstream url;
  url << "method.";
  url << element.name;
  if (element.overload_set.is_some()) {
    url << ".";
    url << element.overload_set.as_value();
  }
  return sus::move(url).str();
}

inline std::string construct_html_url_for_function(
    const FunctionElement& element) noexcept {
  if (!element.record_path.is_empty()) {
    // There's no escaping that happens for anchors on the record page, unlike
    // for file paths. So we don't use construct_html_file_path_for_function()
    // here which escapes.
    std::ostringstream url;
    url << construct_html_file_path(
               std::filesystem::path(), element.namespace_path,
               element.record_path[sus::ops::range(
                   0_usize, element.record_path.len() - 1u)],
               element.record_path.last().unwrap())
               .string();
    url << "#" << construct_html_url_anchor_for_method(element);
    return sus::move(url).str();
  } else {
    return construct_html_file_path_for_function(std::filesystem::path(),
                                                 element)
        .string();
  }
}

/// The AliasElement may point to something not in the database, in which case
/// a link to the alias's definition is produced.
inline sus::Option<std::string> construct_html_url_for_alias(
    const AliasElement& element) noexcept {
  if (element.alias_style == AliasStyle::Forwarding) {
    // Link through to the alias target directly, as the alias doesn't introduce
    // a new symbol name.
    switch (element.target) {
      case AliasTarget::Tag::AliasOfType: {
        const LinkedType& type =
            element.target.as<AliasTarget::Tag::AliasOfType>();
        sus::Option<const TypeRef&> maybe_ref =
            type.type_element_refs.get(0u)
                .map([](const sus::Option<TypeRef>& o) { return o.as_ref(); })
                .flatten();
        return maybe_ref.map([](const TypeRef& ref) {
          switch (ref) {
            case TypeRef::Tag::Record: {
              const RecordElement& e = ref.as<TypeRef::Tag::Record>();
              sus_check_with_message(
                  !e.hidden(),
                  fmt::format("reference to hidden Record {}", e.name));
              return construct_html_url_for_type(e);
            }
            case TypeRef::Tag::Concept:
              // This doesn't occur for the top level type, as it's a type. This
              // occurs for `Concept auto` types, which do not appear in
              // aliases.
              break;
          }
          sus_unreachable();
        });
      }
      case AliasTarget::Tag::AliasOfConcept: {
        const LinkedConcept& con =
            element.target.as<AliasTarget::Tag::AliasOfConcept>();
        switch (con.ref_or_name) {
          case ConceptRefOrName::Tag::Ref: {
            const ConceptElement& e =
                con.ref_or_name.as<ConceptRefOrName::Tag::Ref>();
            return sus::some(construct_html_url_for_concept(e));
          }
          case ConceptRefOrName::Tag::Name: return sus::none();
        }
        sus_unreachable();
      }
      case AliasTarget::Tag::AliasOfMethod: {
        // TODO: Link to method.
        sus_unreachable();
      }
      case AliasTarget::Tag::AliasOfFunction: {
        const LinkedFunction& fun =
            element.target.as<AliasTarget::Tag::AliasOfFunction>();
        switch (fun.ref_or_name) {
          case FunctionRefOrName::Tag::Ref: {
            const FunctionElement& e =
                fun.ref_or_name.as<FunctionRefOrName::Tag::Ref>();
            return sus::some(construct_html_url_for_function(e));
          }
          case FunctionRefOrName::Tag::Name: return sus::none();
        }
        sus_unreachable();
      }
      case AliasTarget::Tag::AliasOfEnumConstant: {
        // TODO: Link to constant.
        sus_unreachable();
      }
      case AliasTarget::Tag::AliasOfVariable: {
        const LinkedVariable& var =
            element.target.as<AliasTarget::Tag::AliasOfVariable>();
        switch (var.ref_or_name) {
          case VariableRefOrName::Tag::Ref: {
            const FieldElement& e =
                var.ref_or_name.as<VariableRefOrName::Tag::Ref>();
            return sus::some(construct_html_url_for_field(e));
          }
          case VariableRefOrName::Tag::Name: return sus::none();
        }
        sus_unreachable();
      }
    }
    sus_unreachable();
  } else {
    // TODO: Link to the alias' page.
    return sus::some("TODO");
  }
}

inline std::string construct_html_url_anchor_for_alias(
    const AliasElement& element) noexcept {
  std::ostringstream url;
  url << "alias.";
  url << element.name;
  return sus::move(url).str();
}

inline std::filesystem::path construct_html_file_path_for_namespace(
    std::filesystem::path root, const NamespaceElement& element) noexcept {
  return construct_html_namespace_file_path(sus::move(root),
                                            element.namespace_path);
}

inline std::string construct_html_url_for_namespace(
    const NamespaceElement& element) noexcept {
  return construct_html_file_path_for_namespace(std::filesystem::path(),
                                                element)
      .string();
}

}  // namespace subdoc::gen
