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

#pragma once

#include <type_traits>

#include "fmt/core.h"
#include "subspace/string/__private/bytes_formatter.h"

namespace sus::string::__private {

/// A formatter that can format any type.
///
/// If the type is not formattable itself, it will be turned into a string of
/// the byte values in the type.
///
/// To also be able to format `void` use `AnyOrVoidFormatter`.
template <class T, class Char>
using AnyFormatter = std::conditional_t<
    fmt::is_formattable<std::remove_cv_t<std::remove_reference_t<T>>,
                        Char>::value,
    fmt::formatter<std::remove_cv_t<std::remove_reference_t<T>>, Char>,
    BytesFormatter<Char>>;

template <class Char>
auto format_void(auto out) {
  *out++ = static_cast<Char>('<');
  *out++ = static_cast<Char>('v');
  *out++ = static_cast<Char>('o');
  *out++ = static_cast<Char>('i');
  *out++ = static_cast<Char>('d');
  *out++ = static_cast<Char>('>');
  return out;
}

template <class Char>
struct VoidFormatter {
  template <class ParseContext>
  constexpr decltype(auto) parse(ParseContext& ctx) {
    return ctx.begin();
  }
  template <class T, class FormatContext>
  constexpr auto format(const T&, FormatContext& ctx) const {
    return format_void<Char>(ctx.out());
  }
};

/// A formatter that can format any type and void.
///
/// If the type is not formattable itself, it will be turned into a string of
/// the byte values in the type. A `void` will be formatted as the string
/// "<void>".
template <class T, class Char>
using AnyOrVoidFormatter =
    std::conditional_t<std::is_void_v<T>, VoidFormatter<Char>,
                       AnyFormatter<T, Char>>;

}  // namespace sus::string::__private
