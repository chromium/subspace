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

#include "mem/size_of.h"

#include <type_traits>

#include "macros/compiler.h"
#include "macros/no_unique_address.h"
#include "num/types.h"
#include "prelude.h"
#include "googletest/include/gtest/gtest.h"

namespace {

// On some compilers (not MSVC):
//
// Y::c fits into the padding bytes of X, so X can't have it's sizeof(X) bytes
// relocated by memcpy, without overwriting potential parts of Y.
//
// Z::c fits into the padding bytes of X, so X can't have it's sizeof(X) bytes
// relocated by memcpy, without overwriting potential parts of Z.

struct NonStandard {
  i64 a;
  // A public and private field makes the class non-standard-layout so the
  // padding can be re-purposed.
 private:
  i32 b;
};
static_assert(!std::is_standard_layout_v<NonStandard>);

struct NonTrivial {
  // The non-trivial constructor makes X non-trivial so the padding can
  // be re-purposed..
  NonTrivial(i32) {}
  i64 a;
  i32 b;
};
static_assert(!std::is_trivial_v<NonTrivial>);

struct SubNonStandard : public NonStandard {
  i32 c;
};

struct SubNonTrivial : public NonTrivial {
  i32 c;
};

struct HoldNonStandard {
  [[sus_no_unique_address]] NonStandard x;
  i32 c;
};

struct HoldNonTrivial {
  [[sus_no_unique_address]] NonTrivial x;
  i32 c;
};

TEST(DataSizeOf, NonEmptyClass) {
  if constexpr (sus_if_msvc_else(true, false)) {
    // On MSVC, a subclass does not use the padding at the end of its base
    // class (unless it is empty).
    EXPECT_GT(sizeof(SubNonStandard), sizeof(NonStandard));
    EXPECT_GT(sizeof(SubNonTrivial), sizeof(NonTrivial));
    // And the class Z with a [[no_unique_address]] field does not use the
    // padding at the end of that field.
    //
    // This is confusing because supposedly [[msvc::no_unique_address]] opts
    // into this attribute actually doing something, but it doesn't appear to
    // actually do so.
    EXPECT_GT(sizeof(HoldNonStandard), sizeof(NonStandard));
    EXPECT_GT(sizeof(HoldNonTrivial), sizeof(NonTrivial));
    // As a result, the data_size_of<X>() includes the padding too, as other
    // types can not be put into the padding.
    EXPECT_EQ(sus::size_of<NonStandard>(), sizeof(i64) + sizeof(i32) * 2);
    EXPECT_EQ(sus::size_of<NonTrivial>(), sizeof(i64) + sizeof(i32) * 2);
    EXPECT_EQ(sus::data_size_of<NonStandard>(), sizeof(i64) + sizeof(i32) * 2);
    EXPECT_EQ(sus::data_size_of<NonTrivial>(), sizeof(i64) + sizeof(i32) * 2);
  } else {
    // On other compilers, a subclass field will be inserted into the padding of
    // its base class, so the `data_size_of<BaseClass>()` only covers its
    // fields and not its padding.
    EXPECT_EQ(sizeof(SubNonStandard), sizeof(NonStandard));
    EXPECT_EQ(sizeof(SubNonStandard), sizeof(NonTrivial));
    EXPECT_EQ(sizeof(HoldNonStandard), sizeof(NonStandard));
    EXPECT_EQ(sizeof(HoldNonTrivial), sizeof(NonTrivial));
    EXPECT_EQ(sus::size_of<NonStandard>(), sizeof(i64) + sizeof(i32) * 2);
    EXPECT_EQ(sus::size_of<NonTrivial>(), sizeof(i64) + sizeof(i32) * 2);
    EXPECT_EQ(sus::data_size_of<NonStandard>(), sizeof(i64) + sizeof(i32));
    EXPECT_EQ(sus::data_size_of<NonTrivial>(), sizeof(i64) + sizeof(i32));
  }
}

struct Empty {};
struct SubEmpty {
  char c;
};

// A subclass of an empty class will reuse its single byte on all compilers.
static_assert(sizeof(SubEmpty) == sizeof(Empty));

// The empty class itself is 1 byte large which is all padding.
static_assert(sus::size_of<Empty>() == 1u);
// Since the padding can be repurposed, the data_size_of<Empty> is 0, which
// marks the padding as not part of the object's fields.
static_assert(sus::data_size_of<Empty>() == 0u);

}  // namespace
