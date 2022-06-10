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

#include "mem/addressof.h"

#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace {

using sus::mem::addressof;

TEST(AddressOf, Object) {
  struct S {
    int i;
  };
  S s;
  static_assert(std::is_object_v<decltype(s)>, "");
  EXPECT_EQ(::sus::mem::addressof(s), &s);

  const S cs(0);
  static_assert(std::is_object_v<decltype(cs)>, "");
  EXPECT_EQ(::sus::mem::addressof(cs), &cs);
}

TEST(AddressOf, OperatorOverride) {
  struct T {
    int i;
    unsigned char* operator&() const { return 0; }
  };
  struct S {
    T t;
  };
  S s;
  EXPECT_EQ(0, &s.t);
  EXPECT_EQ(reinterpret_cast<unsigned char*>(::sus::mem::addressof(s.t)),
            reinterpret_cast<unsigned char*>(&s));
}

TEST(AddressOf, NonObject) {
  struct S {};
  S s;
  S& r = s;
  static_assert(!std::is_object_v<decltype(r)>, "");
  EXPECT_EQ(::sus::mem::addressof(r), &s);

  const S& cr = s;
  static_assert(!std::is_object_v<decltype(cr)>, "");
  EXPECT_EQ(::sus::mem::addressof(cr), &s);
}

}  // namespace
