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

#include "sus/boxed/box.h"

#include "googletest/include/gtest/gtest.h"
#include "sus/error/error.h"
#include "sus/prelude.h"

namespace test::box {
struct MyError {};

struct SuperType {
  virtual std::string name() { return "SuperType"; }
};
struct SubType : public SuperType {
  std::string name() override { return "SubType"; }
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
static_assert(sus::mem::relocate_by_memcpy<Box<i32>>);

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
}

TEST(Box, Clone) {
  static i32 cloned;
  struct Cloneable {
    constexpr explicit Cloneable(i32 i) : i(i) {}
    Cloneable(Cloneable&&) = default;
    Cloneable& operator=(Cloneable&&) = default;

    constexpr Cloneable clone() const noexcept {
      cloned += 1;
      return Cloneable(i);
    }

    i32 i;
  };

  {
    auto b = Box<Cloneable>(Cloneable(2));
    EXPECT_EQ(cloned, 0);
    auto c = sus::clone(b);
    EXPECT_EQ(cloned, 1);
    EXPECT_EQ(c->i, 2);
  }
  EXPECT_EQ(cloned, 1);
}

TEST(Box, CloneInto) {
  static i32 cloned;
  static i32 alloced;
  struct Cloneable {
    constexpr explicit Cloneable(i32 i) : i(i) { alloced += 1; }
    constexpr Cloneable(Cloneable&& r) : i(r.i) { alloced += 1; }
    Cloneable& operator=(Cloneable&&) = default;

    constexpr Cloneable clone() const noexcept {
      cloned += 1;
      return Cloneable(i);
    }
    constexpr void clone_from(const Cloneable& r) noexcept {
      cloned += 1;
      i = r.i;
    }

    i32 i;
  };
  static_assert(sus::mem::Clone<Cloneable>);
  static_assert(sus::mem::CloneFrom<Cloneable>);

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
}

TEST(Box, MoveConstruct) {
  static i32 moved;
  static i32 destroyed;
  struct Moveable {
    constexpr ~Moveable() { destroyed += 1; }
    constexpr explicit Moveable(i32 i) : i(i) {}
    constexpr Moveable(Moveable&& r) : i(r.i) { moved += 1; }
    constexpr Moveable& operator=(Moveable&& r) {
      return i = r.i, moved += 1, *this;
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
}

TEST(Box, MoveAssign) {
  static i32 moved;
  static i32 destroyed;
  struct Moveable {
    constexpr ~Moveable() { destroyed += 1; }
    constexpr explicit Moveable(i32 i) : i(i) {}
    constexpr Moveable(Moveable&& r) : i(r.i) { moved += 1; }
    constexpr Moveable& operator=(Moveable&& r) {
      return i = r.i, moved += 1, *this;
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

TEST(Box, OperatorStar) {
  auto b = Box<i32>(3);
  EXPECT_EQ(*b, 3);
  EXPECT_EQ((*b).wrapping_add(2), 5);
}

TEST(Box, OperatorArrow) {
  auto b = Box<i32>(3);
  EXPECT_EQ(b->wrapping_add(2), 5);
}

TEST(Box, Error) {
  // Box<T> is not Error for non-Error T.
  static_assert(!sus::error::Error<Box<i32>>);
  // Box<T> is Error for Error T.
  static_assert(sus::error::Error<Box<MyError>>);
  static_assert(sus::error::Error<Box<sus::error::DynError>>);
}

TEST(BoxDynError, FromError) {
  {
    auto b = Box<sus::error::DynError>::from(MyError());
    static_assert(sus::error::Error<decltype(b)>);
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

}  // namespace
