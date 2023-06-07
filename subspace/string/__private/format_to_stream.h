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

#include <iosfwd>
#include <string>

#include "fmt/core.h"
#include "subspace/macros/for_each.h"

namespace sus::string::__private {

template <class To, class From>
concept ConvertibleFrom = std::convertible_to<From, To>;

template <class T, class Char>
concept StreamCanReceiveString = requires(T& t, std::basic_string<Char> s) {
  // Check ConvertibleFrom as std streams return std::basic_ostream&, which the
  // input type `T&` is convertible to. The concept ordering means we want
  // `ConvertibleFrom<T&, ..the output type..>` to be true then.
  { t << s } -> ConvertibleFrom<T&>;
};

/// Consumes the string `s` and streams it to the output stream `os`.
template <class Char, StreamCanReceiveString<Char> S>
S& format_to_stream(S& os, const std::basic_string<Char>& s) {
  os << s;
  return os;
}

}  // namespace sus::string::__private

#define sus__format_to_stream_add_class(x) class x

/// Defines operator<< for type `Type` in namespace `Namespace`. The operator
/// uses fmt::formatter<T, char> to generate a string and stream it.
#define sus__format_to_stream(Namespace, Type, ...)                            \
  namespace Namespace {                                                        \
  using namespace ::sus::string::__private;                                    \
  template <StreamCanReceiveString<char> S __VA_OPT__(, ) sus_for_each(        \
      sus__format_to_stream_add_class, sus_for_each_sep_comma, __VA_ARGS__)>   \
  inline S& operator<<(S& stream,                                              \
                       const Type __VA_OPT__(<__VA_ARGS__>) & value) {         \
    static_assert(fmt::is_formattable<Type __VA_OPT__(<__VA_ARGS__>)>::value); \
    return ::sus::string::__private::format_to_stream(                         \
        stream, fmt::format("{}", value));                                     \
  }                                                                            \
  }
