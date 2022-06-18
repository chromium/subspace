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

#include "concepts/into.h"

#include "third_party/googletest/googletest/include/gtest/gtest.h"

using sus::concepts::from::From;
using sus::concepts::into::Into;

namespace {

struct S {
  S() {}
  S(S&&) {}
};
struct T {
  T() {}
  T(T&&) {}
};

struct FromS {
  static auto From(S) { return FromS(); }
};

static_assert(From<FromS, S>);   // S to FromS works.
static_assert(!From<FromS, T>);  // T to FromS doesn't.

template <class To, class From>
concept receive_from = requires(From from) {
                         {
                           [](To) {}(Into(static_cast<From&&>(from)))
                         };
                       };

static_assert(receive_from<FromS, S>);   // Receiving FromS from S works.
static_assert(!receive_from<FromS, T>);  // Receiving FromS from T doesn't.

struct FromInt {
  static auto From(int) { return FromInt(); }
};

// Receiving FromInt from int works.
static_assert(receive_from<FromInt, int>);
// Unfortunately, implicit conversions happen too.
//
// TODO: Can we somehow prevent it?
static_assert(receive_from<FromInt, short>);
static_assert(receive_from<FromInt, long long>);

}  // namespace
