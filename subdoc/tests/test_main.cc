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

#include "subdoc/tests/test_main.h"

#include "googletest/include/gtest/gtest.h"
#include "sus/iter/iterator.h"
#include "sus/prelude.h"

namespace {
sus::Vec<std::string_view>* args;
}

sus::Slice<std::string_view> test_main_command_line_args() { return *args; }

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);

  auto slice = sus::SliceMut<char*>::from_raw_parts_mut(
      unsafe_fn, argv, usize::try_from(argc).unwrap());
  args = new sus::Vec<std::string_view>(
      slice.iter()
          .map([](char* arg) { return std::string_view(arg); })
          .collect<sus::Vec<std::string_view>>());

  return RUN_ALL_TESTS();
}
