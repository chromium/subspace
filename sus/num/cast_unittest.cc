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

#include "sus/num/cast.h"

#include "googletest/include/gtest/gtest.h"
#include "sus/prelude.h"

namespace {

template <class Self>
struct CheckTransmogrify {
  static_assert(sus::construct::Cast<Self, char>);
  static_assert(sus::construct::Cast<Self, signed char>);
  static_assert(sus::construct::Cast<Self, unsigned char>);
  static_assert(sus::construct::Cast<Self, int8_t>);
  static_assert(sus::construct::Cast<Self, int16_t>);
  static_assert(sus::construct::Cast<Self, int32_t>);
  static_assert(sus::construct::Cast<Self, int64_t>);
  static_assert(sus::construct::Cast<Self, uint8_t>);
  static_assert(sus::construct::Cast<Self, uint16_t>);
  static_assert(sus::construct::Cast<Self, uint32_t>);
  static_assert(sus::construct::Cast<Self, uint64_t>);
  static_assert(sus::construct::Cast<Self, size_t>);
  static_assert(sus::construct::Cast<Self, intptr_t>);
  static_assert(sus::construct::Cast<Self, uintptr_t>);
  static_assert(sus::construct::Cast<Self, float>);
  static_assert(sus::construct::Cast<Self, double>);

  static_assert(sus::construct::Cast<Self, i8>);
  static_assert(sus::construct::Cast<Self, i16>);
  static_assert(sus::construct::Cast<Self, i32>);
  static_assert(sus::construct::Cast<Self, i64>);
  static_assert(sus::construct::Cast<Self, u8>);
  static_assert(sus::construct::Cast<Self, u16>);
  static_assert(sus::construct::Cast<Self, u32>);
  static_assert(sus::construct::Cast<Self, u64>);
  static_assert(sus::construct::Cast<Self, isize>);
  static_assert(sus::construct::Cast<Self, usize>);
  static_assert(sus::construct::Cast<Self, uptr>);
  static_assert(sus::construct::Cast<Self, f32>);
  static_assert(sus::construct::Cast<Self, f64>);

  static_assert(sus::construct::Cast<char, Self>);
  static_assert(sus::construct::Cast<signed char, Self>);
  static_assert(sus::construct::Cast<unsigned char, Self>);
  static_assert(sus::construct::Cast<int8_t, Self>);
  static_assert(sus::construct::Cast<int16_t, Self>);
  static_assert(sus::construct::Cast<int32_t, Self>);
  static_assert(sus::construct::Cast<int64_t, Self>);
  static_assert(sus::construct::Cast<uint8_t, Self>);
  static_assert(sus::construct::Cast<uint16_t, Self>);
  static_assert(sus::construct::Cast<uint32_t, Self>);
  static_assert(sus::construct::Cast<uint64_t, Self>);
  static_assert(sus::construct::Cast<size_t, Self>);
  static_assert(sus::construct::Cast<intptr_t, Self>);
  static_assert(sus::construct::Cast<uintptr_t, Self>);
  static_assert(sus::construct::Cast<float, Self>);
  static_assert(sus::construct::Cast<double, Self>);

