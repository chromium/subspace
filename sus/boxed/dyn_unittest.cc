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

#include "sus/boxed/dyn.h"

#include "googletest/include/gtest/gtest.h"
#include "sus/boxed/box.h"
#include "sus/prelude.h"

namespace {
using namespace sus::boxed;

/// Some concept which requires two functions.
template <class T>
concept C = requires(const T& c, T& m) {
  { c.concept_fn() } -> std::same_as<i32>;
  { m.concept_fn_mut() } -> std::same_as<i32>;
};

template <C T, class Store>
struct DynCTyped;

/// The C concept when type-erased.
struct DynC {
  template <class T>
  static constexpr bool SatisfiesConcept = C<T>;
  template <class T, class Store>
  using DynTyped = DynCTyped<T, Store>;

  DynC() = default;
  virtual ~DynC() = default;
  DynC(DynC&&) = delete;
  DynC& operator=(DynC&&) = delete;

  // Virtual concept API.
  virtual i32 concept_fn() const = 0;
  virtual i32 concept_fn_mut() = 0;
};

/// DynC satisfies concept C.
static_assert(C<DynC>);

/// The implementation of type-erasure for the C concept.
template <C T, class Store>
struct DynCTyped final : public DynC {
  constexpr DynCTyped(Store&& c) : c_(sus::forward<Store>(c)) {}

  constexpr i32 concept_fn() const override { return c_.concept_fn(); }
  constexpr i32 concept_fn_mut() override { return c_.concept_fn_mut(); }

 private:
  Store c_;
};

/// Foo satisfies concept C.
struct Foo final {
  i32 concept_fn() const noexcept { return called_const += 1, called_const; }
  i32 concept_fn_mut() noexcept { return called_mut += 1, called_mut; }

