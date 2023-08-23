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

#include "sus/error/error.h"

#include <concepts>

#include "googletest/include/gtest/gtest.h"
#include "sus/boxed/box.h"
#include "sus/env/var.h"
#include "sus/mem/clone.h"
#include "sus/prelude.h"

namespace test::error {

enum class ErrorReason { SomeReason };

struct ErrorString final {
  std::string reason;
};

struct SuperErrorSideKick {};

struct SuperError {
  sus::Box<sus::error::DynError> source;
};

}  // namespace test::error

using namespace test::error;

// Example for `Error` with enum.
template <>
struct sus::error::ErrorImpl<ErrorReason> {
  constexpr static std::string display(const ErrorReason& self) noexcept {
    switch (self) {
      case ErrorReason::SomeReason: return "we saw SomeReason happen";
    }
    sus::unreachable();
  }
};
static_assert(sus::error::error_display(ErrorReason::SomeReason) ==
              "we saw SomeReason happen");

// Example for `Error` with struct/string.
template <>
struct sus::error::ErrorImpl<ErrorString> {
  constexpr static std::string display(const ErrorString& self) noexcept {
    return sus::clone(self.reason);
  }
};
static_assert(sus::error::error_display(ErrorString("oops")) == "oops");

// Example for `Error` with source.
template <>
struct sus::error::ErrorImpl<SuperError> {
  constexpr static std::string display(const SuperError&) noexcept {
    return "SuperError is here!";
  }
  constexpr static sus::Option<const DynError&> source(
      const SuperError& self) noexcept {
    return sus::some(*self.source);
  }
};
static_assert(sus::error::Error<SuperError>);
template <>
struct sus::error::ErrorImpl<SuperErrorSideKick> {
  constexpr static std::string display(const SuperErrorSideKick&) noexcept {
    return "SuperErrorSideKick is here!";
  }
};
static_assert(sus::error::Error<SuperErrorSideKick>);

namespace {
using sus::error::Error;

TEST(Error, Example_ToString) {
  auto err = u32::try_from(-1).unwrap_err();
  sus::check(fmt::to_string(err) == "out of bounds");
}

TEST(Error, Example_Result) {
  auto f =
      [](i32 i) -> sus::result::Result<void, sus::Box<sus::error::DynError>> {
    if (i > 10) return sus::err(sus::into(ErrorReason::SomeReason));
    if (i < -10) return sus::err(sus::into(ErrorString("too low")));
    return sus::ok();
  };

  sus::check(fmt::format("{}", f(20)) == "Err(we saw SomeReason happen)");
  sus::check(fmt::format("{}", f(-20)) == "Err(too low)");
  sus::check(fmt::format("{}", f(0)) == "Ok(<void>)");
}

TEST(Error, Example_Source) {
  auto get_super_error = []() -> sus::result::Result<void, SuperError> {
    return sus::err(SuperError{.source = sus::into(SuperErrorSideKick())});
  };

  if (auto r = get_super_error(); r.is_err()) {
    auto& e = r.as_err();
    sus::check(fmt::format("Error: {}", e) == "Error: SuperError is here!");
    sus::check(
        fmt::format("Caused by: {}", sus::error::error_source(e).unwrap()) ==
        "Caused by: SuperErrorSideKick is here!");
  }
}

TEST(ErrorDeathTest, Example_Unwrap) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEATH(sus::env::var("IMPORTANT_PATH")
                   .expect("env variable `IMPORTANT_PATH` is not set"),
               "PANIC! at 'env variable `IMPORTANT_PATH` is not set: NotFound'");
#endif
}

TEST(Error, Display) {
  EXPECT_EQ(sus::error::error_display(ErrorReason::SomeReason),
            "we saw SomeReason happen");
  EXPECT_EQ(sus::error::error_display(ErrorString("string here")),
            "string here");
}

TEST(Error, Source) {
  EXPECT_EQ(sus::error::error_source(ErrorReason()).is_some(), false);
  EXPECT_EQ(sus::error::error_source(ErrorString("string here")).is_some(),
            false);

  auto super_error = SuperError{.source = sus::into(SuperErrorSideKick())};
  decltype(auto) source = sus::error::error_source(super_error);
  static_assert(
      std::same_as<decltype(source), sus::Option<const sus::error::DynError&>>);
  EXPECT_EQ(source.is_some(), true);
  decltype(auto) inner_source = sus::error::error_source(source.as_value());
  static_assert(std::same_as<decltype(inner_source),
                             sus::Option<const sus::error::DynError&>>);
  EXPECT_EQ(inner_source.is_some(), false);

  EXPECT_EQ(sus::error::error_display(*source), "SuperErrorSideKick is here!");
}

}  // namespace
