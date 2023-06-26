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

#include "subspace/mem/size_of.h"

#include <type_traits>

#include "googletest/include/gtest/gtest.h"
#include "subspace/macros/compiler.h"
#include "subspace/macros/no_unique_address.h"
#include "subspace/num/types.h"
#include "subspace/prelude.h"

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

struct NonStandardFinal final {
  i64 a;
  // A public and private field makes the class non-standard-layout so the
  // padding can be re-purposed.
 private:
  i32 b;
};
static_assert(!std::is_standard_layout_v<NonStandardFinal>);

struct NonTrivialFinal final {
  // The non-trivial constructor makes X non-trivial so the padding can
  // be re-purposed..
  NonTrivialFinal(i32) {}
  i64 a;
  i32 b;
};
static_assert(!std::is_trivial_v<NonTrivialFinal>);

struct HoldNonStandardFinal {
  [[sus_no_unique_address]] NonStandard x;
  i32 c;
};

struct HoldNonTrivialFinal {
  [[sus_no_unique_address]] NonTrivial x;
  i32 c;
};

TEST(DataSizeOf, NonEmptyFinalClass) {
  if constexpr (sus_if_msvc_else(true, false)) {
    // On MSVC, the class with a [[no_unique_address]] field does not use the
    // padding at the end of that field.
    //
    // This is confusing because supposedly [[msvc::no_unique_address]] opts
    // into this attribute actually doing something, but it doesn't appear to
    // actually do so.
    EXPECT_GT(sizeof(HoldNonStandardFinal), sizeof(NonStandard));
    EXPECT_GT(sizeof(HoldNonTrivialFinal), sizeof(NonTrivial));
    // As a result, the data_size_of<X>() includes the padding too, as other
    // types can not be put into the padding.
    EXPECT_EQ(sus::size_of<NonStandardFinal>(), sizeof(i64) + sizeof(i32) * 2);
    EXPECT_EQ(sus::size_of<NonTrivialFinal>(), sizeof(i64) + sizeof(i32) * 2);
    EXPECT_EQ(sus::data_size_of<NonStandardFinal>(),
              sizeof(i64) + sizeof(i32) * 2);
    EXPECT_EQ(sus::data_size_of<NonTrivialFinal>(),
              sizeof(i64) + sizeof(i32) * 2);
  } else {
    // On other compilers, a [[no_unique_address]] field will have its tail
    // padding used, so the `data_size_of<NoUniqueAddressClass>()` only covers
    // its fields and not its padding.
    EXPECT_EQ(sizeof(HoldNonStandardFinal), sizeof(NonStandard));
    EXPECT_EQ(sizeof(HoldNonTrivialFinal), sizeof(NonTrivial));
    EXPECT_EQ(sus::size_of<NonStandardFinal>(), sizeof(i64) + sizeof(i32) * 2);
    EXPECT_EQ(sus::size_of<NonTrivialFinal>(), sizeof(i64) + sizeof(i32) * 2);
    EXPECT_EQ(sus::data_size_of<NonStandardFinal>(), sizeof(i64) + sizeof(i32));
    EXPECT_EQ(sus::data_size_of<NonTrivialFinal>(), sizeof(i64) + sizeof(i32));
  }
}

// For testing non-final classes.

struct SmallerThanMaxAlignNoTailPadding {
  i32 i;
};
// The goal is to have a type that is larger than the max alignment. If this
// fails for some future platform, make the type larger (without tail padding)
// to match.
static_assert(sizeof(SmallerThanMaxAlignNoTailPadding) <
              alignof(std::max_align_t));

struct MaxAlignNoTailPadding {
  alignas(alignof(std::max_align_t)) char i[alignof(std::max_align_t)];
};
// The goal is to have a type that is the size of the max alignment. If this
// fails for some future platform, make the type larger (without tail padding)
// to match.
static_assert(sizeof(MaxAlignNoTailPadding) == alignof(std::max_align_t));

struct LargerThanMaxAlignNoTailPadding {
  alignas(alignof(std::max_align_t)) char i[alignof(std::max_align_t)];
  alignas(alignof(std::max_align_t)) char j[alignof(std::max_align_t)];
};
// The goal is to have a type that is larger than the max alignment. If this
// fails for some future platform, make the type larger (without tail padding)
// to match.
static_assert(sizeof(LargerThanMaxAlignNoTailPadding) >
              alignof(std::max_align_t));

// For testing final classes.

struct SmallerThanMaxAlignNoTailPaddingFinal final {
  i32 i;
};
// Same as SmallerThanMaxAlignNoTailPadding, but also `final`.
static_assert(sizeof(SmallerThanMaxAlignNoTailPaddingFinal) <
              alignof(std::max_align_t));
struct MaxAlignNoTailPaddingFinal final {
  alignas(alignof(std::max_align_t)) char i[alignof(std::max_align_t)];
};
// Same as MaxAlignNoTailPadding, but also `final`.
static_assert(sizeof(MaxAlignNoTailPaddingFinal) == alignof(std::max_align_t));
struct LargerThanMaxAlignNoTailPaddingFinal final {
  alignas(alignof(std::max_align_t)) char i[alignof(std::max_align_t)];
  alignas(alignof(std::max_align_t)) char j[alignof(std::max_align_t)];
};
// Same as LargerThanMaxAlignNoTailPaddingFinal, but also `final`.
static_assert(sizeof(LargerThanMaxAlignNoTailPaddingFinal) >
              alignof(std::max_align_t));

TEST(DataSizeOf, NoTailPadding) {
  EXPECT_EQ(sus::data_size_of<SmallerThanMaxAlignNoTailPadding>(),
            sus::size_of<SmallerThanMaxAlignNoTailPadding>());
  EXPECT_EQ(sus::data_size_of<MaxAlignNoTailPadding>(),
            sus::size_of<MaxAlignNoTailPadding>());
  EXPECT_EQ(sus::data_size_of<LargerThanMaxAlignNoTailPadding>(),
            sus::size_of<LargerThanMaxAlignNoTailPadding>());
  EXPECT_EQ(sus::data_size_of<SmallerThanMaxAlignNoTailPaddingFinal>(),
            sus::size_of<SmallerThanMaxAlignNoTailPaddingFinal>());
  EXPECT_EQ(sus::data_size_of<MaxAlignNoTailPaddingFinal>(),
            sus::size_of<MaxAlignNoTailPaddingFinal>());
  EXPECT_EQ(sus::data_size_of<LargerThanMaxAlignNoTailPaddingFinal>(),
            sus::size_of<LargerThanMaxAlignNoTailPaddingFinal>());
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

// This structure has 7 (i.e. `alignof(max_align_t) - 1`) bytes of padding. That
// requires the data size finder to try up to `alignof(max_align_t)` bytes to
// find the data size. When the usize alignment is the same as max_align_t, this
// test requires the data_size_finder to check up to max_align_t bytes, and
// caught an error where it was only checking to `max_align_t-1`.
struct FinalClass final {
  usize s;
  bool b;
};

// MSVC doesn't make the tail padding available in this case, but other
// compilers do.
static_assert(sus::data_size_of<FinalClass>() ==
              sus_if_msvc_else(sizeof(usize) * 2, sizeof(usize) * 1 + 1));

}  // namespace
