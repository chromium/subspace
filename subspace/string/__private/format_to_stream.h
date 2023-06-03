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

#include "fmt/core.h"
#include "subspace/macros/for_each.h"

#define sus__format_to_stream_add_class(x) class x

/// Defines operator<< for type `Type` in namespace `Namespace`. The operator
/// uses fmt::formatter<T, char> to generate a string and stream it.
#define sus__format_to_stream(Namespace, Type, ...)                            \
  namespace Namespace {                                                        \
  __VA_OPT__(template <sus_for_each(sus__format_to_stream_add_class,           \
                                    sus_for_each_sep_comma, __VA_ARGS__)>      \
  )                                                                            \
  inline std::basic_ostream<char>& operator<<(                                 \
      std::basic_ostream<char>& stream,                                        \
      const Type __VA_OPT__(<__VA_ARGS__>) & value) {                          \
    static_assert(fmt::is_formattable<Type __VA_OPT__(<__VA_ARGS__>)>::value); \
    return ::sus::string::__private::format_to_stream(                         \
        stream, fmt::format("{}", value));                                     \
  }                                                                            \
  }

namespace sus::string::__private {

/// Consumes the string `s` and streams it to the ostream `os`.
std::ostream& format_to_stream(std::ostream& os, const std::string& s);

}  // namespace sus::string::__private
