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
#include "subdoc/tests/cpp_version.h"
#include "subspace/containers/vec.h"
#include "subspace/convert/subclass.h"
#include "subspace/macros/compiler.h"
#include "subspace/option/option.h"
#include "subspace/prelude.h"
#include "subspace/result/result.h"

class SubDocTest : public testing::Test {
 public:
  auto run_code(std::string content) noexcept {
    auto args = sus::Vec<std::string>();
    args.push(std::string(subdoc::tests::cpp_version_flag(cpp_version_)));
    return subdoc::run_test(sus::move(content), sus::move(args));
  }

  /// Returns if a record was found whose comment location ends with
  /// `comment_loc` and whose comment begins with `comment_start`.
  bool has_record_comment(const subdoc::Database& db,
                          std::string_view comment_loc,
                          std::string_view comment_start) const noexcept {
    return verify_comment("record", db.find_record_comment(comment_loc),
                          comment_loc, comment_start);
  }

  /// Returns if a comment was found with a location that ends with the
  /// `comment_loc` suffix, and for which the comment's text begins with
  /// `comment_start` prefix.
  bool has_function_comment(const subdoc::Database& db,
                            std::string_view comment_loc,
                            std::string_view comment_start) const noexcept {
    return verify_comment("function", db.find_function_comment(comment_loc),
                          comment_loc, comment_start);
  }

  /// Returns if a method was found whose comment location ends with
  /// `comment_loc` and whose comment begins with `comment_start`.
  bool has_method_comment(const subdoc::Database& db,
                          std::string_view comment_loc,
                          std::string_view comment_start) const noexcept {
    return verify_comment("method", db.find_method_comment(comment_loc),
                          comment_loc, comment_start);
  }

  /// Returns if a field was found whose comment location ends with
  /// `comment_loc` and whose comment begins with `comment_start`.
  bool has_field_comment(const subdoc::Database& db,
                         std::string_view comment_loc,
                         std::string_view comment_start) const noexcept {
    return verify_comment("field", db.find_field_comment(comment_loc),
                          comment_loc, comment_start);
  }

 private:
  bool verify_comment(std::string_view type,
                      sus::Option<const subdoc::CommentElement&> element,
                      std::string_view comment_loc,
                      std::string_view comment_start) const noexcept {
    if (element.is_none()) {
      ADD_FAILURE() << "Unable to find " << type << " comment at "
                    << comment_loc << "\n";
      return false;
    }
    if (!sus::move(element).unwrap().comment.raw_text.starts_with(
            comment_start)) {
      ADD_FAILURE() << type << " comment at " << comment_loc
                    << " does not match text.\n";
      return false;
    }
    return true;
  }

  subdoc::tests::SubDocCppVersion cpp_version_ =
      subdoc::tests::SubDocCppVersion::Cpp20;
};
