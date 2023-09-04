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

#include "sus/string/compat_string.h"

#include "googletest/include/gtest/gtest.h"
#include "sus/iter/compat_ranges.h"
#include "sus/prelude.h"

namespace {

static_assert(sus::iter::FromIterator<std::string, char>);
static_assert(sus::iter::FromIterator<std::u8string, char8_t>);
static_assert(sus::iter::FromIterator<std::u16string, char16_t>);
static_assert(sus::iter::FromIterator<std::u32string, char32_t>);

template <class Out, class Char>
Out construct(const Char* s) {
  Out o;
  while (*s) {
    o.push_back(*s);
    s += 1u;
  }
  return o;
}

TEST(CompatString, Char) {
  auto in = std::vector<char>{'a', 'b', 'c', 'd'};
  auto out = sus::iter::from_range(sus::move(in))
                 .filter([](char i) { return i != 'c'; })
                 .moved()
                 .collect<std::string>();
  sus::check(out == construct<std::string>("abd"));
}

TEST(CompatString, Char8) {
  auto in = std::vector<char8_t>{'a', 'b', 'c', 'd'};
  auto out = sus::iter::from_range(sus::move(in))
                 .filter([](char8_t i) { return i != 'c'; })
                 .moved()
                 .collect<std::u8string>();
  sus::check(out == construct<std::u8string>("abd"));
}

TEST(CompatString, Char16) {
  auto in = std::vector<char16_t>{'a', 'b', 'c', 'd'};
  auto out = sus::iter::from_range(sus::move(in))
                 .filter([](char16_t i) { return i != 'c'; })
                 .moved()
                 .collect<std::u16string>();
  sus::check(out == construct<std::u16string>("abd"));
}

TEST(CompatString, Char32) {
  auto in = std::vector<char32_t>{'a', 'b', 'c', 'd'};
  auto out = sus::iter::from_range(sus::move(in))
                 .filter([](char32_t i) { return i != 'c'; })
                 .moved()
                 .collect<std::u32string>();
  sus::check(out == construct<std::u32string>("abd"));
}

}  // namespace
