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

#include "cir/lib/run.h"
#include "googletest/include/gtest/gtest.h"
#include "subspace/assertions/unreachable.h"
#include "subspace/containers/vec.h"
#include "subspace/macros/compiler.h"
#include "subspace/option/option.h"
#include "subspace/prelude.h"
#include "subspace/result/result.h"

enum class CirCppVersion {
  Cpp20,
};

std::string_view cpp_version_flag(CirCppVersion v) noexcept {
  switch (v) {
    case CirCppVersion::Cpp20: return "-std=c++20";
  }
  sus::unreachable();
}

class CirTest : public testing::Test {
 public:
  sus::Option<cir::Output> run_code(std::string content) noexcept {
    auto args = sus::Vec<std::string>();
    args.push(std::string(cpp_version_flag(cpp_version_)));

    auto a = cir::run_test(sus::move(content), sus::move(args));
    return sus::move(a).or_else([]() -> sus::Option<cir::Output> {
      ADD_FAILURE() << "Compilation failed.";
      return sus::none();
    });
  }

  static bool cir_eq(sus::Option<cir::Output> output,
                     std::string expected) noexcept {
    if (output.is_none()) return false;
    auto output_string = cir::to_string(sus::move(output).unwrap());
    auto output_no_whitespace = strip_whitespace(output_string);
    auto expected_no_whitespace = strip_whitespace(expected);
    if (output_no_whitespace != expected_no_whitespace) {
      ADD_FAILURE();
      llvm::errs() << "\nFound unexpected CIR output:\n"
                   << output_string << "\n";
      llvm::errs() << "\nExpected:\n" << strip_empty_lines(expected) << "\n";
      return false;
    }
    return true;
  }

 private:
  static std::string strip_whitespace(const std::string& s) noexcept {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
      if (c != ' ' && c != '\r' && c != '\n' && c != '\t') out.push_back(c);
    }
    return out;
  }
  static std::string strip_empty_lines(const std::string& s) noexcept {
    std::string out;
    out.reserve(s.size());
    char last = '\n';
    for (char c : s) {
      if (c == '\n' && last == '\n') continue;
      last = c;
      out.push_back(c);
    }
    return out;
  }

  CirCppVersion cpp_version_ = CirCppVersion::Cpp20;
};
