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

#include "subspace/mem/nonnull.h"

#include "googletest/include/gtest/gtest.h"
#include "subspace/construct/into.h"
#include "subspace/macros/__private/compiler_bugs.h"
#include "subspace/mem/forward.h"
#include "subspace/mem/never_value.h"
#include "subspace/mem/relocate.h"
#include "subspace/num/types.h"
#include "subspace/ops/eq.h"
#include "subspace/ops/ord.h"
#include "subspace/option/option.h"
#include "subspace/prelude.h"

using sus::mem::NonNull;

static_assert(sus::mem::relocate_one_by_memcpy<NonNull<int>>);
static_assert(sus::mem::relocate_array_by_memcpy<NonNull<int>>);

static_assert(sus::mem::NeverValueField<NonNull<int>>);
static_assert(sizeof(sus::Option<NonNull<int>>) == sizeof(int*));

namespace {

template <class T, class From>
concept CanConstruct = requires(From f) { NonNull<T>::with(f); };
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
static_assert(CanFrom<int, int*>);

// NonNull does not erase const.
static_assert(!CanConstruct<int, const int&>);
static_assert(!CanConstructPtr<int, const int*>);
static_assert(!CanFrom<int, const int&>);
static_assert(!CanFrom<int, const int*>);

// NonNull can upgrade to const.
static_assert(CanConstruct<const int, int&>);
static_assert(CanConstructPtr<const int, int*>);
static_assert(CanFrom<const int, int&>);
static_assert(CanFrom<const int, int*>);

// NonNull can't be constructed from an array directly.
static_assert(!CanConstruct<int, int (&)[2]>);
static_assert(!CanConstructPtr<int, int (&)[2]>);
static_assert(!CanFrom<int, int (&)[2]>);

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

TEST(NonNull, FromRvalue) {
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

TEST(NonNull, FromLvalue) {
  int i = 1;
  int* ip = &i;
  const int c = 2;
  const int* cp = &c;
  {
    auto n1 = NonNull<int>::from(ip);
    auto n2 = NonNull<const int>::from(ip);
    auto c1 = NonNull<const int>::from(cp);

    EXPECT_EQ(&i, &n1.as_ref());
    EXPECT_EQ(&i, &n2.as_ref());
    EXPECT_EQ(&c, &c1.as_ref());
  }
  {
    NonNull<int> n1 = sus::into(ip);
    NonNull<const int> n2 = sus::into(ip);
    NonNull<const int> c1 = sus::into(cp);

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
concept AsRefMutExists = requires(T t) { t.as_mut(); };

TEST(NonNull, AsRefMut) {
  int i = 1;

  auto n1 = NonNull<int>::with(i);
  static_assert(std::same_as<decltype(n1.as_mut()), int&>);
  static_assert(AsRefMutExists<decltype(n1)>);
  EXPECT_EQ(&i, &n1.as_mut());

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
concept AsPtrMutExists = requires(T t) { t.as_mut_ptr(); };

TEST(NonNull, AsPtrMut) {
  int i = 1;

  auto n1 = NonNull<int>::with(i);
  static_assert(std::same_as<decltype(n1.as_mut_ptr()), int*>);
  static_assert(AsPtrMutExists<decltype(n1)>);
  EXPECT_EQ(&i, n1.as_mut_ptr());

  auto n2 = NonNull<const int>::with(i);
  static_assert(!AsPtrMutExists<decltype(n2)>);
}

TEST(NonNull, Cast) {
  struct Base {
    i32 i;
  };
  struct Sub : public Base {};
  Sub s;
  s.i = 3_i32;
  auto sn = NonNull<Sub>::with(s);
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
  auto bn = NonNull<Base>::with(s);
  auto sn = bn.downcast<Sub>(unsafe_fn);
  EXPECT_EQ(sn.as_ref().i, 3_i32);
}

TEST(NonNull, Eq) {
  struct NotEq {};
  static_assert(sus::ops::Eq<NonNull<int>>);
  static_assert(!sus::ops::Eq<NonNull<int>, NonNull<NotEq>>);

  auto a = int();
  auto b = int();
  EXPECT_EQ(NonNull<int>::with(a), NonNull<int>::with(a));
  EXPECT_NE(NonNull<int>::with(a), NonNull<int>::with(b));

  struct Base {};
  struct Sub : public Base {};
  auto s = Sub();
  auto s2 = Sub();
  EXPECT_EQ(NonNull<Sub>::with(s), NonNull<Base>::with(static_cast<Base&>(s)));
  EXPECT_NE(NonNull<Sub>::with(s), NonNull<Base>::with(static_cast<Base&>(s2)));
}

TEST(NonNull, Ord) {
  struct NotCmp {};
  static_assert(sus::ops::Ord<NonNull<int>>);
  sus_gcc_bug_107542_else(
      static_assert(!sus::ops::Ord<NonNull<int>, NonNull<NotCmp>>));

  int a[] = {1, 2};
  EXPECT_LE(NonNull<int>::with(a[0]), NonNull<int>::with(a[0]));
  EXPECT_LT(NonNull<int>::with(a[0]), NonNull<int>::with(a[1]));

  struct Base {};
  struct Sub : public Base {};
  Sub s[] = {Sub(), Sub()};
  EXPECT_LE(NonNull<Sub>::with(s[0]),
            NonNull<Base>::with(static_cast<Base&>(s[0])));
  EXPECT_LT(NonNull<Sub>::with(s[0]),
            NonNull<Base>::with(static_cast<Base&>(s[1])));
}

}  // namespace
