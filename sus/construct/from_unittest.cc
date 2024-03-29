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

#include "sus/construct/from.h"

#include "sus/construct/into.h"
#include "sus/result/result.h"

using sus::construct::From;
using sus::construct::TryFrom;

namespace {

struct S {};

struct WithFromInt {
  static auto from(int) { return WithFromInt(); }
};
struct NotFromInt {};
struct WithFromS {
  static auto from(S) { return WithFromS(); }
};
struct FromReturnsVoid {
  static auto from(int) {}
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

enum Error {};

struct TryYes {
  static sus::Result<TryYes, Error> try_from(S) { return sus::ok(TryYes()); }
};
struct TryNoResult {
  static TryYes try_from(S) { return TryYes(); }
};
struct TryWrongResult {
  static sus::Result<S, Error> try_from(S) { return sus::ok(S()); }
};
struct TryVoidResult {
  static sus::Result<void, Error> try_from(S) { return sus::ok(); }
};

static_assert(TryFrom<TryYes, S>);
static_assert(!TryFrom<TryNoResult, S>);
static_assert(!TryFrom<TryWrongResult, S>);
static_assert(!TryFrom<TryVoidResult, S>);

}  // namespace
