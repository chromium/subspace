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

#include "subdoc/lib/run.h"
#include "googletest/include/gtest/gtest.h"
#include "subspace/assertions/unreachable.h"
#include "subspace/containers/vec.h"
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
  sus::Option<subdoc::Database> run_code(std::string content) noexcept {
    auto args = sus::Vec<std::string>();
    args.push(std::string(cpp_version_flag(cpp_version_)));

    auto a = subdoc::run_test(sus::move(content), sus::move(args));
    return sus::move(a).or_else([]() -> sus::Option<subdoc::Database> {
      ADD_FAILURE() << "Compilation failed.";
      return sus::none();
    });
  }

 private:
  SubDocCppVersion cpp_version_ = SubDocCppVersion::Cpp20;
};
