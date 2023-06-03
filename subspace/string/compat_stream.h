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

#include <concepts>
#include <iosfwd>

#include "fmt/core.h"
#include "subspace/string/__private/format_to_stream.h"

#define sus__stream(n)                                                      \
  namespace n {                                                             \
  template <class T>                                                        \
    requires(fmt::is_formattable<T>::value)                                 \
  inline std::basic_ostream<char>& operator<<(std::basic_ostream<char>& s,  \
                                              const T& t) {                 \
    return ::sus::string::__private::format_to_stream(fmt::format("{}", t), \
                                                      s);                   \
  }                                                                         \
  }                                                                         \
  static_assert(true)

// This file defines operator<< for all formattable subspace types by
// placing it in the same namespace as those types. So we must define it for
// every namespace.

sus__stream(sus::assertions);
sus__stream(sus::choice_type);
sus__stream(sus::construct);
sus__stream(sus::convert);
sus__stream(sus::fn);
sus__stream(sus::iter);
sus__stream(sus::marker);
sus__stream(sus::mem);
sus__stream(sus::num);
sus__stream(sus::ops);
sus__stream(sus::option);
sus__stream(sus::ptr);
sus__stream(sus::result);
sus__stream(sus::string);
sus__stream(sus::tuple_type);
