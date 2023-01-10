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

#include <fstream>
#include <sstream>
#include <string>
#include <string_view>

#include "googletest/include/gtest/gtest.h"
#include "subdoc/lib/database.h"
#include "subdoc/lib/gen/generate.h"
#include "subdoc/lib/run.h"
#include "subdoc/tests/cpp_version.h"
#include "subspace/assertions/unreachable.h"
#include "subspace/containers/vec.h"
#include "subspace/convert/subclass.h"
#include "subspace/macros/compiler.h"
#include "subspace/option/option.h"
#include "subspace/prelude.h"
#include "subspace/result/result.h"

class SubDocGenTest : public testing::Test {
 public:
  bool run_gen_test(std::string directory) noexcept {
    std::string content =
        read_file(path_to_input(directory, "test.cc")).unwrap();

    auto args = sus::Vec<std::string>();
    args.push(std::string(subdoc::tests::cpp_version_flag(cpp_version_)));
    auto result = subdoc::run_test(sus::move(content), sus::move(args));
    if (!result.is_ok()) return false;

    auto options = subdoc::gen::Options{
        .output_root = std::string(output_root),
    };
    subdoc::gen::generate(sus::move(result).unwrap(), options);

    // TODO: Read the directory for all html files and compare them all.
    std::string expected =
        read_file(path_to_input(directory, "index.html")).unwrap();
    sus::Option<std::string> actual_opt =
        read_file(path_to_output(directory, "index.html"));
    if (actual_opt.is_none()) return false;
    std::string actual = sus::move(actual_opt).unwrap();

    return true;
  }

 private:
  /// Gives the path to a test input file.
  static std::string path_to_input(std::string_view directory,
                                   std::string_view file) noexcept {
    std::ostringstream path_stream;
    path_stream << ".." << sep << ".." << sep << "subdoc" << sep << "gen_tests"
                << sep << directory << sep << file;
    return sus::move(path_stream).str();
  }
  /// Gives the path to a test-generated output file.
  static std::string path_to_output(std::string_view directory,
                                    std::string_view file) noexcept {
    std::ostringstream path_stream;
    path_stream << output_root << sep << directory << sep << file;
    return sus::move(path_stream).str();
  }

  static sus::Option<std::string> read_file(std::string_view path) noexcept {
    std::ifstream file;
    file.open(path.data());
    if (!file.is_open()) {
      ADD_FAILURE() << "Unable to open file " << path;
      return sus::none();
    }
    std::ostringstream content;
    std::string line;
    while (std::getline(file, line)) {
      content << line << "\n";
    }
    file.close();
    return sus::some(sus::move(content).str());
  }

#ifdef _MSC_VER
  constexpr static std::string_view sep = "\\";
#else
  constexpr static std::string_view sep = "/";
#endif

  constexpr static std::string_view output_root = "gen_tests_out";

  subdoc::tests::SubDocCppVersion cpp_version_ =
      subdoc::tests::SubDocCppVersion::Cpp20;
};
