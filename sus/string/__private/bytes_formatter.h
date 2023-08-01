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

#include "fmt/core.h"
#include "sus/mem/size_of.h"

namespace sus::string::__private {

template <class Char>
auto format_bytes(auto out, fmt::string_view bytes) {
  for (size_t i = 0; i < bytes.size(); i += 1) {
    const auto p = static_cast<unsigned char>(bytes[i]);
    const auto low = p % 16;
    const auto high = p / 16;
    if (high > 9)
      *out++ = static_cast<Char>('a' + high - 10);
    else
      *out++ = static_cast<Char>('0' + high);
    if (low > 9)
      *out++ = static_cast<Char>('a' + low - 10);
    else
      *out++ = static_cast<Char>('0' + low);
    if (i < bytes.size() - 1) *out++ = static_cast<Char>('-');
  }
  return out;
}

/// A formatter for any type that formats it as its byte values.
///
/// If the type's data size is known (it's not a union) then only
/// its data bytes will be included, not its tail padding.
template <class Char>
struct BytesFormatter {
  template <class ParseContext>
  constexpr auto parse(ParseContext& ctx) {
    return ctx.begin();
  }

  template <class T, class FormatContext>
  constexpr auto format(const T& t, FormatContext& ctx) const {
    constexpr size_t num_bytes = ::sus::mem::data_size_of<T>()
                                     ? ::sus::mem::data_size_of<T>()
                                     : ::sus::mem::size_of<T>();
    return format_bytes<Char>(
        ctx.out(),
        fmt::string_view(reinterpret_cast<const char*>(&t), num_bytes));
  }
};

}  // namespace sus::string::__private
