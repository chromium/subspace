// Copyright 2022 Google LLC
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
#include <string_view>

#include "googletest/include/gtest/gtest.h"
#include "subdoc/lib/database.h"
#include "subdoc/lib/run.h"
#include "subspace/assertions/unreachable.h"
#include "subspace/containers/vec.h"
#include "subspace/convert/subclass.h"
#include "subspace/macros/compiler.h"
#include "subspace/option/option.h"
#include "subspace/prelude.h"
#include "subspace/result/result.h"

enum class SubDocCppVersion {
  Cpp20,
};

inline std::string_view cpp_version_flag(SubDocCppVersion v) noexcept {
  switch (v) {
    case SubDocCppVersion::Cpp20: return "-std=c++20";
  }
  sus::unreachable();
}

class SubDocTest : public testing::Test {
 public:
  auto run_code(std::string content) noexcept {
    auto args = sus::Vec<std::string>();
    args.push(std::string(cpp_version_flag(cpp_version_)));
    return subdoc::run_test(sus::move(content), sus::move(args));
  }

  /// Returns if a class was found whose comment location ends with
  /// `comment_loc` and whose comment begins with `comment_start`.
  bool has_class_comment(const subdoc::Database& db,
                         std::string_view comment_loc,
                         std::string_view comment_start) noexcept {
    return find_comment(db.classes, comment_loc, comment_start);
  }

  /// Returns if there were no class comments found.
  bool has_no_class_comments(const subdoc::Database& db) noexcept {
    return db.classes.size() == 0;
  }

  /// Returns if a function was found whose comment location ends with
  /// `comment_loc` and whose comment begins with `comment_start`.
  bool has_fn_comment(const subdoc::Database& db, std::string_view comment_loc,
                      std::string_view comment_start) noexcept {
    return find_comment(db.functions, comment_loc, comment_start);
  }

  /// Returns if there were no function comments found.
  bool has_no_fn_comments(const subdoc::Database& db) noexcept {
    return db.functions.size() == 0;
  }

  /// Returns if a field was found whose comment location ends with
  /// `comment_loc` and whose comment begins with `comment_start`.
  bool has_field_comment(const subdoc::Database& db,
                         std::string_view comment_loc,
                         std::string_view comment_start) noexcept {
    return find_comment(db.fields, comment_loc, comment_start);
  }

  /// Returns if there were no field comments found.
  bool has_no_field_comments(const subdoc::Database& db) noexcept {
    return db.fields.size() == 0;
  }

 private:
  template <class MapT>
  bool find_comment(const MapT& map, std::string_view comment_loc,
                    std::string_view comment_start) noexcept {
    for (const auto& [k, element] : map) {
      if (element.comment.begin_loc.ends_with(comment_loc))
        return element.comment.raw_text.starts_with(comment_start);
    }
    ADD_FAILURE() << "Comment location " << comment_loc
                  << " not found. Valid locations:";
    for (const auto& [k, element] : map)
      llvm::errs() << "  " << element.comment.begin_loc << "\n";
    return false;
  }

  SubDocCppVersion cpp_version_ = SubDocCppVersion::Cpp20;
};
