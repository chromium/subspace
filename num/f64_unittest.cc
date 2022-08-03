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

#include "num/float.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

TEST(f64, Literals) {
  static_assert((0._f64).primitive_value == 0.);
  static_assert((0.0_f64).primitive_value == 0.);
  static_assert((1.2345678912345_f64).primitive_value == 1.2345678912345);
  static_assert((-1.2345678912345_f64).primitive_value == -1.2345678912345);

  // Whole numbers.
  static_assert((0_f64).primitive_value == 0.);
  static_assert((1_f64).primitive_value == 1.);
  static_assert((-5_f64).primitive_value == -5.);
}
