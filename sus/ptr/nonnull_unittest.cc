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

#include "sus/ptr/nonnull.h"

#include <sstream>

#include "googletest/include/gtest/gtest.h"
#include "sus/construct/into.h"
#include "sus/macros/__private/compiler_bugs.h"
#include "sus/mem/forward.h"
#include "sus/mem/never_value.h"
#include "sus/mem/relocate.h"
#include "sus/num/types.h"
#include "sus/ops/eq.h"
#include "sus/ops/ord.h"
#include "sus/option/option.h"
#include "sus/prelude.h"

using sus::ptr::NonNull;

static_assert(sus::mem::relocate_by_memcpy<NonNull<int>>);

static_assert(sus::mem::NeverValueField<NonNull<int>>);
static_assert(sizeof(sus::Option<NonNull<int>>) == sizeof(int*));

namespace {

template <class T, class From>
concept CanConstruct = requires(From f) { NonNull<T>(f); };
template <class T, class From>
concept CanConstructPtr = requires(From f) { NonNull<T>::with_ptr(f); };
template <class T, class From, size_t N>
concept CanConstructPtrArray =
    requires(From (&f)[N]) { NonNull<T>::with_ptr(f); };
template <class T, class From>
concept CanFrom = requires(From f) { NonNull<T>::from(f); };
template <class T, class From, size_t N>
concept CanFromArray = requires(From (&f)[N]) { NonNull<T>::from(f); };

// NonNull can be constructed from a reference or pointer.
static_assert(CanConstruct<int, int&>);
static_assert(CanConstructPtr<int, int*>);
static_assert(CanFrom<int, int&>);

// NonNull does not erase const.
static_assert(!CanConstruct<int, const int&>);
static_assert(!CanConstructPtr<int, const int*>);
static_assert(!CanFrom<int, const int&>);
static_assert(!CanFrom<int, const int*>);

// NonNull can upgrade to const.
static_assert(CanConstruct<const int, int&>);
static_assert(CanConstructPtr<const int, int*>);
static_assert(CanFrom<const int, int&>);

// NonNull can't be constructed from an array directly.
static_assert(!CanConstruct<int, int (&)[2]>);
static_assert(!CanConstructPtr<int, int (&)[2]>);
static_assert(!CanFrom<int, int (&)[2]>);

TEST(NonNull, ConstructRef) {
  int i = 1;
  const int c = 2;
  auto n1 = NonNull<int>(i);
  auto n2 = NonNull<const int>(i);
  auto c1 = NonNull<const int>(c);

  EXPECT_EQ(&i, &n1.as_ref());
  EXPECT_EQ(&i, &n2.as_ref());
  EXPECT_EQ(&c, &c1.as_ref());
}

TEST(NonNull, AddressOf) {
  struct S {
    S* operator&() const { return nullptr; }
  } s;
  auto n1 = NonNull<S>(s);
  auto n2 = NonNull<const S>(s);

  EXPECT_EQ(&s, &n1.as_ref());
  EXPECT_EQ(&s, &n2.as_ref());
}

TEST(NonNull, ConstructPtr) {
  int i = 1;
  const int c = 2;
  auto n1 = NonNull<int>::with_ptr(&i).unwrap();
  auto n2 = NonNull<const int>::with_ptr(&i).unwrap();
  auto c1 = NonNull<const int>::with_ptr(&c).unwrap();

  EXPECT_EQ(&i, &n1.as_ref());
  EXPECT_EQ(&i, &n2.as_ref());
  EXPECT_EQ(&c, &c1.as_ref());
}

TEST(NonNull, ConstructPtrUnchecked) {
  int i = 1;
  const int c = 2;
  auto n1 = NonNull<int>::with_ptr_unchecked(unsafe_fn, &i);
  auto n2 = NonNull<const int>::with_ptr_unchecked(unsafe_fn, &i);
  auto c1 = NonNull<const int>::with_ptr_unchecked(unsafe_fn, &c);

  EXPECT_EQ(&i, &n1.as_ref());
  EXPECT_EQ(&i, &n2.as_ref());
  EXPECT_EQ(&c, &c1.as_ref());
}

TEST(NonNull, From) {
  int i = 1;
  const int c = 2;
  {
    auto n1 = NonNull<int>::from(i);
    auto n2 = NonNull<const int>::from(i);
    auto c1 = NonNull<const int>::from(c);

    EXPECT_EQ(&i, &n1.as_ref());
    EXPECT_EQ(&i, &n2.as_ref());
    EXPECT_EQ(&c, &c1.as_ref());
  }
  {
    NonNull<int> n1 = sus::into(i);
    NonNull<const int> n2 = sus::into(i);
    NonNull<const int> c1 = sus::into(c);

    EXPECT_EQ(&i, &n1.as_ref());
    EXPECT_EQ(&i, &n2.as_ref());
    EXPECT_EQ(&c, &c1.as_ref());
  }
}

TEST(NonNull, AsRef) {
  int i = 1;

  auto n1 = NonNull<int>(i);
  static_assert(std::same_as<decltype(n1.as_ref()), const int&>);
  EXPECT_EQ(&i, &n1.as_ref());

  auto n2 = NonNull<const int>(i);
  static_assert(std::same_as<decltype(n2.as_ref()), const int&>);
  EXPECT_EQ(&i, &n2.as_ref());
}

template <class T>
concept AsRefMutExists = requires(T& t) { t.as_mut(); };

TEST(NonNull, AsRefMut) {
  int i = 1;

  auto n1 = NonNull<int>(i);
  static_assert(std::same_as<decltype(n1.as_mut()), int&>);
  static_assert(AsRefMutExists<decltype(n1)>);
  EXPECT_EQ(&i, &n1.as_mut());

  auto n2 = NonNull<const int>(i);
  static_assert(!AsRefMutExists<decltype(n2)>);
}

TEST(NonNull, AsPtr) {
  int i = 1;

  auto n1 = NonNull<int>(i);
  static_assert(std::same_as<decltype(n1.as_ptr()), const int*>);
  EXPECT_EQ(&i, n1.as_ptr());

  auto n2 = NonNull<const int>(i);
  static_assert(std::same_as<decltype(n2.as_ptr()), const int*>);
  EXPECT_EQ(&i, n2.as_ptr());
}

template <class T>
concept AsPtrMutExists = requires(T& t) { t.as_mut_ptr(); };

TEST(NonNull, AsPtrMut) {
  int i = 1;

  auto n1 = NonNull<int>(i);
  static_assert(std::same_as<decltype(n1.as_mut_ptr()), int*>);
  static_assert(AsPtrMutExists<decltype(n1)>);
  EXPECT_EQ(&i, n1.as_mut_ptr());

  auto n2 = NonNull<const int>(i);
  static_assert(!AsPtrMutExists<decltype(n2)>);
}

TEST(NonNull, Cast) {
  struct Base {
    i32 i;
  };
  struct Sub : public Base {};
  Sub s;
  s.i = 3_i32;
  auto sn = NonNull<Sub>(s);
  auto bn = sn.cast<Base>();
  EXPECT_EQ(bn.as_ref().i, 3_i32);
}

TEST(NonNull, Downcast) {
  struct Base {
    i32 i;
  };
  struct Sub : public Base {};
  Sub s;
  s.i = 3_i32;
  auto bn = NonNull<Base>(s);
  auto sn = bn.downcast<Sub>(unsafe_fn);
  EXPECT_EQ(sn.as_ref().i, 3_i32);
}

TEST(NonNull, Eq) {
  struct NotEq {};
  static_assert(sus::ops::Eq<NonNull<int>>);
  static_assert(!sus::ops::Eq<NonNull<int>, NonNull<NotEq>>);

  auto a = int();
  auto b = int();
  EXPECT_EQ(NonNull<int>(a), NonNull<int>(a));
  EXPECT_NE(NonNull<int>(a), NonNull<int>(b));

  struct Base {};
  struct Sub : public Base {};
  auto s = Sub();
  auto s2 = Sub();
  EXPECT_EQ(NonNull<Sub>(s), NonNull<Base>(static_cast<Base&>(s)));
  EXPECT_NE(NonNull<Sub>(s), NonNull<Base>(static_cast<Base&>(s2)));
}

TEST(NonNull, StrongOrd) {
  struct NotCmp {};
  static_assert(sus::ops::StrongOrd<NonNull<int>>);
  sus_gcc_bug_107542_else(
      static_assert(!sus::ops::StrongOrd<NonNull<int>, NonNull<NotCmp>>));

  int a[] = {1, 2};
  EXPECT_LE(NonNull<int>(a[0]), NonNull<int>(a[0]));
  EXPECT_LT(NonNull<int>(a[0]), NonNull<int>(a[1]));

  struct Base {};
  struct Sub : public Base {};
  Sub s[] = {Sub(), Sub()};
  EXPECT_LE(NonNull<Sub>(s[0]),
            NonNull<Base>(static_cast<Base&>(s[0])));
  EXPECT_LT(NonNull<Sub>(s[0]),
            NonNull<Base>(static_cast<Base&>(s[1])));
}

TEST(NonNull, OperatorArrow) {
  struct S {
    i32 i = 3;
  } s;

  // Mutable.
  {
    auto n = NonNull<S>(s);
    n->i += 1;
    EXPECT_EQ(n.operator->(), &s);
    EXPECT_EQ(s.i, 4);
  }

  // Const.
  {
    const S& cs = s;
    auto n = NonNull<const S>(cs);
    EXPECT_EQ(n.operator->(), &cs);
    EXPECT_EQ(n->i, 4);
  }
}

TEST(NonNull, fmt) {
  i32 i = 3;
  auto nm = NonNull<i32>(i);

  EXPECT_EQ(fmt::format("{}", nm), fmt::format("{}", fmt::ptr(&i)));
}

TEST(NonNull, Stream) {
  i32 i = 3;
  auto nm = NonNull<i32>(i);

  std::stringstream s;
  s << nm;
  EXPECT_EQ(s.str(), fmt::format("{}", fmt::ptr(&i)));
}

TEST(NonNull, GTest) {
  i32 i = 3;
  auto nm = NonNull<i32>(i);

  EXPECT_EQ(testing::PrintToString(nm), fmt::format("{}", fmt::ptr(&i)));
}

}  // namespace
