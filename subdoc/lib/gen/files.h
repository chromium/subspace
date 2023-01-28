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
#include "subspace/containers/slice.h"
#include "subspace/iter/iterator.h"

namespace subdoc::gen {

inline sus::Option<std::ofstream> open_file_for_writing(
    std::filesystem::path path) noexcept {
  sus::Option<std::ofstream> out;

  std::ofstream file;
  file.open(path.c_str());
  if (file.is_open()) {
    out.insert(sus::move(file));
  } else {
    llvm::errs() << "Unable to open file " << path.string() << " for writing\n";
  }

  return out;
}

inline std::filesystem::path construct_html_file_path(
    std::filesystem::path root, sus::Slice<const Namespace> namespace_path,
    sus::Slice<const std::string> class_path, std::string_view name) noexcept {
  std::filesystem::path p = sus::move(root);

  std::ostringstream fname;
  // TODO: Add Iterator::reverse() and use that.
  for (size_t i = 0; i < namespace_path.len(); ++i) {
    const Namespace& n = namespace_path[namespace_path.len() - i - 1u];
    switch (n) {
      case Namespace::Tag::Global: break;
      case Namespace::Tag::Anonymous:
        fname << "anonymous";
        fname << "-";
        break;
      case Namespace::Tag::Named:
        fname << n.get_ref<Namespace::Tag::Named>();
        fname << "-";
        break;
    }
  }
  // TODO: Add Iterator::reverse.
  for (const auto& n : class_path.iter() /*.reverse()*/) {
    fname << n + "-";
  }
  fname << name;
  fname << ".html";
  p.append(sus::move(fname).str());
  return p;
}

inline std::filesystem::path construct_html_file_path_for_namespace(
    std::filesystem::path root, const NamespaceElement& element) noexcept {
  // The namespace path includes the namespace element itself, so drop
  // that one.
  sus::Slice<const Namespace> short_namespace_path =
      element.namespace_path.as_ref()[{1u, element.namespace_path.len() - 1u}];

  std::string file_name = [&]() {
    if (element.namespace_name.which() == Namespace::Tag::Global) {
      // We're generating the global namespace, which will go in
      // `global-ns.html`. Namespaces can't be named `namespace` so this can't
      // collide with a real namespace `global::namespace`.
      return std::string("global-namespace");
    } else {
      // Otherwise, just use the local name of the namespace.
      return element.name;
    }
  }();

  return construct_html_file_path(root, short_namespace_path,
                                  sus::Slice<const std::string>(),
                                  sus::move(file_name));
}

}  // namespace subdoc::gen
