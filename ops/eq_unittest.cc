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

#include "ops/eq.h"

#include <type_traits>

#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace {

using sus::ops::Eq;

// These types are comparable.
static_assert(Eq<int, int>);
static_assert(Eq<int, char>);
static_assert(Eq<char, int>);

// These types are not comparable.
static_assert(!Eq<int, char*>);

}
