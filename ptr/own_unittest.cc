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

#include "ptr/own.h"

#include <compare>

#include "mem/move.h"
#include "num/types.h"
#include "option/option.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

namespace {

using sus::Own;

struct S {
  explicit S(usize& muts, usize& consts) : muts_(muts), consts_(consts) {}

  void Method() { muts_ += 1_usize; }
  void Method() const { consts_ += 1_usize; }

 private:
  usize& muts_;
  usize& consts_;
};

struct SArrow {
  explicit SArrow(usize& muts, usize& consts) : s(muts, consts) {}

  const S* operator->() const { return &s; }
  S* operator->() { return &s; }

  S s;
};

TEST(Own, Arrow) {
  auto muts = 0_usize;
  auto consts = 0_usize;
  {
    auto o = Own<S>::with(muts, consts);
    o->Method();
    EXPECT_EQ(muts, 1_usize);
  }
  muts = consts = 0_usize;
  {
    const auto o = Own<S>::with(muts, consts);
    o->Method();
    EXPECT_EQ(consts, 1_usize);
  }
  muts = consts = 0_usize;
  {
    auto o = Own<const S>::with(muts, consts);
    o->Method();
    EXPECT_EQ(consts, 1_usize);
  }
  muts = consts = 0_usize;
  {
    auto o = Own<SArrow>::with(muts, consts);
    o->Method();
    EXPECT_EQ(muts, 1_usize);
  }
  muts = consts = 0_usize;
  {
    const auto o = Own<SArrow>::with(muts, consts);
    o->Method();
    EXPECT_EQ(consts, 1_usize);
  }
  muts = consts = 0_usize;
  {
    auto o = Own<const SArrow>::with(muts, consts);
    o->Method();
    EXPECT_EQ(consts, 1_usize);
  }
}

class DefaultConstruct {};
class WithDefaultConstruct {
 public:
  static auto with_default() noexcept { return WithDefaultConstruct(); }

 private:
  WithDefaultConstruct() {}
};

template <class T>
concept HasMakeDefault = requires {
                           { Own<T>::with_default() };
                         };

static_assert(!HasMakeDefault<S>);
static_assert(HasMakeDefault<DefaultConstruct>);
static_assert(HasMakeDefault<WithDefaultConstruct>);

struct FreeCounter {
  FreeCounter(usize& frees) : frees_(frees) {}
  ~FreeCounter() { frees_ += 1_usize; }

 private:
  usize& frees_;
};

TEST(OwnDeath, DestroyInConstruct) {
  struct DeleteIt;
  struct S {
    Own<DeleteIt> own = Own<DeleteIt>::with(this);
  };
  struct DeleteIt {
    DeleteIt(S* s) { delete s; }
  };

#if GTEST_HAS_DEATH_TEST
  auto s = sus::Option<Own<S>>::none();
  EXPECT_DEATH(s = sus::Option<Own<S>>::with_default(), "");
#endif
}

TEST(Own, Freed) {
  auto frees = 0_usize;
  {
    auto o = Own<FreeCounter>::with(frees);
    EXPECT_EQ(frees, 0_usize);
  }
  EXPECT_EQ(frees, 1_usize);
}

TEST(Own, Drop) {
  auto frees = 0_usize;
  auto o = Own<FreeCounter>::with(frees);
  EXPECT_EQ(frees, 0_usize);
  sus::move(o).drop();
  EXPECT_EQ(frees, 1_usize);
}

TEST(Own, ToConst) {
  {
    auto o = Own<usize>::with(2_usize);
    const Own<usize> oc(sus::move(o));
    EXPECT_EQ(*oc, 2_usize);
  }
  {
    auto o = Own<usize>::with(2_usize);
    auto oc = Own<const usize>::from(sus::move(o));
    EXPECT_EQ(*oc, 2_usize);
  }
}

TEST(Own, Upcast) {
  struct Base {
    i32 i = 3_i32;
  };
  struct Sub : Base {};
  auto sub = Own<Sub>::with();
  auto base = sus::move(sub).cast_to<Base>();
  EXPECT_EQ(base->i, 3_i32);
}

TEST(Own, CloneCopyable) {
  auto o1 = Own<i32>::with(2_i32);
  auto o2 = o1.clone();
  EXPECT_EQ(*o1, *o2);
}

TEST(Own, FromRaw) {
  auto frees = 0_usize;
  {
    auto o = Own<FreeCounter>::from_raw(unsafe_fn, new FreeCounter(frees));
    EXPECT_EQ(frees, 0_usize);
  }
  EXPECT_EQ(frees, 1_usize);
}

TEST(Own, IntoRaw) {
  auto frees = 0_usize;
  FreeCounter* f;
  {
    auto o = Own<FreeCounter>::with(frees);
    EXPECT_EQ(frees, 0_usize);
    f = sus::move(o).into_raw(unsafe_fn);
  }
  EXPECT_EQ(frees, 0_usize);
  delete f;
  EXPECT_EQ(frees, 1_usize);
}

TEST(Own, Eq) {
  static_assert(sus::num::Eq<i32, i32>);
  static_assert(sus::num::Eq<Own<i32>, Own<i32>>);
  static_assert(!sus::num::Eq<i32, S>);
  static_assert(!sus::num::Eq<Own<i32>, Own<S>>);

  auto a = Own<i32>::from(2_i32);
  auto b = Own<i32>::from(2_i32);
  auto c = Own<i32>::from(-2_i32);
  EXPECT_EQ(a, b);
  EXPECT_NE(b, c);
}

TEST(Own, Ord) {
  static_assert(sus::num::Ord<i32, i32>);
  static_assert(sus::num::Ord<Own<i32>, Own<i32>>);
  static_assert(!sus::num::Ord<i32, S>);
  static_assert(!sus::num::Ord<Own<i32>, Own<S>>);

  EXPECT_GT(Own<i32>::from(2_i32), Own<i32>::from(-2_i32));
}

TEST(Own, StrongOrder) {
  EXPECT_EQ(std::strong_order(Own<i32>::from(2_i32), Own<i32>::from(2_i32)),
            std::strong_ordering::equal);
  EXPECT_EQ(std::strong_order(Own<i32>::from(2_i32), Own<i32>::from(3_i32)),
            std::strong_ordering::less);
  EXPECT_EQ(std::strong_order(Own<i32>::from(2_i32), Own<i32>::from(1_i32)),
            std::strong_ordering::greater);
}

struct Weak {
  auto operator==(Weak const& o) const& { return a == o.a && b == o.b; }
  auto operator<=>(Weak const& o) const& {
    if (a == o.a) return std::weak_ordering::equivalent;
    if (a < o.a) return std::weak_ordering::less;
    return std::weak_ordering::greater;
  }

