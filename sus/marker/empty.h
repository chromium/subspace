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

/// A marker that designates emptinesss, for constructing an empty collection.
///
/// To use this type as an argument, use the global [`empty`](
/// $sus::marker::empty).
struct EmptyMarker {
  /// `explicit` prevents `{}` or `{any values...}` from being used to construct
  /// an `Empty`. The marker should only be used as [`empty`](
  /// $sus::marker::empty).
  /// #[doc.hidden]
  explicit consteval EmptyMarker() {}
};

/// The global [`EmptyMarker`]($sus::marker::EmptyMarker) which can be passed to
/// constructors to allow type deduction instead of having to write out the full
/// default constructor.
constexpr inline auto empty = EmptyMarker();

}  // namespace sus::marker

// fmt support.
template <class Char>
struct fmt::formatter<::sus::marker::EmptyMarker, Char> {
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return ctx.begin();
  }

  template <class FormatContext>
  constexpr auto format(const ::sus::marker::EmptyMarker&,
                        FormatContext& ctx) const {
    return fmt::format_to(ctx.out(), "empty");
  }
};

// Stream support.
sus__format_to_stream(sus::marker, EmptyMarker);

// Promote `empty` into the `sus` namespace.
namespace sus {
using sus::marker::empty;
}