  mutable i32 called_const;
  i32 called_mut;
};
static_assert(C<Foo>);

/// These act on the `C` concept but without being templated.
i32 GiveC(const DynC& c) { return c.concept_fn(); }
i32 GiveCMut(DynC& c) { return c.concept_fn_mut(); }
i32 GiveCRval(DynC&& c) { return c.concept_fn_mut(); }
Box<DynC> GiveBoxC(Box<DynC> c) { return c; }

/// `DynC` satisfies `DynConcept` with an implementation of `C` (which is `Foo`
/// here).
static_assert(sus::boxed::DynConcept<DynC, Foo>);

TEST(Dyn, Box) {
  // Box::from(C) exists since DynC exists and satisfies `DynConcept` for `C`.
  {
    static_assert(sus::construct::From<Box<DynC>, Foo>);
    auto b = Box<DynC>::from(Foo());
    EXPECT_EQ(b->concept_fn(), 1);
    EXPECT_EQ(b->concept_fn(), 2);
    EXPECT_EQ(b->concept_fn_mut(), 1);
    const auto bc = sus::move(b);
    EXPECT_EQ(bc->concept_fn(), 3);
  }
  // Into<C, Box> works.
  {
    static_assert(sus::construct::Into<Foo, Box<DynC>>);
    auto b = GiveBoxC(sus::into(Foo()));
    EXPECT_EQ(b->concept_fn(), 1);
    EXPECT_EQ(b->concept_fn(), 2);
    EXPECT_EQ(b->concept_fn_mut(), 1);
  }
}

template <class DynC, class Foo, class Ref>
concept CanGiveCFromStruct = requires(Ref r) {
  { GiveC(Dyn<DynC, Foo>(r)) };
};
template <class DynC, class Foo, class Ref>
concept CanGiveCMutFromStruct = requires(Ref r) {
  { GiveCMut(Dyn<DynC, Foo>(r)) };
};
template <class DynC, class Foo, class Ref>
concept CanGiveCRvalFromStruct = requires(Ref r) {
  { GiveCMut(Dyn<DynC, Foo>(r)) };
};

// Tests the semantics of the struct itself, though it's not used directly.
TEST(Dyn, DynStruct) {
  // Mutable.
  {
    Foo f;

    static_assert(CanGiveCFromStruct<DynC, Foo, decltype(f)>);
    static_assert(CanGiveCMutFromStruct<DynC, Foo, decltype(f)>);
    static_assert(CanGiveCRvalFromStruct<DynC, Foo, decltype(f)>);
    static_assert(CanGiveCFromStruct<const DynC, Foo, decltype(f)>);
    // Can't give a const `DynC` to a mutable reference `DynC&`.
    static_assert(!CanGiveCMutFromStruct<const DynC, Foo, decltype(f)>);
    // Can't give a const `DynC` to an rvalue reference `DynC&&`
    static_assert(!CanGiveCRvalFromStruct<const DynC, Foo, decltype(f)>);

    EXPECT_EQ(GiveC(Dyn<DynC, Foo>(f)), 1);
    EXPECT_EQ(GiveC(Dyn<DynC, Foo>(f)), 2);
    EXPECT_EQ(GiveCMut(Dyn<DynC, Foo>(f)), 1);
    EXPECT_EQ(GiveC(Dyn<const DynC, Foo>(f)), 3);
    EXPECT_EQ(GiveC(Dyn<const DynC, Foo>(f)), 4);
    // ERROR: Can't give a const `DynC` to a mutable reference `DynC&`.
    // GiveCMut(Dyn<const DynC, Foo>(f));

    EXPECT_EQ(GiveCRval(Dyn<DynC, Foo>(Foo())), 1);
  }
  // Const.
  {
    const Foo f;

    // Can't use a const `Foo` to construct a mutable `DynC`.
    static_assert(!CanGiveCFromStruct<DynC, Foo, decltype(f)>);
    // Can't use a const `Foo` to construct a mutable `DynC&`.
    static_assert(!CanGiveCMutFromStruct<DynC, Foo, decltype(f)>);
    // Can't use a const `Foo` to construct an rvalue reference`DynC&&`.
    static_assert(!CanGiveCRvalFromStruct<const DynC, Foo, decltype(f)>);
    static_assert(CanGiveCFromStruct<const DynC, Foo, decltype(f)>);
    // Can't give a const `DynC` to a mutable reference `DynC&`.
    static_assert(!CanGiveCMutFromStruct<const DynC, Foo, decltype(f)>);
    // Can't give a const `DynC` to an rvalue reference `DynC&&`
    static_assert(!CanGiveCRvalFromStruct<const DynC, Foo, decltype(f)>);

    EXPECT_EQ(GiveC(Dyn<const DynC, Foo>(f)), 1);
    EXPECT_EQ(GiveC(Dyn<const DynC, Foo>(f)), 2);
    // ERROR: Can't use a const `Foo` to construct a mutable `DynC`.
    // GiveC(Dyn<DynC, Foo>(f));
    // GiveCMut(Dyn<DynC, Foo>(f));
    // ERROR: Can't give a const `DynC` to a mutable reference `DynC&`.
    // GiveCMut(Dyn<const DynC, Foo>(f));
  }

  // ERROR: Holding a reference to a temporary (on clang only).
  // TODO: How to test this with a concept?
  // [[maybe_unused]] auto x = Dyn<DynC, Foo>(Foo());
}

template <class DynC, class Foo, class Ref>
concept CanGiveCMut = requires(Ref r) {
  { GiveCMut(sus::dyn<DynC>(r)) };
};
template <class DynC, class Foo, class Ref>
concept CanGiveC = requires(Ref r) {
  { GiveC(sus::dyn<DynC>(r)) };
};

TEST(Dyn, DynFunction) {
  // Mutable.
  {
    Foo f;

    static_assert(CanGiveC<DynC, Foo, decltype(f)>);
    static_assert(CanGiveCMut<DynC, Foo, decltype(f)>);
    static_assert(CanGiveC<const DynC, Foo, decltype(f)>);
    // Can't give a const `DynC` to a mutable reference `DynC&`.
    static_assert(!CanGiveCMut<const DynC, Foo, decltype(f)>);

    EXPECT_EQ(GiveC(sus::dyn<DynC>(f)), 1);
    EXPECT_EQ(GiveC(sus::dyn<DynC>(f)), 2);
    EXPECT_EQ(GiveCMut(sus::dyn<DynC>(f)), 1);
    EXPECT_EQ(GiveC(sus::dyn<const DynC>(f)), 3);
    EXPECT_EQ(GiveC(sus::dyn<const DynC>(f)), 4);
    // ERROR: Can't give a const `DynC` to a mutable reference `DynC&`.
    // GiveCMut(sus::dyn<const DynC>(f));
  }
  // Const.
  {
    const Foo f;

    // Can't use a const `Foo` to construct a mutable `DynC`.
    static_assert(!CanGiveC<DynC, Foo, decltype(f)>);
    // Can't use a const `Foo` to construct a mutable `DynC&`.
    static_assert(!CanGiveCMut<DynC, Foo, decltype(f)>);
    static_assert(CanGiveC<const DynC, Foo, decltype(f)>);
    // Can't give a const `DynC` to a mutable reference `DynC&`.
    static_assert(!CanGiveCMut<const DynC, Foo, decltype(f)>);

    EXPECT_EQ(GiveC(sus::dyn<const DynC>(f)), 1);
    EXPECT_EQ(GiveC(sus::dyn<const DynC>(f)), 2);
    // ERROR: Can't use a const `Foo` to construct a mutable `DynC`.
    // GiveC(sus::dyn<DynC>(f));
    // GiveCMut(sus::dyn<DynC>(f));
    // ERROR: Can't give a const `DynC` to a mutable reference `DynC&`.
    // GiveCMut(sus::dyn<const DynC>(f));
  }

  // ERROR: Holding a reference to a temporary (on clang only).
  // TODO: How to test this with a concept?
  // [[maybe_unused]] auto x = sus::dyn<DynC>(Foo());
}

namespace example_no_macro {

// A concept which requires a single const-access method named `concept_fn`.
template <class T>
concept MyConcept = requires(const T& t) {
  { t.concept_fn() } -> std::same_as<void>;
};

template <class T, class Store>
class DynMyConceptTyped;

class DynMyConcept {
 public:
  // Pure virtual concept API.
  virtual void concept_fn() const = 0;
  template <class T>

