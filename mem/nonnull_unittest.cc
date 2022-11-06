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

#include "mem/nonnull.h"

#include "construct/into.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace {

using sus::mem::NonNull;

TEST(NonNull, ConstructRef) {
  int i = 1;
  const int c = 2;
  auto n1 = NonNull<int>::with(i);
  auto n2 = NonNull<const int>::with(i);
  auto c1 = NonNull<const int>::with(c);

  EXPECT_EQ(&i, &n1.as_ref());
  EXPECT_EQ(&i, &n2.as_ref());
  EXPECT_EQ(&c, &c1.as_ref());
}

TEST(NonNull, AddressOf) {
  struct S {
    S* operator&() const { return nullptr; }
  } s;
  auto n1 = NonNull<S>::with(s);
  auto n2 = NonNull<const S>::with(s);

  EXPECT_EQ(&s, &n1.as_ref());
  EXPECT_EQ(&s, &n2.as_ref());
}

TEST(NonNull, ConstructPtr) {
  int i = 1;
  const int c = 2;
  auto n1 = NonNull<int>::with_ptr(&i);
  auto n2 = NonNull<const int>::with_ptr(&i);
  auto c1 = NonNull<const int>::with_ptr(&c);

  EXPECT_EQ(&i, &n1.as_ref());
  EXPECT_EQ(&i, &n2.as_ref());
  EXPECT_EQ(&c, &c1.as_ref());
}

TEST(NonNull, From) {
  int i = 1;
  const int c = 2;
  {
    auto n1 = NonNull<int>::from(&i);
    auto n2 = NonNull<const int>::from(&i);
    auto c1 = NonNull<const int>::from(&c);

    EXPECT_EQ(&i, &n1.as_ref());
    EXPECT_EQ(&i, &n2.as_ref());
    EXPECT_EQ(&c, &c1.as_ref());
  }
  {
    NonNull<int> n1 = sus::into(&i);
    NonNull<const int> n2 = sus::into(&i);
    NonNull<const int> c1 = sus::into(&c);

    EXPECT_EQ(&i, &n1.as_ref());
    EXPECT_EQ(&i, &n2.as_ref());
    EXPECT_EQ(&c, &c1.as_ref());
  }
}

TEST(NonNull, AsRef) {
  int i = 1;

  auto n1 = NonNull<int>::with(i);
  static_assert(std::same_as<decltype(n1.as_ref()), const int&>);
  EXPECT_EQ(&i, &n1.as_ref());

  auto n2 = NonNull<const int>::with(i);
  static_assert(std::same_as<decltype(n2.as_ref()), const int&>);
  EXPECT_EQ(&i, &n2.as_ref());
}

template <class T>
concept AsRefMutExists = requires(T t) { t.as_ref_mut(); };

TEST(NonNull, AsRefMut) {
  int i = 1;

  auto n1 = NonNull<int>::with(i);
  static_assert(std::same_as<decltype(n1.as_ref_mut()), int&>);
  static_assert(AsRefMutExists<decltype(n1)>);
  EXPECT_EQ(&i, &n1.as_ref_mut());

  auto n2 = NonNull<const int>::with(i);
  static_assert(!AsRefMutExists<decltype(n2)>);
}

TEST(NonNull, AsPtr) {
  int i = 1;

  auto n1 = NonNull<int>::with(i);
  static_assert(std::same_as<decltype(n1.as_ptr()), const int*>);
  EXPECT_EQ(&i, n1.as_ptr());

  auto n2 = NonNull<const int>::with(i);
  static_assert(std::same_as<decltype(n2.as_ptr()), const int*>);
  EXPECT_EQ(&i, n2.as_ptr());
}

template <class T>
concept AsPtrMutExists = requires(T t) { t.as_ptr_mut(); };

TEST(NonNull, AsPtrMut) {
  int i = 1;

  auto n1 = NonNull<int>::with(i);
  static_assert(std::same_as<decltype(n1.as_ptr_mut()), int*>);
  static_assert(AsPtrMutExists<decltype(n1)>);
  EXPECT_EQ(&i, n1.as_ptr_mut());

  auto n2 = NonNull<const int>::with(i);
  static_assert(!AsPtrMutExists<decltype(n2)>);
}

}  // namespace
