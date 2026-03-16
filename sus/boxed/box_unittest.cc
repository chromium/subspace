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

#ifdef TEST_MODULE
import sus;

#  include "fmt/format.h"
#  include "sus/assertions/check.h"
#else
#  include "sus/boxed/box.h"
#  include "sus/error/error.h"
#  include "sus/iter/iterator.h"
#  include "sus/iter/once.h"
#  include "sus/ops/range.h"
#  include "sus/prelude.h"
#endif

#include "googletest/include/gtest/gtest.h"

namespace test::box {
struct MyError {};

struct SuperType {
  constexpr virtual ~SuperType() = default;
  constexpr virtual std::string name() { return "SuperType"; }
};
struct SubType : public SuperType {
  ~SubType() override = default;
  constexpr std::string name() override { return "SubType"; }
};

}  // namespace test::box

using namespace test::box;

template <>
struct sus::error::ErrorImpl<MyError> {
  constexpr static std::string display(const MyError&) noexcept {
    return "my error";
  }
};

namespace {
using sus::boxed::Box;

static_assert(sus::mem::Clone<Box<i32>>);
static_assert(sus::mem::CloneFrom<Box<i32>>);

// For any T.
static_assert(sus::mem::NeverValueField<Box<i32>>);
// For any T.
static_assert(sus::mem::TriviallyRelocatable<Box<i32>>);

TEST(Box, RecursiveType) {
  struct Cycle {
    sus::Option<Box<Cycle>> b;
  };
  // This fails (on MSVC at least) when Box is default-constructible. I suspect
  // the compiler walks an infinite recursive loop when this is the case, though
  // Option's default constructor does not construct a `Box<Cycle>`.
  static_assert(std::destructible<Cycle>);

  auto c = Cycle();
  c.b.insert(Box<Cycle>(Cycle()));
}

TEST(Box, Construct) {
  i32 i = 3;
  {
    auto b = Box<i32>(i);
    EXPECT_EQ(*b, 3);
  }
  {
    auto b = Box<SuperType>(SubType());
    EXPECT_EQ(b->name(), "SubType");
  }

  static_assert([] {
    auto b = Box<i32>(3_i32);
    return *b == 3;
  }());
  static_assert([] {
    auto b = Box<SuperType>(SubType());
    return b->name() == "SubType";
  }());
}

TEST(Box, Default) {
  auto b = Box<i32>::with_default();
  EXPECT_EQ(*b, 0);

  static_assert([] {
    auto b = Box<i32>::with_default();
    return *b == 0;
  }());
}

TEST(Box, WithArgs) {
  struct NoMove {
    constexpr NoMove(i32 i) : i(i) {}

    NoMove(NoMove&&) = delete;
    NoMove& operator=(NoMove&&) = delete;

    i32 i;
  };
  static_assert(!sus::mem::Move<NoMove>);

  auto b = Box<NoMove>::with_args(3);
  EXPECT_EQ(b->i, 3);

  auto b2 = sus::move(b);
  EXPECT_EQ(b2->i, 3);

  static_assert([] {
    auto b = Box<NoMove>::with_args(3);
    auto b2 = sus::move(b);
    return b2->i == 3;
  }());
}

TEST(Box, FromT) {
  i32 i = 3;
  {
    auto b = Box<i32>::from(i);
    EXPECT_EQ(*b, 3);
  }
  {
    Box<i32> b = sus::into(i);
    EXPECT_EQ(*b, 3);
  }
  {
    auto b = Box<SuperType>::from(SubType());
    EXPECT_EQ(b->name(), "SubType");
  }
  {
    Box<SuperType> b = sus::into(SubType());
    EXPECT_EQ(b->name(), "SubType");
  }

  static_assert([] {
    auto b = Box<i32>::from(3_i32);
    return *b == 3;
  }());
  static_assert([] {
    auto b = Box<SuperType>::from(SubType());
    return b->name() == "SubType";
  }());
}

TEST(Box, Clone) {
  static i32 cloned;
  struct Cloneable {
    explicit Cloneable(i32 i) : i(i) { }
    Cloneable(Cloneable&& r) = default;
    Cloneable& operator=(Cloneable&&) = default;

    Cloneable clone() const noexcept {
      cloned += 1;
      return Cloneable(i);
    }
    void clone_from(const Cloneable& r) noexcept {
      cloned += 1;
      i = r.i;
    }

    i32 i;
  };
  static_assert(sus::mem::Clone<Cloneable>);
  static_assert(sus::mem::CloneFrom<Cloneable>);
  struct ConstCloneable {
    constexpr explicit ConstCloneable(i32 i) : i(i) {}
    constexpr ConstCloneable(ConstCloneable&& r) = default;
    ConstCloneable& operator=(ConstCloneable&&) = default;

    constexpr ConstCloneable clone() const noexcept {
      return ConstCloneable(i);
    }
    constexpr void clone_from(const ConstCloneable& r) noexcept { i = r.i; }

    i32 i;
  };
  static_assert(sus::mem::Clone<ConstCloneable>);
  static_assert(sus::mem::CloneFrom<ConstCloneable>);

  {
    auto b = Box<Cloneable>(Cloneable(2));
    EXPECT_EQ(cloned, 0);
    auto c = sus::clone(b);
    EXPECT_EQ(cloned, 1);
    EXPECT_EQ(c->i, 2);
  }
  EXPECT_EQ(cloned, 1);

  static_assert([] {
    auto b = Box<ConstCloneable>(ConstCloneable(2));
    auto c = sus::clone(b);
    return c->i == 2;
  }());
}

TEST(Box, CloneInto) {
  static i32 cloned;
  static i32 alloced;
  struct Cloneable {
    explicit Cloneable(i32 i) : i(i) { alloced += 1; }
    Cloneable(Cloneable&& r) : i(r.i) { alloced += 1; }
    Cloneable& operator=(Cloneable&&) = default;

    Cloneable clone() const noexcept {
      cloned += 1;
      return Cloneable(i);
    }
    void clone_from(const Cloneable& r) noexcept {
      cloned += 1;
      i = r.i;
    }

    i32 i;
  };
  static_assert(sus::mem::Clone<Cloneable>);
  static_assert(sus::mem::CloneFrom<Cloneable>);
  struct ConstCloneable {
    constexpr explicit ConstCloneable(i32 i) : i(i) {}
    constexpr ConstCloneable(ConstCloneable&& r) = default;
    ConstCloneable& operator=(ConstCloneable&&) = default;

    constexpr ConstCloneable clone() const noexcept {
      return ConstCloneable(i);
    }
    constexpr void clone_from(const ConstCloneable& r) noexcept { i = r.i; }

    i32 i;
  };
  static_assert(sus::mem::Clone<ConstCloneable>);
  static_assert(sus::mem::CloneFrom<ConstCloneable>);

  {
    auto b = Box<Cloneable>(Cloneable(2));
    auto c = Box<Cloneable>(Cloneable(3));
    EXPECT_EQ(cloned, 0);
    EXPECT_EQ(alloced, 4);
    sus::clone_into(b, c);
    EXPECT_EQ(cloned, 1);
    EXPECT_EQ(alloced, 4);  // No new alloc.
    EXPECT_EQ(b->i, 3);
  }
  EXPECT_EQ(cloned, 1);

  static_assert([] {
    auto b = Box<ConstCloneable>(ConstCloneable(2));
    auto c = Box<ConstCloneable>(ConstCloneable(3));
    sus::clone_into(b, c);
    return b->i == 3;
  }());
}

TEST(Box, MoveConstruct) {
  static i32 moved;
  static i32 destroyed;
  struct Moveable {
    ~Moveable() { destroyed += 1; }
    explicit Moveable(i32 i) : i(i) {}
    Moveable(Moveable&& r) : i(r.i) { moved += 1; }
    Moveable& operator=(Moveable&& r) { return i = r.i, moved += 1, *this; }

    i32 i;
  };
  struct ConstMoveable {
    constexpr ~ConstMoveable() = default;
    constexpr explicit ConstMoveable(i32 i) : i(i) {}
    constexpr ConstMoveable(ConstMoveable&& r) = default;
    constexpr ConstMoveable& operator=(ConstMoveable&& r) {
      return i = r.i, *this;
    }

    i32 i;
  };

  {
    auto b = Box<Moveable>(Moveable(2));
    // Moved into the heap.
    EXPECT_EQ(moved, 1);
    // The stack object is destroyed.
    EXPECT_EQ(destroyed, 1);
    auto c = sus::move(b);
    // The box moved but not the Movable; it's at a pinned location on the
    // heap.
    EXPECT_EQ(moved, 1);
    // The Moveable in `b` was not destroyed.
    EXPECT_EQ(destroyed, 1);
    EXPECT_EQ(c->i, 2);

#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(b->i += 1, "used after move");
#endif
  }
  EXPECT_EQ(moved, 1);
  EXPECT_EQ(destroyed, 2);

  // Upcasting.
  {
    auto b = Box<SubType>(SubType());
    auto c = Box<SuperType>(sus::move(b));
    EXPECT_EQ(c->name(), "SubType");
  }

  static_assert([] {
    auto b = Box<ConstMoveable>(ConstMoveable(2));
    auto c = sus::move(b);
    return c->i == 2;
  }());

  static_assert([] {
    auto b = Box<SubType>(SubType());
    auto c = Box<SuperType>(sus::move(b));
    return c->name() == "SubType";
  }());
}

TEST(Box, MoveAssign) {
  static i32 moved;
  static i32 destroyed;
  struct Moveable {
    ~Moveable() { destroyed += 1; }
    explicit Moveable(i32 i) : i(i) {}
    Moveable(Moveable&& r) : i(r.i) { moved += 1; }
    Moveable& operator=(Moveable&& r) { return i = r.i, moved += 1, *this; }

    i32 i;
  };
  struct ConstMoveable {
    constexpr ~ConstMoveable() = default;
    constexpr explicit ConstMoveable(i32 i) : i(i) {}
    constexpr ConstMoveable(ConstMoveable&& r) = default;
    constexpr ConstMoveable& operator=(ConstMoveable&& r) {
      return i = r.i, *this;
    }

    i32 i;
  };

  {
    auto b = Box<Moveable>(Moveable(2));
    auto c = Box<Moveable>(Moveable(3));
    // Moved into the heap.
    EXPECT_EQ(moved, 2);
    // The stack object is destroyed.
    EXPECT_EQ(destroyed, 2);
    c = sus::move(b);
    // The box moved but not the Movable; it's at a pinned location on the
    // heap.
    EXPECT_EQ(moved, 2);
    // The Moveable in `b` was not destroyed, but the one in `c` was.
    EXPECT_EQ(destroyed, 3);
    EXPECT_EQ(c->i, 2);

#if GTEST_HAS_DEATH_TEST
    EXPECT_DEATH(b->i += 1, "used after move");
#endif
  }
  EXPECT_EQ(moved, 2);
  EXPECT_EQ(destroyed, 4);

  // Upcasting.
  {
    auto b = Box<SubType>(SubType());
    auto c = Box<SuperType>(SuperType());
    c = sus::move(b);
    EXPECT_EQ(c->name(), "SubType");
  }

  static_assert([] {
    auto b = Box<ConstMoveable>(ConstMoveable(2));
    auto c = Box<ConstMoveable>(ConstMoveable(3));
    c = sus::move(b);
    return c->i == 2;
  }());

  static_assert([] {
    auto b = Box<SubType>(SubType());
    auto c = Box<SuperType>(SuperType());
    c = sus::move(b);
    return c->name() == "SubType";
  }());
}

// === AsRef

static_assert([] {
  auto* i = new i32(3);
  auto b = Box<i32>::from_raw(unsafe_fn, i);
  auto&& j = b.as_ref();
  static_assert(std::same_as<decltype(j), const i32&>);
  return i == &j;
}());

// === AsMut

static_assert([] {
  auto* i = new i32(3);
  auto b = Box<i32>::from_raw(unsafe_fn, i);
  auto&& j = b.as_mut();
  static_assert(std::same_as<decltype(j), i32&>);
  return i == &j;
}());

TEST(Box, IntoRaw) {
  static i32 deleted = 0;
  struct S {
    constexpr ~S() noexcept { deleted += 1; }
  };
  auto* i = new S();
  {
    auto b = Box<S>::from_raw(unsafe_fn, i);
    auto* j = sus::move(b).into_raw();
    EXPECT_EQ(i, j);
    EXPECT_EQ(deleted, 0);
    delete j;
    EXPECT_EQ(deleted, 1);
  }
  // `b` did not delete again.
  EXPECT_EQ(deleted, 1);
}

TEST(Box, Leak) {
  static i32 deleted = 0;
  struct S {
    constexpr ~S() noexcept { deleted += 1; }
  };
  auto* i = new S();
  {
    auto b = Box<S>::from_raw(unsafe_fn, i);
    auto& j = sus::move(b).leak();
    EXPECT_EQ(i, &j);
    EXPECT_EQ(deleted, 0);
    delete i;
    EXPECT_EQ(deleted, 1);
  }
  // `b` did not delete again.
  EXPECT_EQ(deleted, 1);
}

TEST(BoxDeathTest, UseAfterMove) {
  auto b = Box<i32>(2);
  auto c = sus::move(b);

#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH({ [[maybe_unused]] auto x = sus::clone(b); }, "used after move");
  EXPECT_DEATH(sus::clone_into(c, b), "used after move");
  EXPECT_DEATH({ [[maybe_unused]] auto x = sus::move(b); }, "used after move");
  EXPECT_DEATH({ c = sus::move(b); }, "used after move");
  EXPECT_DEATH(b.operator*() += 1, "used after move");
  EXPECT_DEATH(*b.operator->() += 1, "used after move");
#endif
}

// ==== OperatorStar

static_assert(std::same_as<decltype(*Box<i32>(3)), i32&>);
static_assert(std::same_as<decltype(*static_cast<const Box<i32>&>(Box<i32>(3))),
                           const i32&>);
static_assert((*Box<i32>(3)).wrapping_add(2) == 5);

// ==== OperatorArrow

static_assert(std::same_as<decltype(Box<i32>(3).operator->()), i32*>);
static_assert(std::same_as<
              decltype(static_cast<const Box<i32>&>(Box<i32>(3)).operator->()),
              const i32*>);
static_assert(Box<i32>(3)->wrapping_add(2) == 5);

TEST(Box, OperatorArrow) {
  auto b = Box<i32>(3);
  EXPECT_EQ(b->wrapping_add(2), 5);
}

// ==== Error

// Box<T> is not Error for non-Error T.
static_assert(!sus::error::Error<Box<i32>>);
// Box<T> is Error for Error T.
static_assert(sus::error::Error<Box<MyError>>);
static_assert(sus::error::Error<Box<sus::error::DynError>>);

// ==== FromError

TEST(BoxDynError, FromError) {
  {
    auto b = Box<sus::error::DynError>::from(MyError());
    EXPECT_EQ(sus::error::error_display(*b), "my error");
    EXPECT_EQ(sus::error::error_display(b), "my error");
  }
  {
    Box<sus::error::DynError> b = sus::into(MyError());
    static_assert(sus::error::Error<decltype(b)>);  // Box<DynError> is Error.
    EXPECT_EQ(sus::error::error_display(*b), "my error");
    EXPECT_EQ(sus::error::error_display(b), "my error");
  }
}

TEST(BoxDynError, FromString) {
  {
    auto b = Box<sus::error::DynError>::from("error string");
    static_assert(sus::error::Error<decltype(b)>);
    EXPECT_EQ(sus::error::error_display(*b), "error string");
    EXPECT_EQ(sus::error::error_display(b), "error string");
  }
  {
    auto b = Box<sus::error::DynError>::from(std::string("error string"));
    static_assert(sus::error::Error<decltype(b)>);
    EXPECT_EQ(sus::error::error_display(*b), "error string");
    EXPECT_EQ(sus::error::error_display(b), "error string");
  }
  {
    Box<sus::error::DynError> b = sus::into("error string");
    static_assert(sus::error::Error<decltype(b)>);  // Box<DynError> is Error.
    EXPECT_EQ(sus::error::error_display(*b), "error string");
    EXPECT_EQ(sus::error::error_display(b), "error string");
  }
  {
    Box<sus::error::DynError> b = sus::into(std::string("error string"));
    static_assert(sus::error::Error<decltype(b)>);  // Box<DynError> is Error.
    EXPECT_EQ(sus::error::error_display(*b), "error string");
    EXPECT_EQ(sus::error::error_display(b), "error string");
  }
}

TEST(BoxDynFn, Example_Call) {
  {
    const auto b = Box<sus::fn::DynFn<usize(std::string_view)>>::from(
        &std::string_view::size);
    sus_check(b("hello world") == 11u);

    auto mut_b = Box<sus::fn::DynFn<usize(std::string_view)>>::from(
        &std::string_view::size);
    sus_check(mut_b("hello world") == 11u);

    sus_check(sus::move(mut_b)("hello world") == 11u);
    // The object inside `mut_b` is now destroyed.
  }
  {
    auto mut_b = Box<sus::fn::DynFnMut<usize(std::string_view)>>::from(
        &std::string_view::size);
    sus_check(mut_b("hello world") == 11u);

    sus_check(sus::move(mut_b)("hello world") == 11u);
    // The object inside `mut_b` is now destroyed.
  }
  {
    auto b = Box<sus::fn::DynFnOnce<usize(std::string_view)>>::from(
        &std::string_view::size);
    sus_check(sus::move(b)("hello world") == 11u);

    auto x = [] {
      return Box<sus::fn::DynFnOnce<usize(std::string_view)>>::from(
          &std::string_view::size);
    };
    sus_check(x()("hello world") == 11u);
  }
}

// ==== fmt

TEST(Box, fmt) {
  static_assert(fmt::is_formattable<Box<i32>, char>::value);
  EXPECT_EQ(fmt::format("{}", Box<i32>(12345)), "12345");
  EXPECT_EQ(fmt::format("{:06}", Box<i32>(12345)), "012345");

  struct NoFormat {
    i32 a = 0x16ae3cf2;
  };
  static_assert(!fmt::is_formattable<NoFormat, char>::value);
  static_assert(fmt::is_formattable<Box<NoFormat>, char>::value);
  EXPECT_EQ(fmt::format("{}", Box<NoFormat>(NoFormat())), "f2-3c-ae-16");
  EXPECT_EQ(fmt::format("{}", sus::Option<NoFormat>()), "None");
}

TEST(Box, Stream) {
  std::stringstream s;
  s << Box<i32>(12345);
  EXPECT_EQ(s.str(), "12345");
}

TEST(Box, GTest) {
  EXPECT_EQ(testing::PrintToString(Box<i32>(12345)), "12345");
}

TEST(Box, Example_IntoRaw) {
  {
    auto x = Box<std::string>("Hello");
    auto* ptr = sus::move(x).into_raw();
    x = Box<std::string>::from_raw(unsafe_fn, ptr);
  }
  {
    auto x = Box<std::string>("Hello");
    auto* p = sus::move(x).into_raw();
    delete p;
  }
}

struct AnError {
  virtual ~AnError() = default;
  virtual std::string describe() const = 0;
};
struct Specific : public AnError {
  std::string describe() const override {
    return "specific problem has occurred";
  }
};

}  // namespace

