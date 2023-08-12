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

// IWYU pragma: private, include "sus/prelude.h"
// IWYU pragma: friend "sus/.*"
#pragma once

#include "fmt/core.h"
#include "sus/string/__private/format_to_stream.h"

namespace sus::marker {

/// A marker that designates a function as unsafe, or containing Undefined
/// Behaviour if its preconditions are not met.
///
/// Use of an unsafe function should require a comment documenting how the
/// required preconditions are met in the form:
///
/// ```
/// // SAFETY: This is known to be true because of that which we checked there.
/// do_risky_thing(unsafe_fn);
/// ```
///
/// Input conditions of the unsafe function should be well encapsulated so that
/// it is even possible to reason about how they are met and to maintain that
/// over time.
///
/// To call such an unsafe function, pass it the global `unsafe_fn` object,
/// which is brought into the global namespace by `#include "sus/prelude.h"`.
struct UnsafeFnMarker {
  // `explicit` prevents `{}` or `{any values...}` from being used to construct
  // an `UnsafeFnMarker`. The marker should only be used as `unsafe_fn`.
  explicit consteval UnsafeFnMarker() {}
};

/// The global `UnsafeFnMarker` which can be passed to unsafe functions. See
/// the `UnsafeFnMarker` type for an explanation.
constexpr inline auto unsafe_fn = UnsafeFnMarker();

}  // namespace sus::marker

// fmt support.
template <class Char>
struct fmt::formatter<::sus::marker::UnsafeFnMarker, Char> {
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return ctx.begin();
  }

  template <class FormatContext>
  constexpr auto format(const ::sus::marker::UnsafeFnMarker&,
                        FormatContext& ctx) const {
    return fmt::format_to(ctx.out(), "unsafe_fn");
  }
};

// Stream support.
sus__format_to_stream(sus::marker, UnsafeFnMarker);