  static_assert(sus::construct::Cast<i8, Self>);
  static_assert(sus::construct::Cast<i16, Self>);
  static_assert(sus::construct::Cast<i32, Self>);
  static_assert(sus::construct::Cast<i64, Self>);
  static_assert(sus::construct::Cast<u8, Self>);
  static_assert(sus::construct::Cast<u16, Self>);
  static_assert(sus::construct::Cast<u32, Self>);
  static_assert(sus::construct::Cast<u64, Self>);
  static_assert(sus::construct::Cast<isize, Self>);
  static_assert(sus::construct::Cast<usize, Self>);
  static_assert(sus::construct::Cast<uptr, Self>);
  static_assert(sus::construct::Cast<f32, Self>);
  static_assert(sus::construct::Cast<f64, Self>);
};

TEST(NumTransmogrify, Satisfies) {
  CheckTransmogrify<char>();
  CheckTransmogrify<signed char>();
  CheckTransmogrify<unsigned char>();
  CheckTransmogrify<int8_t>();
  CheckTransmogrify<int16_t>();
  CheckTransmogrify<int32_t>();
  CheckTransmogrify<int64_t>();
  CheckTransmogrify<uint8_t>();
  CheckTransmogrify<uint16_t>();
  CheckTransmogrify<uint32_t>();
  CheckTransmogrify<uint64_t>();
  CheckTransmogrify<size_t>();
  CheckTransmogrify<intptr_t>();
  CheckTransmogrify<uintptr_t>();

  CheckTransmogrify<i8>();
  CheckTransmogrify<i16>();
  CheckTransmogrify<i32>();
  CheckTransmogrify<i64>();
  CheckTransmogrify<u8>();
  CheckTransmogrify<u16>();
  CheckTransmogrify<u32>();
  CheckTransmogrify<u64>();
  CheckTransmogrify<isize>();
  CheckTransmogrify<usize>();
  CheckTransmogrify<uptr>();
}

TEST(NumTransmogrify, u8) {
  using Self = u8;
  // Negative to larger unsigned self.
  {
    auto i = sus::cast<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX - Self::try_from(i8::MAX).unwrap());
  }
  // Larger unsigned to smaller self.
  {
    auto i = sus::cast<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX);
  }
  // Unsigned self to signed.
  {
    auto i = sus::cast<int8_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), int8_t>);
    EXPECT_EQ(i, int8_t{-1});
  }
}

TEST(NumTransmogrify, u16) {
  using Self = u16;
  // Smaller negative to larger unsigned self.
  {
    auto i = sus::cast<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX - Self::try_from(i8::MAX).unwrap());
  }
  // Larger unsigned to smaller self.
  {
    auto i = sus::cast<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX);
  }
  // Larger unsigned self to smaller signed.
  {
    auto i = sus::cast<int16_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), int16_t>);
    EXPECT_EQ(i, int16_t{-1});
  }
}

TEST(NumTransmogrify, u32) {
  using Self = u32;
  // Smaller negative to larger unsigned self.
  {
    auto i = sus::cast<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX - Self::try_from(i8::MAX).unwrap());
  }
  // Larger unsigned to smaller self.
  {
    auto i = sus::cast<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX);
  }
  // Larger unsigned self to smaller signed.
  {
    auto i = sus::cast<int16_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), int16_t>);
    EXPECT_EQ(i, int16_t{-1});
  }
}

TEST(NumTransmogrify, u64) {
  using Self = u64;
  // Smaller negative to larger unsigned self.
  {
    auto i = sus::cast<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX - Self::try_from(i8::MAX).unwrap());
  }
  // Larger unsigned to smaller self.
  {
    auto i = sus::cast<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX);
  }
  // Larger unsigned self to smaller signed.
  {
    auto i = sus::cast<int16_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), int16_t>);
    EXPECT_EQ(i, int16_t{-1});
  }
}

TEST(NumTransmogrify, uptr) {
  using Self = uptr;
  // Smaller negative to larger unsigned self.
  {
    auto i = sus::cast<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, static_cast<uintptr_t>(i8::MIN_PRIMITIVE));
  }
  // Larger unsigned to smaller self.
  {
    auto i = sus::cast<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX_BIT_PATTERN);
  }
  // Larger unsigned self to smaller signed.
  {
    auto i = sus::cast<int16_t>(Self::MAX_BIT_PATTERN);
    static_assert(std::same_as<decltype(i), int16_t>);
    EXPECT_EQ(i, int16_t{-1});
  }
}

TEST(NumTransmogrify, usize) {
  using Self = usize;
  // Smaller negative to larger unsigned self.
  {
    auto i = sus::cast<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX - Self::try_from(i8::MAX).unwrap());
  }
  // Larger unsigned to smaller self.
  {
    auto i = sus::cast<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX);
  }
  // Larger unsigned self to smaller signed.
  {
    auto i = sus::cast<int16_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), int16_t>);
    EXPECT_EQ(i, int16_t{-1});
  }
}

TEST(NumTransmogrify, i8) {
  using Self = i8;
  // Smaller unsigned to larger signed self.
  {
    auto i = sus::cast<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, i8::MIN);
  }
  // Larger unsigned to smaller signed self.
  {
    auto i = sus::cast<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, -1_i8);
  }
  // Signed self to unsigned.
  {
    auto i = sus::cast<uint8_t>(-1_i8);
    static_assert(std::same_as<decltype(i), uint8_t>);
    EXPECT_EQ(i, u8::MAX_PRIMITIVE);
  }
}

