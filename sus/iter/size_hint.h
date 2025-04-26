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

// IWYU pragma: private, include "sus/iter/iterator.h"
// IWYU pragma: friend "sus/.*"
#pragma once

#include "sus/num/unsigned_integer.h"
#include "sus/option/option.h"
#include "sus/string/__private/format_to_stream.h"

namespace sus::iter {

struct SizeHint {
  ::sus::num::usize lower;
  ::sus::option::Option<::sus::num::usize> upper;

  friend constexpr bool operator==(const SizeHint& lhs,
                                   const SizeHint& rhs) noexcept = default;

  // Stream support.
  _sus_format_to_stream(SizeHint)
};

}  // namespace sus::iter

// fmt support.
template <class Char>
struct fmt::formatter<::sus::iter::SizeHint, Char> {
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return ctx.begin();
  }

  template <class FormatContext>
  FormatContext::iterator format(const ::sus::iter::SizeHint& t, FormatContext& ctx) const;
};
