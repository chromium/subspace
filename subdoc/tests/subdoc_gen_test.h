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

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>

#include "googletest/include/gtest/gtest.h"
#include "subdoc/lib/database.h"
#include "subdoc/lib/gen/generate.h"
#include "subdoc/lib/run.h"
#include "subdoc/tests/cpp_version.h"
#include "subdoc/tests/test_main.h"
#include "sus/assertions/unreachable.h"
#include "sus/collections/vec.h"
#include "sus/macros/compiler.h"
#include "sus/option/option.h"
#include "sus/prelude.h"
#include "sus/ptr/subclass.h"
#include "sus/result/result.h"

class SubDocGenTest : public testing::Test {
 public:
  bool run_gen_test(std::string directory) noexcept {
    std::string content =
        read_file(path_to_input(directory, sus::some("test.cc"))).unwrap();

    auto args = sus::Vec<std::string>();
    args.push(std::string(subdoc::tests::cpp_version_flag(cpp_version_)));

    auto run_options = subdoc::RunOptions().set_show_progress(false);

    auto result = subdoc::run_test(sus::move(content), args.as_slice(),
                                   sus::move(run_options));
    if (!result.is_ok()) return false;

    using subdoc::gen::FavIcon;
    auto options = subdoc::gen::Options{
        .output_root =
            [&]() {
              std::filesystem::path test_root = output_root;
              test_root.append(directory);
              return test_root;
            }(),
        .stylesheets = sus::Vec<std::string>("../subdoc-test-style.css"),
        .favicons =
            sus::Vec(FavIcon::from_string("../icon.svg;image/svg+xml").unwrap(),
                     FavIcon::from_string("../icon.png;image/png").unwrap()),
        .copy_files = sus::Vec<std::string>(),
        .ignore_bad_code_links = false,
    };
    sus::result::Result<void, sus::Box<sus::error::DynError>> r =
        subdoc::gen::generate(sus::move(result).unwrap(), options);
    if (r.is_err()) {
      std::string fail = fmt::to_string(r.as_err());
      for (sus::Option<const sus::error::DynError&> source =
               sus::error::error_source(r.as_err());
           source.is_some();
           source = sus::error::error_source(source.as_value())) {
        fail = fmt::format("{}: {}", fail, source.as_value());
      }
      ADD_FAILURE() << fail;
      return false;
    }

    // TODO: Add an Iterator<const std::filesystem::path&> concept.
    auto paths_to_strings = [](auto paths_iter) {
      std::string s;
      for (const auto& p : paths_iter) s = s + p.string() + "\n";
      return s;
    };

    sus::Vec<std::filesystem::path> expecteds;
    find_paths(path_to_input(directory, sus::none()), std::filesystem::path(),
               expecteds);
    sus::Vec<std::filesystem::path> actuals;
    find_paths(path_to_output(directory, sus::none()), std::filesystem::path(),
               actuals);
    // TODO: Implement Vec::Eq.
    auto eq = [](const auto& a, const auto& b) {
      if (a.len() != b.len()) return false;
      for (auto i = 0_usize; i < a.len(); i += 1u)
        if (a[i] != b[i]) return false;
      return true;
    };
    if (!eq(expecteds, actuals)) {
      ADD_FAILURE() << "Found different files in output than expected.\n"
                    << "Expected:\n"
                    << paths_to_strings(expecteds.iter())  //
                    << "Actual:\n"
                    << paths_to_strings(actuals.iter());
      return false;
    }
    return compare_files(path_to_input(directory, sus::none()),
                         path_to_output(directory, sus::none()),
                         std::filesystem::path());
  }

 private:
  /// Gives the path to a test input file.
  static std::filesystem::path path_to_input(
      std::string_view directory, sus::Option<std::string_view> file) noexcept {
    std::filesystem::path path;
    path.append("..");
    path.append("..");
    path.append("subdoc");
    path.append("gen_tests");
    path.append(directory);
    if (file.is_some()) path.append(sus::move(file).unwrap());
    return path;
  }
  /// Gives the path to a test-generated output file.
  static std::filesystem::path path_to_output(
      std::string_view directory, sus::Option<std::string_view> file) noexcept {
    std::filesystem::path path;
    path.append(output_root);
    path.append(directory);
    if (file.is_some()) path.append(sus::move(file).unwrap());
    return path;
  }

  static sus::Option<std::string> read_file(
      const std::filesystem::path& path) noexcept {
    std::ifstream file;
    file.open(path.c_str());
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

  static void find_paths(const std::filesystem::path& base,
                         const std::filesystem::path& relative,
                         sus::Vec<std::filesystem::path>& collect) noexcept {
    std::filesystem::path working = base;
    working /= relative;

    auto it = std::filesystem::directory_iterator(working);
    while (it != std::filesystem::directory_iterator()) {
      if (it->path().filename().string().ends_with(".html")) {
        auto path = relative;
        path.append(it->path().filename().string());
        collect.push(path);
      }
      if (it->is_directory()) {
        auto recurse_relative = relative;
        recurse_relative.append(it->path().filename().string());
        find_paths(base, recurse_relative, collect);
      }
      ++it;
    }
  }

  static bool compare_files(const std::filesystem::path& in_base,
                            const std::filesystem::path& out_base,
                            const std::filesystem::path& relative) noexcept {
    std::filesystem::path in = in_base;
    in /= relative;
    std::filesystem::path out = out_base;
    out /= relative;

    auto it = std::filesystem::directory_iterator(in);
    while (it != std::filesystem::directory_iterator()) {
      if (it->path().filename().string().ends_with(".html")) {
        auto expected_path = in;
        expected_path.append(it->path().filename().string());
        auto actual_path = out;
        actual_path.append(it->path().filename().string());

        if (test_main_command_line_args().contains("--rebaseline")) {
          std::error_code ec;
          std::filesystem::copy_file(
              actual_path, expected_path,
              std::filesystem::copy_options::overwrite_existing, ec);
          if (ec) {
            llvm::errs() << "--rebaseline failed to copy "
                         << actual_path.string() << " to "
                         << expected_path.string() << ": " << ec.message()
                         << "\n";
            return false;
          }
        } else {
          std::string expected = read_file(expected_path).unwrap();
          std::string actual = read_file(actual_path).unwrap();
          if (expected != actual) {
            llvm::errs() << "Expected:\n" << expected << "\n";
            llvm::errs() << "Actual:\n" << actual << "\n";
            ADD_FAILURE() << "Files differ: " << expected_path.string()
                          << " vs " << actual_path.string() << "\n";
            return false;
          }
        }
      }
      if (it->is_directory()) {
        auto recurse_relative = relative;
        recurse_relative.append(it->path().filename().string());
        if (!compare_files(in_base, out_base, recurse_relative)) return false;
      }
      ++it;
    }
    return true;
  };

  constexpr static std::string_view output_root = "gen_tests_out";

  subdoc::tests::SubDocCppVersion cpp_version_ =
      subdoc::tests::SubDocCppVersion::Cpp20;
};