TEST(NumTransmogrify, i16) {
  using Self = i16;
  // Smaller unsigned to larger signed self.
  {
    auto i = sus::cast<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, i8::MIN);
  }
  // Larger unsigned to smaller signed self.
  {
    auto i = sus::cast<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, -1_i8);
  }
  // Larger signed self to smaller unsigned.
  {
    auto i = sus::cast<uint16_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), uint16_t>);
    EXPECT_EQ(i, i16::MAX_PRIMITIVE);
  }
}

TEST(NumTransmogrify, i32) {
  using Self = i32;
  // Smaller unsigned to larger signed self.
  {
    auto i = sus::cast<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, i8::MIN);
  }
  // Larger unsigned to smaller signed self.
  {
    auto i = sus::cast<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, -1_i8);
  }
  // Larger signed self to smaller unsigned.
  {
    auto i = sus::cast<uint16_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), uint16_t>);
    EXPECT_EQ(i, u16::MAX_PRIMITIVE);
  }
}

TEST(NumTransmogrify, i64) {
  using Self = i64;
  // Smaller unsigned to larger signed self.
  {
    auto i = sus::cast<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, i8::MIN);
  }
  // Larger unsigned to smaller signed self.
  {
    auto i = sus::cast<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, -1_i8);
  }
  // Larger signed self to smaller unsigned.
  {
    auto i = sus::cast<uint16_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), uint16_t>);
    EXPECT_EQ(i, u16::MAX_PRIMITIVE);
  }
}

TEST(NumTransmogrify, isize) {
  using Self = isize;
  // Smaller unsigned to larger signed self.
  {
    auto i = sus::cast<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, i8::MIN);
  }
  // Larger unsigned to smaller signed self.
  {
    auto i = sus::cast<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, -1_i8);
  }
  // Larger signed self to smaller unsigned.
  {
    auto i = sus::cast<uint16_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), uint16_t>);
    EXPECT_EQ(i, u16::MAX_PRIMITIVE);
  }
}

TEST(NumTransmogrify, LosslessFloatConversion) {
  EXPECT_EQ(sus::cast<f64>(-1.8949651689383756e-14_f32),
            -1.8949651689383756e-14_f64);
  EXPECT_EQ(sus::cast<f32>(-1.8949651689383756e-14_f32),
            -1.8949651689383756e-14_f32);
  EXPECT_EQ(sus::cast<f64>(-4.59218127443847370761468605771e-102_f64),
            -4.59218127443847370761468605771e-102_f64);
}

TEST(NumTransmogrify, f64tof32) {
  EXPECT_EQ(sus::cast<f32>(f64::NAN).is_nan(), true);
  EXPECT_EQ(sus::cast<f32>(f64::INFINITY), f32::INFINITY);
  EXPECT_EQ(sus::cast<f32>(f64::NEG_INFINITY), f32::NEG_INFINITY);
  EXPECT_EQ(sus::cast<f32>(f64::MAX), f32::INFINITY);
  EXPECT_EQ(sus::cast<f32>(f64::MIN), f32::NEG_INFINITY);

  // Just past the valid range of values for f32 in either direciton. A
  // static_cast<float>(double) for these values would cause UB.
  EXPECT_EQ(
      sus::cast<f32>(sus::cast<f64>(f32::MIN).next_toward(f64::NEG_INFINITY)),
      f32::NEG_INFINITY);
  EXPECT_EQ(sus::cast<f32>(sus::cast<f64>(f32::MAX).next_toward(f64::INFINITY)),
            f32::INFINITY);

  // This is a value with bits set throughout the exponent and mantissa. Its
  // exponent is <= 127 and >= -126 so it's possible to represent it in f32.
  EXPECT_EQ(sus::cast<f32>(-4.59218127443847370761468605771e-102_f64),
            -4.59218127443847370761468605771e-102_f32);
}

