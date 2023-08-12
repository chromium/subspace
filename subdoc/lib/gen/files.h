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
#include "sus/containers/slice.h"
#include "sus/iter/iterator.h"

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
    std::filesystem::path root, const sus::Slice<Namespace>& namespace_path,
    const sus::Slice<std::string>& record_path,
    std::string_view name) noexcept {
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

inline std::filesystem::path construct_html_file_path_for_function(
    std::filesystem::path root, const FunctionElement& element,
    u32 overload_set) noexcept {
  std::ostringstream s;
  s << element.name;
  // TODO: Should collect all overload sets together on one html page with
  // #anchors to each set? How can the overload be more stable over time?
  if (overload_set > 0u) s << "." << overload_set;
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

inline std::filesystem::path construct_html_file_path_for_namespace(
    std::filesystem::path root, const NamespaceElement& element) noexcept {
  // The namespace path includes the namespace element itself, so drop
  // that one.
  sus::Slice<Namespace> short_namespace_path =
      element.namespace_path.as_slice()["1.."_r];

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

  return construct_html_file_path(sus::move(root), short_namespace_path,
                                  sus::Slice<std::string>(),
                                  sus::move(file_name));
}

}  // namespace subdoc::gen