template <>  // Implement `sus::error::Error` for AnError.
struct sus::error::ErrorImpl<AnError> {
  static std::string display(const AnError& e) { return e.describe(); }
};

namespace {

sus::Result<void, sus::Box<AnError>> always_error() {
  return sus::err(sus::into(Specific()));  // Deduces to Result<Box<AnError>>
};

TEST(BoxDeathTest, Example_ResultCustomHierarchy) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(always_error().unwrap(), "specific problem has occurred");
// Prints:
// PANIC! at 'specific problem has occurred', path/to/sus/result/result.h:790:11
#endif
}

// ==== Eq

struct NotEq {};
static_assert(sus::cmp::Eq<Box<i32>>);
static_assert(!sus::cmp::Eq<Box<NotEq>>);

static_assert(Box<i32>(3) == Box<i32>(3));
static_assert(Box<i32>(3) != Box<i32>(4));

// ==== Ord

struct Ordered {
  constexpr bool operator==(const Ordered&) const = default;
  constexpr std::weak_ordering operator<=>(const Ordered& rhs) const {
    return key <=> rhs.key;
  }

  i32 key;
  i32 value;
};

static_assert(sus::cmp::StrongOrd<Box<i32>>);
static_assert(sus::cmp::Ord<Box<Ordered>>);
static_assert(sus::cmp::PartialOrd<Box<f32>>);