TEST(NumTransmogrify, f32) {
  static_assert(std::same_as<decltype(sus::cast<u16>(0_f32)), u16>);

  // Float to smaller unsigned.
  {
    EXPECT_EQ(sus::cast<u16>(f32::NAN), 0_u16);

    EXPECT_EQ(sus::cast<u16>(0_f32), u16::MIN);
    EXPECT_EQ(sus::cast<u16>(-0_f32), u16::MIN);
    EXPECT_EQ(sus::cast<u16>(-0.00001_f32), u16::MIN);
    EXPECT_EQ(sus::cast<u16>(-99999999_f32), u16::MIN);
    EXPECT_EQ(sus::cast<u16>(f32::NEG_INFINITY), u16::MIN);

    EXPECT_EQ(sus::cast<u16>(0.1_f32), 0_u16);
    EXPECT_EQ(sus::cast<u16>(0.51_f32), 0_u16);
    EXPECT_EQ(sus::cast<u16>(0.9999_f32), 0_u16);
    EXPECT_EQ(sus::cast<u16>(1_f32), 1_u16);
    EXPECT_EQ(sus::cast<u16>(65535_f32), u16::MAX);
    EXPECT_EQ(sus::cast<u16>(65535.00001_f32), u16::MAX);
    EXPECT_EQ(sus::cast<u16>(65536_f32), u16::MAX);
    EXPECT_EQ(sus::cast<u16>(999999999_f32), u16::MAX);
    EXPECT_EQ(sus::cast<u16>(f32::INFINITY), u16::MAX);

    EXPECT_EQ(sus::cast<u8>(f32::NAN), 0_u8);
    EXPECT_EQ(sus::cast<u8>(-99999999_f32), u8::MIN);
    EXPECT_EQ(sus::cast<u8>(999999999_f32), u8::MAX);
    EXPECT_EQ(sus::cast<u8>(1.1_f32), 1_u8);
    EXPECT_EQ(sus::cast<u8>(0.9_f32), 0_u8);
  }
  // Float to smaller signed.
  {
    EXPECT_EQ(sus::cast<i16>(f32::NAN), 0_i16);

    EXPECT_EQ(sus::cast<i16>(0_f32), 0_i16);
    EXPECT_EQ(sus::cast<i16>(-0_f32), 0_i16);
    EXPECT_EQ(sus::cast<i16>(-0.00001_f32), 0_i16);
    EXPECT_EQ(sus::cast<i16>(-0.9999_f32), 0_i16);
    EXPECT_EQ(sus::cast<i16>(-1_f32), -1_i16);
    EXPECT_EQ(sus::cast<i16>(-32767.999_f32), -32767_i16);
    EXPECT_EQ(sus::cast<i16>(-32768_f32), i16::MIN);
    EXPECT_EQ(sus::cast<i16>(-32768.00001_f32), i16::MIN);
    EXPECT_EQ(sus::cast<i16>(-99999999_f32), i16::MIN);
    EXPECT_EQ(sus::cast<i16>(f32::NEG_INFINITY), i16::MIN);

    EXPECT_EQ(sus::cast<i16>(0.1_f32), 0_i16);
    EXPECT_EQ(sus::cast<i16>(0.51_f32), 0_i16);
    EXPECT_EQ(sus::cast<i16>(0.9999_f32), 0_i16);
    EXPECT_EQ(sus::cast<i16>(1_f32), 1_i16);
    EXPECT_EQ(sus::cast<i16>(32767.999_f32), i16::MAX);
    EXPECT_EQ(sus::cast<i16>(32767.00001_f32), i16::MAX);
    EXPECT_EQ(sus::cast<i16>(32767_f32), i16::MAX);
    EXPECT_EQ(sus::cast<i16>(999999999_f32), i16::MAX);
    EXPECT_EQ(sus::cast<i16>(f32::INFINITY), i16::MAX);

    EXPECT_EQ(sus::cast<i8>(f32::NAN), 0_i8);
    EXPECT_EQ(sus::cast<i8>(f32::NAN), 0_i8);
    EXPECT_EQ(sus::cast<i8>(-99999999_f32), i8::MIN);
    EXPECT_EQ(sus::cast<i8>(999999999_f32), i8::MAX);
    EXPECT_EQ(sus::cast<i8>(1.1_f32), 1_i8);
    EXPECT_EQ(sus::cast<i8>(0.9_f32), 0_i8);
    EXPECT_EQ(sus::cast<i8>(-1.1_f32), -1_i8);
    EXPECT_EQ(sus::cast<i8>(-0.9_f32), 0_i8);
  }

  // Float to larger unsigned.
  {
    EXPECT_EQ(sus::cast<u64>(f32::NAN), 0_u64);

    EXPECT_EQ(sus::cast<u64>(0_f32), u64::MIN);
    EXPECT_EQ(sus::cast<u64>(-0_f32), u64::MIN);
    EXPECT_EQ(sus::cast<u64>(-0.00001_f32), u64::MIN);
    EXPECT_EQ(sus::cast<u64>(-99999999_f32), u64::MIN);
    EXPECT_EQ(sus::cast<u64>(f32::NEG_INFINITY), u64::MIN);

    EXPECT_EQ(sus::cast<u64>(0.1_f32), 0_u64);
    EXPECT_EQ(sus::cast<u64>(0.51_f32), 0_u64);
    EXPECT_EQ(sus::cast<u64>(0.9999_f32), 0_u64);
    EXPECT_EQ(sus::cast<u64>(1_f32), 1_u64);
    EXPECT_LT(sus::cast<u64>((18446744073709551615_f32).next_toward(0_f32)),
              u64::MAX);
    EXPECT_EQ(sus::cast<u64>(18446744073709551615_f32), u64::MAX);
    EXPECT_EQ(sus::cast<u64>(18446744073709551615.00001_f32), u64::MAX);
    EXPECT_EQ(sus::cast<u64>(18446744073709551615_f32 + 1_f32), u64::MAX);
    EXPECT_EQ(sus::cast<u64>(18446744073709551615_f32 * 2_f32), u64::MAX);
    EXPECT_EQ(sus::cast<u64>(f32::INFINITY), u64::MAX);

    EXPECT_EQ(sus::cast<u32>(f32::NAN), 0_u32);
    EXPECT_EQ(sus::cast<u32>(-99999999999_f32), u32::MIN);
    EXPECT_EQ(sus::cast<u32>(99999999999_f32), u32::MAX);
    EXPECT_EQ(sus::cast<u32>(0.9_f32), 0_u32);
    EXPECT_EQ(sus::cast<u32>(1.1_f32), 1_u32);
  }
  // Float to larger signed.
  {
    EXPECT_EQ(sus::cast<i64>(f32::NAN), 0_i64);

    EXPECT_EQ(sus::cast<i64>(0_f32), 0_i64);
    EXPECT_EQ(sus::cast<i64>(-0_f32), 0_i64);
    EXPECT_EQ(sus::cast<i64>(-0.00001_f32), 0_i64);
    EXPECT_EQ(sus::cast<i64>(-0.9999_f32), 0_i64);
    EXPECT_EQ(sus::cast<i64>(-1_f32), -1_i64);
    EXPECT_GT(sus::cast<i64>((-9223372036854775808_f32).next_toward(0_f32)),
              i64::MIN);
    EXPECT_EQ(sus::cast<i64>(-9223372036854775808_f32), i64::MIN);
    EXPECT_EQ(sus::cast<i64>(-9223372036854775808.00001_f32), i64::MIN);
    EXPECT_EQ(sus::cast<i64>(-9999999999999999999_f32), i64::MIN);
    EXPECT_EQ(sus::cast<i64>(f32::NEG_INFINITY), i64::MIN);

    EXPECT_EQ(sus::cast<i64>(0.1_f32), 0_i64);
    EXPECT_EQ(sus::cast<i64>(0.51_f32), 0_i64);
    EXPECT_EQ(sus::cast<i64>(0.9999_f32), 0_i64);
    EXPECT_EQ(sus::cast<i64>(1_f32), 1_i64);
    EXPECT_LT(sus::cast<i64>((9223372036854775807_f32).next_toward(0_f32)),
              i64::MAX);
    EXPECT_EQ(sus::cast<i64>(9223372036854775807_f32), i64::MAX);
    EXPECT_EQ(sus::cast<i64>(92233720368547758075.00001_f32), i64::MAX);
    EXPECT_EQ(sus::cast<i64>(9223372036854775808_f32), i64::MAX);
    EXPECT_EQ(sus::cast<i64>(9999999999999999999_f32), i64::MAX);
    EXPECT_EQ(sus::cast<i64>(f32::INFINITY), i64::MAX);

    EXPECT_EQ(sus::cast<i32>(f32::NAN), 0_i32);
    EXPECT_EQ(sus::cast<i32>(-99999999999_f32), i32::MIN);
    EXPECT_EQ(sus::cast<i32>(999999999999_f32), i32::MAX);
    EXPECT_EQ(sus::cast<i32>(1.1_f32), 1_i32);
    EXPECT_EQ(sus::cast<i32>(0.9_f32), 0_i32);
    EXPECT_EQ(sus::cast<i32>(-1.1_f32), -1_i32);
    EXPECT_EQ(sus::cast<i32>(-0.9_f32), 0_i32);
  }

  // Ints to f32.
  {
    EXPECT_EQ(sus::cast<f32>(0_i8), 0_f32);
    EXPECT_EQ(sus::cast<f32>(0_u8), 0_f32);
    EXPECT_EQ(sus::cast<f32>(i16::MIN), -32768_f32);
    EXPECT_EQ(sus::cast<f32>(i16::MAX), 32767_f32);
    // These values are rounded in an implementation-defined way.
    EXPECT_EQ(sus::cast<f32>(i32::MIN), -2147483600_f32);
    EXPECT_EQ(sus::cast<f32>(i32::MAX), 2147483600_f32);
    EXPECT_EQ(sus::cast<f32>(i64::MIN), -9223372036854775808_f32);
    EXPECT_EQ(sus::cast<f32>(i64::MAX), 9223372036854775808_f32);
    EXPECT_EQ(sus::cast<f32>(u64::MIN), 0_f32);
    EXPECT_EQ(sus::cast<f32>(u64::MAX), 18446744073709551616e+0_f32);
  }
}