  static constexpr bool SatisfiesConcept = MyConcept<T>;
  template <class T, class Store>
  using DynTyped = DynMyConceptTyped<T, Store>;

  DynMyConcept() = default;
  virtual ~DynMyConcept() = default;
  DynMyConcept(DynC&&) = delete;
  DynMyConcept& operator=(DynMyConcept&&) = delete;
};
// Verifies that DynMyConcept also satisfies MyConcept, which is required.
static_assert(MyConcept<DynMyConcept>);

template <class T, class Store>
class DynMyConceptTyped final : public DynMyConcept {
 public:
  // Virtual concept API implementation.
  void concept_fn() const override { return c_.concept_fn(); };

  constexpr DynMyConceptTyped(Store&& c) : c_(sus::forward<Store>(c)) {}

 private:
  Store c_;
};

// A type which satiesfies `MyConcept`.
struct MyConceptType {
  void concept_fn() const {}
};

TEST(Dyn, Example_NoMacro) {
  // Verifies that DynMyConcept is functioning correctly, testing it against
  // a type that satisfies MyConcept.
  static_assert(sus::boxed::DynConcept<DynMyConcept, MyConceptType>);

  auto b = [](Box<DynMyConcept> c) { c->concept_fn(); };
  // `Box<DynMyConcept>` constructs from `MyConceptType`.
  b(sus::into(MyConceptType()));

  auto d = [](const DynMyConcept& c) { c.concept_fn(); };
  // `MyConceptType` converts to `const MyConcept&` with `sus::dyn()`.
  d(sus::dyn<const DynMyConcept>(MyConceptType()));
}

}  // namespace example_no_macro

namespace example_macro {

// A concept which requires a single const-access method named `concept_fn`.
template <class T>
concept MyConcept = requires(const T& t) {
  { t.concept_fn() } -> std::same_as<void>;
};

template <class T, class Store>
class DynMyConceptTyped;

class DynMyConcept {
  sus_dyn_concept(MyConcept, DynMyConcept, DynMyConceptTyped);

 public:
  // Pure virtual concept API.
  virtual void concept_fn() const = 0;
};
// Verifies that DynMyConcept also satisfies MyConcept, which is required.
static_assert(MyConcept<DynMyConcept>);

template <class T, class Store>
class DynMyConceptTyped final : public DynMyConcept {
  sus_dyn_concept_typed(MyConcept, DynMyConcept, DynMyConceptTyped, v);

  // Virtual concept API implementation.
  void concept_fn() const override { return v.concept_fn(); };
};

// A type which satiesfies `MyConcept`.
struct MyConceptType {
  void concept_fn() const {}
};

TEST(Dyn, Example_Macro) {
  // Verifies that DynMyConcept is functioning correctly, testing it against
  // a type that satisfies MyConcept.
  static_assert(sus::boxed::DynConcept<DynMyConcept, MyConceptType>);

  auto b = [](Box<DynMyConcept> c) { c->concept_fn(); };
  // `Box<DynMyConcept>` constructs from `MyConceptType`.
  b(sus::into(MyConceptType()));

  auto d = [](const DynMyConcept& c) { c.concept_fn(); };
  // `MyConceptType` converts to `const MyConcept&` with `sus::dyn()`.
  d(sus::dyn<const DynMyConcept>(MyConceptType()));
}

}  // namespace example_macro

TEST(Dyn, Example_Stack) {
  std::srand(sus::mog<unsigned>(std::time(nullptr)));

  auto x = [](sus::Option<sus::fn::DynFn<std::string()>&> fn) {
    if (fn.is_some())
      return sus::move(fn).unwrap()();
    else
      return std::string("tails");
  };

  auto heads = [] { return std::string("heads"); };
  // Type-erased `Fn<std::string()>` that represents `heads`. Placed on the
  // stack to outlive its use in the `Option` and the call to `x(cb)`.
  auto dyn_heads = sus::dyn<sus::fn::DynFn<std::string()>>(heads);
  // Conditionally holds a type-erased reference to `heads`. This requires a
  // type-erasure that outlives the `cb` variable.
  auto cb = [&]() -> sus::Option<sus::fn::DynFn<std::string()>&> {
    if (std::rand() % 2) return sus::some(dyn_heads);
    return sus::none();
  }();

  std::string s = x(cb);

  fmt::println("{}", s);  // Prints one of "heads" or "tails.
}

}  // namespace
