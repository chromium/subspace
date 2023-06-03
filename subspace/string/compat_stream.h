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

// TODO: Overload all && functions with const& version if T is copyable? The
// latter copies T instead of moving it. This could lead to a lot of unintended
// copies if expensive types have copy constructors, which is common in
// preexisting C++ code since there's no concept of Clone there (which will TBD
// in this library). So it's not clear if this is the right thing to do
// actually, needs thought.

#pragma once

#include <concepts>
#include <ostream>

#include "fmt/core.h"

#define sus__stream(n)                                                         \
  namespace n {                                                                \
  template <class Char, class T>                                               \
    requires(fmt::is_formattable<T>::value)                                    \
  inline std::basic_ostream<Char>& operator<<(std::basic_ostream<Char>& s,     \
                                              const T& t) {                    \
    /* Move to the `std` namespace to stream std::string without recursing. */ \
    std::operator<<(s, fmt::format("{}", t));                                  \
    return s;                                                                  \
  }                                                                            \
  }                                                                            \
  static_assert(true)

// This file defines operator<< for all formattable subspace types by
// placing it in the same namespace as those types. So we must define it for
// every sub namespace.

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