static_assert(!sus::cmp::StrongOrd<Box<NotEq>>);
static_assert(!sus::cmp::Ord<Box<NotEq>>);
static_assert(!sus::cmp::PartialOrd<Box<NotEq>>);

static_assert(Box<i32>(3) <=> Box<i32>(3) == 0);
static_assert(Box<i32>(3) <=> Box<i32>(4) < 0);
static_assert(
    std::same_as<decltype(Box<i32>(3) <=> Box<i32>(3)), std::strong_ordering>);

static_assert(Box<Ordered>(Ordered{1, 1}) <=> Box<Ordered>(Ordered{1, 3}) == 0);
static_assert(Box<Ordered>(Ordered{1, 1}) <=> Box<Ordered>(Ordered{2, 1}) < 0);
static_assert(std::same_as<decltype(Box<Ordered>(Ordered{1, 1}) <=>
                                    Box<Ordered>(Ordered{1, 2})),
                           std::weak_ordering>);

static_assert(Box<f32>(3.f) <=> Box<f32>(3.f) == 0);
static_assert(Box<f32>(4.f) <=> Box<f32>(3.f) > 0);
static_assert(std::same_as<decltype(Box<f32>(3.f) <=> Box<f32>(3.f)),
                           std::partial_ordering>);

static_assert(sus::iter::Iterator<Box<decltype(sus::iter::once(2_i32))>, i32>);
static_assert(
    sus::iter::DoubleEndedIterator<Box<decltype(sus::iter::once(2_i32))>, i32>);
static_assert(
    sus::iter::ExactSizeIterator<Box<decltype(sus::iter::once(2_i32))>, i32>);
static_assert(sus::iter::TrustedLen<decltype(sus::iter::once(2_i32))>);
static_assert(sus::iter::TrustedLen<Box<decltype(sus::iter::once(2_i32))>>);

TEST(Box, Iterator) {
  auto b = sus::Box<sus::ops::Range<i32>>(sus::ops::range(0_i32, 3_i32));
  EXPECT_EQ(b.exact_size_hint(), 3u);
  EXPECT_EQ(b.next().unwrap(), 0);
  EXPECT_EQ(b.exact_size_hint(), 2u);
  EXPECT_EQ(b.next_back().unwrap(), 2);
  EXPECT_EQ(b.exact_size_hint(), 1u);
  EXPECT_EQ(b.next().unwrap(), 1);
  EXPECT_EQ(b.exact_size_hint(), 0u);
  EXPECT_EQ(b.next_back().is_none(), true);
  EXPECT_EQ(b.exact_size_hint(), 0u);
}

}  // namespace
