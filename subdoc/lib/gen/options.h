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
#include <string>

#include "sus/collections/vec.h"
#include "sus/prelude.h"
#include "sus/result/result.h"

namespace subdoc::gen {

struct FavIcon {
  /// Expects a string of form 'path;mimetype'.
  static sus::Result<FavIcon, std::string> from_string(
      std::string_view s) noexcept {
    usize pos = s.find(';');
    if (pos == std::string::npos) {
      return sus::err("invalid favicon string, use 'path;mimetype'");
    }
    auto path = std::string_view(s).substr(0u, pos);
    auto mime = std::string_view(s).substr(pos + 1u);
    return sus::ok(FavIcon(std::string(path), std::string(mime)));
  }

  FavIcon(FavIcon&&) = default;
  FavIcon& operator=(FavIcon&&) = default;

  FavIcon clone() const noexcept {
    return FavIcon(sus::clone(path), sus::clone(mime));
  }

  std::string path;
  std::string mime;

 private:
  FavIcon(std::string path, std::string mime)
      : path(sus::move(path)), mime(sus::move(mime)) {}
};

struct Options {
  std::string project_name = "PROJECT NAME";
  std::string project_logo = "PROJECT LOGO.png";
  // TODO: sus::PathBuf type would be real nice.
  std::filesystem::path output_root;
  sus::Vec<std::string> stylesheets;
  sus::Vec<FavIcon> favicons;
  sus::Vec<std::string> copy_files;
  bool ignore_bad_code_links = false;
};
static_assert(sus::mem::Move<Options>);

}  // namespace subdoc::gen