TEST(NumTransmogrify, f64) {
  static_assert(std::same_as<decltype(sus::cast<u16>(0_f64)), u16>);

  // Float to smaller unsigned.
  {
    EXPECT_EQ(sus::cast<u16>(f64::NAN), 0_u16);

    EXPECT_EQ(sus::cast<u16>(0_f64), u16::MIN);
    EXPECT_EQ(sus::cast<u16>(-0_f64), u16::MIN);
    EXPECT_EQ(sus::cast<u16>(-0.00001_f64), u16::MIN);
    EXPECT_EQ(sus::cast<u16>(-99999999_f64), u16::MIN);
    EXPECT_EQ(sus::cast<u16>(f64::NEG_INFINITY), u16::MIN);

    EXPECT_EQ(sus::cast<u16>(0.1_f64), 0_u16);
    EXPECT_EQ(sus::cast<u16>(0.51_f64), 0_u16);
    EXPECT_EQ(sus::cast<u16>(0.9999_f64), 0_u16);
    EXPECT_EQ(sus::cast<u16>(1_f64), 1_u16);
    EXPECT_EQ(sus::cast<u16>(65535_f64), u16::MAX);
    EXPECT_EQ(sus::cast<u16>(65535.00001_f64), u16::MAX);
    EXPECT_EQ(sus::cast<u16>(65536_f64), u16::MAX);
    EXPECT_EQ(sus::cast<u16>(999999999_f64), u16::MAX);
    EXPECT_EQ(sus::cast<u16>(f64::INFINITY), u16::MAX);
  }
  // Float to smaller signed.
  {
    EXPECT_EQ(sus::cast<i16>(f64::NAN), 0_i16);

    EXPECT_EQ(sus::cast<i16>(0_f64), 0_i16);
    EXPECT_EQ(sus::cast<i16>(-0_f64), 0_i16);
    EXPECT_EQ(sus::cast<i16>(-0.00001_f64), 0_i16);
    EXPECT_EQ(sus::cast<i16>(-0.9999_f64), 0_i16);
    EXPECT_EQ(sus::cast<i16>(-1_f64), -1_i16);
    EXPECT_EQ(sus::cast<i16>(-32767.999_f64), -32767_i16);
    EXPECT_EQ(sus::cast<i16>(-32768_f64), i16::MIN);
    EXPECT_EQ(sus::cast<i16>(-32768.00001_f64), i16::MIN);
    EXPECT_EQ(sus::cast<i16>(-99999999_f64), i16::MIN);
    EXPECT_EQ(sus::cast<i16>(f64::NEG_INFINITY), i16::MIN);

    EXPECT_EQ(sus::cast<i16>(0.1_f64), 0_i16);
    EXPECT_EQ(sus::cast<i16>(0.51_f64), 0_i16);
    EXPECT_EQ(sus::cast<i16>(0.9999_f64), 0_i16);
    EXPECT_EQ(sus::cast<i16>(1_f64), 1_i16);
    EXPECT_EQ(sus::cast<i16>(65535_f64), i16::MAX);
    EXPECT_EQ(sus::cast<i16>(65535.00001_f64), i16::MAX);
    EXPECT_EQ(sus::cast<i16>(65536_f64), i16::MAX);
    EXPECT_EQ(sus::cast<i16>(999999999_f64), i16::MAX);
    EXPECT_EQ(sus::cast<i16>(f64::INFINITY), i16::MAX);
  }

  // Float to unsigned.
  {
    EXPECT_EQ(sus::cast<u64>(f64::NAN), 0_u64);

    EXPECT_EQ(sus::cast<u64>(0_f64), u64::MIN);
    EXPECT_EQ(sus::cast<u64>(-0_f64), u64::MIN);
    EXPECT_EQ(sus::cast<u64>(-0.00001_f64), u64::MIN);
    EXPECT_EQ(sus::cast<u64>(-99999999_f64), u64::MIN);
    EXPECT_EQ(sus::cast<u64>(f64::NEG_INFINITY), u64::MIN);

    EXPECT_EQ(sus::cast<u64>(0.1_f64), 0_u64);
    EXPECT_EQ(sus::cast<u64>(0.51_f64), 0_u64);
    EXPECT_EQ(sus::cast<u64>(0.9999_f64), 0_u64);
    EXPECT_EQ(sus::cast<u64>(1_f64), 1_u64);
    EXPECT_LT(sus::cast<u64>((18446744073709551615_f64).next_toward(0_f64)),
              u64::MAX);
    EXPECT_EQ(sus::cast<u64>(18446744073709551615_f64), u64::MAX);
    EXPECT_EQ(sus::cast<u64>(18446744073709551615.00001_f64), u64::MAX);
    EXPECT_EQ(sus::cast<u64>(18446744073709551615_f64 + 1_f64), u64::MAX);
    EXPECT_EQ(sus::cast<u64>(18446744073709551615_f64 * 2_f64), u64::MAX);
    EXPECT_EQ(sus::cast<u64>(f64::INFINITY), u64::MAX);
  }
  // Float to signed.
  {
    EXPECT_EQ(sus::cast<i64>(f64::NAN), 0_i64);

    EXPECT_EQ(sus::cast<i64>(0_f64), 0_i64);
    EXPECT_EQ(sus::cast<i64>(-0_f64), 0_i64);
    EXPECT_EQ(sus::cast<i64>(-0.00001_f64), 0_i64);
    EXPECT_EQ(sus::cast<i64>(-0.9999_f64), 0_i64);
    EXPECT_EQ(sus::cast<i64>(-1_f64), -1_i64);
    EXPECT_GT(sus::cast<i64>((-9223372036854775808_f64).next_toward(0_f64)),
              i64::MIN);
    EXPECT_EQ(sus::cast<i64>(-9223372036854775808_f64), i64::MIN);
    EXPECT_EQ(sus::cast<i64>(-9223372036854775808.00001_f64), i64::MIN);
    EXPECT_EQ(sus::cast<i64>(-9223372036854775808_f64 * 2_f64), i64::MIN);
    EXPECT_EQ(sus::cast<i64>(f64::NEG_INFINITY), i64::MIN);

    EXPECT_EQ(sus::cast<i64>(0.1_f64), 0_i64);
    EXPECT_EQ(sus::cast<i64>(0.51_f64), 0_i64);
    EXPECT_EQ(sus::cast<i64>(0.9999_f64), 0_i64);
    EXPECT_EQ(sus::cast<i64>(1_f64), 1_i64);
    EXPECT_LT(sus::cast<i64>((9223372036854775807_f64).next_toward(0_f64)),
              i64::MAX);
    EXPECT_EQ(sus::cast<i64>(9223372036854775807_f64), i64::MAX);
    EXPECT_EQ(sus::cast<i64>(9223372036854775807.00001_f64), i64::MAX);
    EXPECT_EQ(sus::cast<i64>(9223372036854775807_f64 * 2_f64), i64::MAX);
    EXPECT_EQ(sus::cast<i64>(f64::INFINITY), i64::MAX);
  }

  // Ints to f64.
  {
    EXPECT_EQ(sus::cast<f64>(0_i8), 0_f64);
    EXPECT_EQ(sus::cast<f64>(0_u8), 0_f64);
    EXPECT_EQ(sus::cast<f64>(i16::MIN), -32768_f64);
    EXPECT_EQ(sus::cast<f64>(i16::MAX), 32767_f64);
    // These values are rounded in an implementation-defined way.
    EXPECT_EQ(sus::cast<f64>(i32::MIN), -2147483648_f64);
    EXPECT_EQ(sus::cast<f64>(i32::MAX), 2147483648_f64);
    EXPECT_EQ(sus::cast<f64>(i64::MIN), -9223372036854775808_f64);
    EXPECT_EQ(sus::cast<f64>(i64::MAX), 9223372036854775808_f64);
    EXPECT_EQ(sus::cast<f64>(u64::MIN), 0_f64);
    EXPECT_EQ(sus::cast<f64>(u64::MAX), 18446744073709551616e+0_f64);
  }
}

