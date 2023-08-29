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

#include "sus/env/var.h"

#include "googletest/include/gtest/gtest.h"

namespace {

TEST(EnvVar, Present) {
  const char VAR_NAME[] = "subspace_test_envvar_present_42389489423423";
  const char VAR_VALUE[] = "HelLo wOrLd";
  sus::env::set_var(VAR_NAME, VAR_VALUE);
  auto value = sus::env::var(VAR_NAME).unwrap();
  EXPECT_EQ(value, VAR_VALUE);
}

TEST(EnvVar, Absent) {
  const char VAR_NAME[] = "subspace_test_envvar_present_42389489423424";
  auto value = sus::env::var(VAR_NAME);
  EXPECT_EQ(value, sus::err(sus::env::VarError::NotFound));
}

}  // namespace
