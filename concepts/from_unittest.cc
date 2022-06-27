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

#include "concepts/from.h"

#include "concepts/into.h"

using sus::concepts::from::From;

namespace {

struct S {};

struct WithFromInt {
  static auto from(int) { return WithFromInt(); }
};
struct NotFromInt {
};
struct WithFromS {
  static auto from(S) { return WithFromS(); }
};
struct FromReturnsVoid {
  static auto from(int) {  }
};
struct FromReturnsElse {
  static auto from(int i) { return i; }
};

static_assert(From<WithFromInt, int>);
// TODO: Can we prevent type conversion?
static_assert(From<WithFromInt, short>);
static_assert(!From<NotFromInt, int>);
static_assert(!From<WithFromS, int>);
static_assert(!From<FromReturnsVoid, int>);
static_assert(!From<FromReturnsElse, int>);

}  // namespace