TEST(NumTransmogrify, stdbyte) {
  EXPECT_EQ(sus::cast<u8>(std::byte{0xff}), 0xff_u8);
  EXPECT_EQ(sus::cast<u32>(std::byte{0xff}), 0xff_u32);
  EXPECT_EQ(sus::cast<i8>(std::byte{0xff}), -1_i8);
  EXPECT_EQ(sus::cast<i32>(std::byte{0xff}), 0xff_i32);

  EXPECT_EQ(sus::cast<std::byte>(0xff_u8), std::byte{0xff});
  EXPECT_EQ(sus::cast<std::byte>(0xff_u32), std::byte{0xff});
  EXPECT_EQ(sus::cast<std::byte>(-1_i8), std::byte{0xff});
  EXPECT_EQ(sus::cast<std::byte>(0xff_i32), std::byte{0xff});

  // Truncating from integers.
  EXPECT_EQ(sus::cast<std::byte>(-2_i32), std::byte{0xfe});
  EXPECT_EQ(sus::cast<std::byte>(259_i32), std::byte{3});
}

enum E : uint16_t {
  EA = 1,
  EB = 2,
  ED = 4,
};

enum class EC : int16_t {
  A = 1,
  B = 2,
  D = 4,
};

TEST(NumTransmogrify, Enums) {
  static_assert(sus::construct::Cast<E, i8>);
  static_assert(sus::construct::Cast<E, i64>);
  static_assert(sus::construct::Cast<E, int16_t>);
  static_assert(sus::construct::Cast<E, intptr_t>);
  static_assert(sus::construct::Cast<E, u8>);
  static_assert(sus::construct::Cast<E, u64>);
  static_assert(sus::construct::Cast<E, uint16_t>);
  static_assert(sus::construct::Cast<E, size_t>);
  static_assert(!sus::construct::Cast<E, f32>);
  static_assert(!sus::construct::Cast<E, f64>);

  EXPECT_EQ(sus::cast<E>(1_i64), EA);
  EXPECT_EQ(sus::cast<E>(2_u64), EB);

  static_assert(sus::construct::Cast<EC, i8>);
  static_assert(sus::construct::Cast<EC, i64>);
  static_assert(sus::construct::Cast<EC, int16_t>);
  static_assert(sus::construct::Cast<EC, intptr_t>);
  static_assert(sus::construct::Cast<EC, u8>);
  static_assert(sus::construct::Cast<EC, u64>);
  static_assert(sus::construct::Cast<EC, uint16_t>);
  static_assert(sus::construct::Cast<EC, size_t>);
  static_assert(!sus::construct::Cast<EC, f32>);
  static_assert(!sus::construct::Cast<EC, f64>);

  EXPECT_EQ(sus::cast<EC>(2_i64), EC::B);
  EXPECT_EQ(sus::cast<EC>(4_u64), EC::D);
}

}  // namespace