  Weak(int a, int b) : a(a), b(b) {}
  int a;
  int b;
};

TEST(Own, WeakOrder) {
  static_assert(sus::num::WeakOrd<Weak, Weak>);
  static_assert(sus::num::WeakOrd<Own<Weak>, Own<Weak>>);
  static_assert(!sus::num::WeakOrd<Weak, S>);
  static_assert(!sus::num::WeakOrd<Own<Weak>, Own<S>>);

  EXPECT_EQ(
      std::weak_order(Own<Weak>::from(Weak(1, 2)), Own<Weak>::from(Weak(1, 2))),
      std::weak_ordering::equivalent);
  EXPECT_EQ(
      std::weak_order(Own<Weak>::from(Weak(1, 2)), Own<Weak>::from(Weak(1, 3))),
      std::weak_ordering::equivalent);
  EXPECT_EQ(
      std::weak_order(Own<Weak>::from(Weak(1, 2)), Own<Weak>::from(Weak(2, 3))),
      std::weak_ordering::less);
  EXPECT_EQ(
      std::weak_order(Own<Weak>::from(Weak(2, 2)), Own<Weak>::from(Weak(1, 3))),
      std::weak_ordering::greater);
}

TEST(Own, PartialOrder) {
  EXPECT_EQ(
      std::partial_order(Own<f32>::from(11_f32), Own<f32>::from(11.2_f32)),
      std::partial_ordering::less);
  EXPECT_EQ(std::partial_order(Own<f32>::from(11_f32),
                               Own<f32>::from(f32::TODO_NAN())),
            std::partial_ordering::unordered);
}

TEST(Own, PtrEqual) {
  auto a = Own<i32>::from(2_i32);
  auto b = Own<i32>::from(2_i32);
  EXPECT_EQ(a.ptr_equal(b), false);
  EXPECT_EQ(a.ptr_equal(a), true);
}

TEST(Own, ToCopy) {
  auto o = Own<i32>::from(2_i32);
  auto i = o.to_copy();
  EXPECT_EQ(i, *o);
}

TEST(Own, CopyFrom) {
  auto o = Own<i32>::from(2_i32);
  auto i = 3_i32;
  o.copy_from(i);
  EXPECT_EQ(i, *o);
}

struct Mover {
  i32 i = 2_i32;
  Mover() {}

  Mover(Mover&& m) : i(m.i) { m.i = 0_i32; }
  void operator=(Mover&& m) { i = sus::mem::replace(mref(m.i), 0_i32); }

  Mover(const Mover& m) : i(m.i) {}
  void operator=(const Mover& m) { i = m.i; }
};

TEST(Own, ToMove) {
  auto o = Own<Mover>::with();
  EXPECT_EQ(o->i, 2_i32);
  auto m = o.to_move();
  EXPECT_EQ(m.i, 2_i32);
  EXPECT_EQ(o->i, 0_i32);
}

TEST(Own, MoveFrom) {
  auto o = Own<Mover>::with();
  auto m = Mover();
  m.i = 3_i32;
  EXPECT_EQ(o->i, 2_i32);
  o.move_from(sus::move(m));
  EXPECT_EQ(o->i, 3_i32);
  EXPECT_EQ(m.i, 0_i32);
}

TEST(Own, ForwardFrom) {
  auto o = Own<Mover>::with();
  auto m = Mover();

  m.i = 3_i32;
  EXPECT_EQ(o->i, 2_i32);
  o.forward_from(static_cast<const Mover&>(m));
  EXPECT_EQ(o->i, 3_i32);
  EXPECT_EQ(m.i, 3_i32);

  m.i = 4_i32;
  o.forward_from(sus::move(m));
  EXPECT_EQ(o->i, 4_i32);
  EXPECT_EQ(m.i, 0_i32);
}

}  // namespace
