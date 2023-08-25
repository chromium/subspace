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
#include "subdoc/lib/gen/markdown_to_html.h"
#include "subdoc/lib/gen/options.h"
#include "subdoc/lib/parse_comment.h"
#include "subdoc/lib/run.h"
#include "subdoc/tests/cpp_version.h"
#include "sus/collections/vec.h"
#include "sus/macros/compiler.h"
#include "sus/option/option.h"
#include "sus/prelude.h"
#include "sus/ptr/subclass.h"
#include "sus/result/result.h"

class SubDocTest : public testing::Test {
 public:
  auto run_code_with_options(const subdoc::RunOptions& options,
                             std::string content) noexcept {
    auto args = sus::Vec<std::string>::with_capacity(1u);
    args.push(std::string(subdoc::tests::cpp_version_flag(cpp_version_)));

    return subdoc::run_test(sus::move(content), args.as_slice(), options);
  }

  auto run_code(std::string content) noexcept {
    return run_code_with_options(subdoc::RunOptions().set_show_progress(false),
                                 sus::move(content));
  }

  /// Returns if a record was found whose comment location ends with
  /// `comment_loc` and whose comment begins with `comment_start`.
  bool has_namespace_comment(const subdoc::Database& db,
                             std::string_view comment_loc,
                             std::string_view comment_start) const noexcept {
    return verify_comment("namespace", db,
                          db.find_namespace_comment(comment_loc), comment_loc,
                          comment_start);
  }

  /// Returns if a record was found whose comment location ends with
  /// `comment_loc` and whose comment begins with `comment_start`.
  bool has_record_comment(const subdoc::Database& db,
                          std::string_view comment_loc,
                          std::string_view comment_start) const noexcept {
    return verify_comment("record", db, db.find_record_comment(comment_loc),
                          comment_loc, comment_start);
  }

  /// Returns if a comment was found with a location that ends with the
  /// `comment_loc` suffix, and for which the comment's text begins with
  /// `comment_start` prefix.
  bool has_function_comment(const subdoc::Database& db,
                            std::string_view comment_loc,
                            std::string_view comment_start) const noexcept {
    return verify_comment("function", db, db.find_function_comment(comment_loc),
                          comment_loc, comment_start);
  }

  /// Returns if a ctor was found whose comment location ends with
  /// `comment_loc` and whose comment begins with `comment_start`.
  bool has_ctor_comment(const subdoc::Database& db,
                        std::string_view comment_loc,
                        std::string_view comment_start) const noexcept {
    return verify_comment("method", db, db.find_ctor_comment(comment_loc),
                          comment_loc, comment_start);
  }

  /// Returns if a ctor was found whose comment location ends with
  /// `comment_loc` and whose comment begins with `comment_start`.
  bool has_dtor_comment(const subdoc::Database& db,
                        std::string_view comment_loc,
                        std::string_view comment_start) const noexcept {
    return verify_comment("method", db, db.find_dtor_comment(comment_loc),
                          comment_loc, comment_start);
  }

  /// Returns if a method was found whose comment location ends with
  /// `comment_loc` and whose comment begins with `comment_start`.
  bool has_method_comment(const subdoc::Database& db,
                          std::string_view comment_loc,
                          std::string_view comment_start) const noexcept {
    return verify_comment("method", db, db.find_method_comment(comment_loc),
                          comment_loc, comment_start);
  }

  /// Returns if a field was found whose comment location ends with
  /// `comment_loc` and whose comment begins with `comment_start`.
  bool has_field_comment(const subdoc::Database& db,
                         std::string_view comment_loc,
                         std::string_view comment_start) const noexcept {
    return verify_comment("field", db, db.find_field_comment(comment_loc),
                          comment_loc, comment_start);
  }

 private:
  bool verify_comment(std::string_view type, const subdoc::Database& db,
                      sus::Option<const subdoc::CommentElement&> element,
                      std::string_view comment_loc,
                      std::string_view comment_start) const noexcept {
    if (element.is_none()) {
      llvm::errs() << "Unable to find " << type << " comment at " << comment_loc
                   << "\n";
      return false;
    }

    auto options = subdoc::gen::Options();
    auto page_state = subdoc::gen::ParseMarkdownPageState{
        .db = db,
        .options = options,
        .self_link_counts = self_link_counts.take().unwrap(),
    };
    sus::Result<std::string, subdoc::gen::MarkdownToHtmlError> html_result =
        markdown_to_html_full(element->comment, page_state);
    self_link_counts = sus::some(sus::move(page_state).self_link_counts);

    if (html_result.is_err()) {
      llvm::errs() << type << " comment at " << comment_loc
                   << " had error parsing comment markdown: "
                   << sus::move(html_result).unwrap_err().message;
      return false;
    } else {
      std::string html = sus::move(html_result).unwrap();
      if (!html.starts_with(comment_start)) {
        llvm::errs() << type << " comment at " << comment_loc
                     << " does not match text. Found:\n"
                     << html << "\n";
        return false;
      }
    }
    return true;
  }

  subdoc::tests::SubDocCppVersion cpp_version_ =
      subdoc::tests::SubDocCppVersion::Cpp20;
  mutable sus::Option<std::unordered_map<std::string, u32>> self_link_counts =
      sus::some(std::unordered_map<std::string, u32>());
};
