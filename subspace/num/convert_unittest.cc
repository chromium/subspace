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

#include "subspace/num/convert.h"

#include "googletest/include/gtest/gtest.h"
#include "subspace/prelude.h"

namespace {

template <class Self>
struct CheckToBits {
  static_assert(sus::construct::ToBits<Self, char>);
  static_assert(sus::construct::ToBits<Self, signed char>);
  static_assert(sus::construct::ToBits<Self, unsigned char>);
  static_assert(sus::construct::ToBits<Self, int8_t>);
  static_assert(sus::construct::ToBits<Self, int16_t>);
  static_assert(sus::construct::ToBits<Self, int32_t>);
  static_assert(sus::construct::ToBits<Self, int64_t>);
  static_assert(sus::construct::ToBits<Self, uint8_t>);
  static_assert(sus::construct::ToBits<Self, uint16_t>);
  static_assert(sus::construct::ToBits<Self, uint32_t>);
  static_assert(sus::construct::ToBits<Self, uint64_t>);
  static_assert(sus::construct::ToBits<Self, size_t>);
  static_assert(sus::construct::ToBits<Self, intptr_t>);
  static_assert(sus::construct::ToBits<Self, uintptr_t>);
  static_assert(sus::construct::ToBits<Self, std::byte>);
  static_assert(sus::construct::ToBits<Self, float>);
  static_assert(sus::construct::ToBits<Self, double>);

  static_assert(sus::construct::ToBits<Self, i8>);
  static_assert(sus::construct::ToBits<Self, i16>);
  static_assert(sus::construct::ToBits<Self, i32>);
  static_assert(sus::construct::ToBits<Self, i64>);
  static_assert(sus::construct::ToBits<Self, u8>);
  static_assert(sus::construct::ToBits<Self, u16>);
  static_assert(sus::construct::ToBits<Self, u32>);
  static_assert(sus::construct::ToBits<Self, u64>);
  static_assert(sus::construct::ToBits<Self, isize>);
  static_assert(sus::construct::ToBits<Self, usize>);
  static_assert(sus::construct::ToBits<Self, uptr>);

  static_assert(sus::construct::ToBits<char, Self>);
  static_assert(sus::construct::ToBits<signed char, Self>);
  static_assert(sus::construct::ToBits<unsigned char, Self>);
  static_assert(sus::construct::ToBits<int8_t, Self>);
  static_assert(sus::construct::ToBits<int16_t, Self>);
  static_assert(sus::construct::ToBits<int32_t, Self>);
  static_assert(sus::construct::ToBits<int64_t, Self>);
  static_assert(sus::construct::ToBits<uint8_t, Self>);
  static_assert(sus::construct::ToBits<uint16_t, Self>);
  static_assert(sus::construct::ToBits<uint32_t, Self>);
  static_assert(sus::construct::ToBits<uint64_t, Self>);
  static_assert(sus::construct::ToBits<size_t, Self>);
  static_assert(sus::construct::ToBits<intptr_t, Self>);
  static_assert(sus::construct::ToBits<uintptr_t, Self>);
  static_assert(sus::construct::ToBits<std::byte, Self>);
  static_assert(sus::construct::ToBits<float, Self>);
  static_assert(sus::construct::ToBits<double, Self>);

  static_assert(sus::construct::ToBits<i8, Self>);
  static_assert(sus::construct::ToBits<i16, Self>);
  static_assert(sus::construct::ToBits<i32, Self>);
  static_assert(sus::construct::ToBits<i64, Self>);
  static_assert(sus::construct::ToBits<u8, Self>);
  static_assert(sus::construct::ToBits<u16, Self>);
  static_assert(sus::construct::ToBits<u32, Self>);
  static_assert(sus::construct::ToBits<u64, Self>);
  static_assert(sus::construct::ToBits<isize, Self>);
  static_assert(sus::construct::ToBits<usize, Self>);
  static_assert(sus::construct::ToBits<uptr, Self>);
};

TEST(ConvertToBits, Satisfies) {
  CheckToBits<char>();
  CheckToBits<signed char>();
  CheckToBits<unsigned char>();
  CheckToBits<std::byte>();
  CheckToBits<int8_t>();
  CheckToBits<int16_t>();
  CheckToBits<int32_t>();
  CheckToBits<int64_t>();
  CheckToBits<uint8_t>();
  CheckToBits<uint16_t>();
  CheckToBits<uint32_t>();
  CheckToBits<uint64_t>();
  CheckToBits<size_t>();
  CheckToBits<intptr_t>();
  CheckToBits<uintptr_t>();

  CheckToBits<i8>();
  CheckToBits<i16>();
  CheckToBits<i32>();
  CheckToBits<i64>();
  CheckToBits<u8>();
  CheckToBits<u16>();
  CheckToBits<u32>();
  CheckToBits<u64>();
  CheckToBits<isize>();
  CheckToBits<usize>();
  CheckToBits<uptr>();
}

TEST(ConvertToBits, u8) {
  using Self = u8;
  // Negative to larger unsigned self.
  {
    auto i = sus::to_bits<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX - Self::try_from(i8::MAX).unwrap());
  }
  // Larger unsigned to smaller self.
  {
    auto i = sus::to_bits<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX);
  }
  // Unsigned self to signed.
  {
    auto i = sus::to_bits<int8_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), int8_t>);
    EXPECT_EQ(i, int8_t{-1});
  }
}

TEST(ConvertToBits, u16) {
  using Self = u16;
  // Smaller negative to larger unsigned self.
  {
    auto i = sus::to_bits<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX - Self::try_from(i8::MAX).unwrap());
  }
  // Larger unsigned to smaller self.
  {
    auto i = sus::to_bits<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX);
  }
  // Larger unsigned self to smaller signed.
  {
    auto i = sus::to_bits<int16_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), int16_t>);
    EXPECT_EQ(i, int16_t{-1});
  }
}

TEST(ConvertToBits, u32) {
  using Self = u32;
  // Smaller negative to larger unsigned self.
  {
    auto i = sus::to_bits<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX - Self::try_from(i8::MAX).unwrap());
  }
  // Larger unsigned to smaller self.
  {
    auto i = sus::to_bits<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX);
  }
  // Larger unsigned self to smaller signed.
  {
    auto i = sus::to_bits<int16_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), int16_t>);
    EXPECT_EQ(i, int16_t{-1});
  }
}

TEST(ConvertToBits, u64) {
  using Self = u64;
  // Smaller negative to larger unsigned self.
  {
    auto i = sus::to_bits<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX - Self::try_from(i8::MAX).unwrap());
  }
  // Larger unsigned to smaller self.
  {
    auto i = sus::to_bits<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX);
  }
  // Larger unsigned self to smaller signed.
  {
    auto i = sus::to_bits<int16_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), int16_t>);
    EXPECT_EQ(i, int16_t{-1});
  }
}

TEST(ConvertToBits, uptr) {
  using Self = uptr;
  // Smaller negative to larger unsigned self.
  {
    auto i = sus::to_bits<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, static_cast<uintptr_t>(i8::MIN_PRIMITIVE));
  }
  // Larger unsigned to smaller self.
  {
    auto i = sus::to_bits<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX_BIT_PATTERN);
  }
  // Larger unsigned self to smaller signed.
  {
    auto i = sus::to_bits<int16_t>(Self::MAX_BIT_PATTERN);
    static_assert(std::same_as<decltype(i), int16_t>);
    EXPECT_EQ(i, int16_t{-1});
  }
}

TEST(ConvertToBits, usize) {
  using Self = usize;
  // Smaller negative to larger unsigned self.
  {
    auto i = sus::to_bits<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX - Self::try_from(i8::MAX).unwrap());
  }
  // Larger unsigned to smaller self.
  {
    auto i = sus::to_bits<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX);
  }
  // Larger unsigned self to smaller signed.
  {
    auto i = sus::to_bits<int16_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), int16_t>);
    EXPECT_EQ(i, int16_t{-1});
  }
}

TEST(ConvertToBits, i8) {
  using Self = i8;
  // Smaller unsigned to larger signed self.
  {
    auto i = sus::to_bits<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, i8::MIN);
  }
  // Larger unsigned to smaller signed self.
  {
    auto i = sus::to_bits<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, -1_i8);
  }
  // Signed self to unsigned.
  {
    auto i = sus::to_bits<uint8_t>(-1_i8);
    static_assert(std::same_as<decltype(i), uint8_t>);
    EXPECT_EQ(i, u8::MAX_PRIMITIVE);
  }
}

TEST(ConvertToBits, i16) {
  using Self = i16;
  // Smaller unsigned to larger signed self.
  {
    auto i = sus::to_bits<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, i8::MIN);
  }
  // Larger unsigned to smaller signed self.
  {
    auto i = sus::to_bits<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, -1_i8);
  }
  // Larger signed self to smaller unsigned.
  {
    auto i = sus::to_bits<uint16_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), uint16_t>);
    EXPECT_EQ(i, i16::MAX_PRIMITIVE);
  }
}

TEST(ConvertToBits, i32) {
  using Self = i32;
  // Smaller unsigned to larger signed self.
  {
    auto i = sus::to_bits<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, i8::MIN);
  }
  // Larger unsigned to smaller signed self.
  {
    auto i = sus::to_bits<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, -1_i8);
  }
  // Larger signed self to smaller unsigned.
  {
    auto i = sus::to_bits<uint16_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), uint16_t>);
    EXPECT_EQ(i, u16::MAX_PRIMITIVE);
  }
}

TEST(ConvertToBits, i64) {
  using Self = i64;
  // Smaller unsigned to larger signed self.
  {
    auto i = sus::to_bits<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, i8::MIN);
  }
  // Larger unsigned to smaller signed self.
  {
    auto i = sus::to_bits<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, -1_i8);
  }
  // Larger signed self to smaller unsigned.
  {
    auto i = sus::to_bits<uint16_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), uint16_t>);
    EXPECT_EQ(i, u16::MAX_PRIMITIVE);
  }
}

TEST(ConvertToBits, isize) {
  using Self = isize;
  // Smaller unsigned to larger signed self.
  {
    auto i = sus::to_bits<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, i8::MIN);
  }
  // Larger unsigned to smaller signed self.
  {
    auto i = sus::to_bits<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, -1_i8);
  }
  // Larger signed self to smaller unsigned.
  {
    auto i = sus::to_bits<uint16_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), uint16_t>);
    EXPECT_EQ(i, u16::MAX_PRIMITIVE);
  }
}

TEST(ConvertToBits, LosslessFloatConversion) {
  EXPECT_EQ(sus::to_bits<f64>(-1.8949651689383756e-14_f32),
            -1.8949651689383756e-14_f64);
  EXPECT_EQ(sus::to_bits<f32>(-1.8949651689383756e-14_f32),
            -1.8949651689383756e-14_f32);
  EXPECT_EQ(sus::to_bits<f64>(-4.59218127443847370761468605771e-102_f64),
            -4.59218127443847370761468605771e-102_f64);
}

TEST(ConvertToBits, f64tof32) {
  EXPECT_EQ(sus::to_bits<f32>(f64::NAN).is_nan(), true);
  EXPECT_EQ(sus::to_bits<f32>(f64::INFINITY), f32::INFINITY);
  EXPECT_EQ(sus::to_bits<f32>(f64::NEG_INFINITY), f32::NEG_INFINITY);
  EXPECT_EQ(sus::to_bits<f32>(f64::MAX), f32::INFINITY);
  EXPECT_EQ(sus::to_bits<f32>(f64::MIN), f32::NEG_INFINITY);

  // Just past the valid range of values for f32 in either direciton. A
  // static_cast<float>(double) for these values would cause UB.
  EXPECT_EQ(sus::to_bits<f32>(
                sus::to_bits<f64>(f32::MIN).next_toward(f64::NEG_INFINITY)),
            f32::NEG_INFINITY);
  EXPECT_EQ(
      sus::to_bits<f32>(sus::to_bits<f64>(f32::MAX).next_toward(f64::INFINITY)),
      f32::INFINITY);

  // This is a value with bits set throughout the exponent and mantissa. Its
  // exponent is <= 127 and >= -126 so it's possible to represent it in f32.
  EXPECT_EQ(sus::to_bits<f32>(-4.59218127443847370761468605771e-102_f64),
            -4.59218127443847370761468605771e-102_f32);
}

TEST(ConvertToBits, f32) {
  static_assert(std::same_as<decltype(sus::to_bits<u16>(0_f32)), u16>);

  // Float to smaller unsigned.
  {
    EXPECT_EQ(sus::to_bits<u16>(f32::NAN), 0_u16);

    EXPECT_EQ(sus::to_bits<u16>(0_f32), u16::MIN);
    EXPECT_EQ(sus::to_bits<u16>(-0_f32), u16::MIN);
    EXPECT_EQ(sus::to_bits<u16>(-0.00001_f32), u16::MIN);
    EXPECT_EQ(sus::to_bits<u16>(-99999999_f32), u16::MIN);
    EXPECT_EQ(sus::to_bits<u16>(f32::NEG_INFINITY), u16::MIN);

    EXPECT_EQ(sus::to_bits<u16>(0.1_f32), 0_u16);
    EXPECT_EQ(sus::to_bits<u16>(0.51_f32), 0_u16);
    EXPECT_EQ(sus::to_bits<u16>(0.9999_f32), 0_u16);
    EXPECT_EQ(sus::to_bits<u16>(1_f32), 1_u16);
    EXPECT_EQ(sus::to_bits<u16>(65535_f32), u16::MAX);
    EXPECT_EQ(sus::to_bits<u16>(65535.00001_f32), u16::MAX);
    EXPECT_EQ(sus::to_bits<u16>(65536_f32), u16::MAX);
    EXPECT_EQ(sus::to_bits<u16>(999999999_f32), u16::MAX);
    EXPECT_EQ(sus::to_bits<u16>(f32::INFINITY), u16::MAX);
  }
  // Float to smaller signed.
  {
    EXPECT_EQ(sus::to_bits<i16>(f32::NAN), 0_i16);

    EXPECT_EQ(sus::to_bits<i16>(0_f32), 0_i16);
    EXPECT_EQ(sus::to_bits<i16>(-0_f32), 0_i16);
    EXPECT_EQ(sus::to_bits<i16>(-0.00001_f32), 0_i16);
    EXPECT_EQ(sus::to_bits<i16>(-0.9999_f32), 0_i16);
    EXPECT_EQ(sus::to_bits<i16>(-1_f32), -1_i16);
    EXPECT_EQ(sus::to_bits<i16>(-32767.999_f32), -32767_i16);
    EXPECT_EQ(sus::to_bits<i16>(-32768_f32), i16::MIN);
    EXPECT_EQ(sus::to_bits<i16>(-32768.00001_f32), i16::MIN);
    EXPECT_EQ(sus::to_bits<i16>(-99999999_f32), i16::MIN);
    EXPECT_EQ(sus::to_bits<i16>(f32::NEG_INFINITY), i16::MIN);

    EXPECT_EQ(sus::to_bits<i16>(0.1_f32), 0_i16);
    EXPECT_EQ(sus::to_bits<i16>(0.51_f32), 0_i16);
    EXPECT_EQ(sus::to_bits<i16>(0.9999_f32), 0_i16);
    EXPECT_EQ(sus::to_bits<i16>(1_f32), 1_i16);
    EXPECT_EQ(sus::to_bits<i16>(32767.999_f32), i16::MAX);
    EXPECT_EQ(sus::to_bits<i16>(32767.00001_f32), i16::MAX);
    EXPECT_EQ(sus::to_bits<i16>(32767_f32), i16::MAX);
    EXPECT_EQ(sus::to_bits<i16>(999999999_f32), i16::MAX);
    EXPECT_EQ(sus::to_bits<i16>(f32::INFINITY), i16::MAX);
  }

  // Float to larger unsigned.
  {
    EXPECT_EQ(sus::to_bits<u64>(f32::NAN), 0_u64);

    EXPECT_EQ(sus::to_bits<u64>(0_f32), u64::MIN);
    EXPECT_EQ(sus::to_bits<u64>(-0_f32), u64::MIN);
    EXPECT_EQ(sus::to_bits<u64>(-0.00001_f32), u64::MIN);
    EXPECT_EQ(sus::to_bits<u64>(-99999999_f32), u64::MIN);
    EXPECT_EQ(sus::to_bits<u64>(f32::NEG_INFINITY), u64::MIN);

    EXPECT_EQ(sus::to_bits<u64>(0.1_f32), 0_u64);
    EXPECT_EQ(sus::to_bits<u64>(0.51_f32), 0_u64);
    EXPECT_EQ(sus::to_bits<u64>(0.9999_f32), 0_u64);
    EXPECT_EQ(sus::to_bits<u64>(1_f32), 1_u64);
    EXPECT_LT(sus::to_bits<u64>((18446744073709551615_f32).next_toward(0_f32)),
              u64::MAX);
    EXPECT_EQ(sus::to_bits<u64>(18446744073709551615_f32), u64::MAX);
    EXPECT_EQ(sus::to_bits<u64>(18446744073709551615.00001_f32), u64::MAX);
    EXPECT_EQ(sus::to_bits<u64>(18446744073709551615_f32 + 1_f32), u64::MAX);
    EXPECT_EQ(sus::to_bits<u64>(18446744073709551615_f32 * 2_f32), u64::MAX);
    EXPECT_EQ(sus::to_bits<u64>(f32::INFINITY), u64::MAX);
  }
  // Float to larger signed.
  {
    EXPECT_EQ(sus::to_bits<i64>(f32::NAN), 0_i64);

    EXPECT_EQ(sus::to_bits<i64>(0_f32), 0_i64);
    EXPECT_EQ(sus::to_bits<i64>(-0_f32), 0_i64);
    EXPECT_EQ(sus::to_bits<i64>(-0.00001_f32), 0_i64);
    EXPECT_EQ(sus::to_bits<i64>(-0.9999_f32), 0_i64);
    EXPECT_EQ(sus::to_bits<i64>(-1_f32), -1_i64);
    EXPECT_GT(sus::to_bits<i64>((-9223372036854775808_f32).next_toward(0_f32)),
              i64::MIN);
    EXPECT_EQ(sus::to_bits<i64>(-9223372036854775808_f32), i64::MIN);
    EXPECT_EQ(sus::to_bits<i64>(-9223372036854775808.00001_f32), i64::MIN);
    EXPECT_EQ(sus::to_bits<i64>(-9999999999999999999_f32), i64::MIN);
    EXPECT_EQ(sus::to_bits<i64>(f32::NEG_INFINITY), i64::MIN);

    EXPECT_EQ(sus::to_bits<i64>(0.1_f32), 0_i64);
    EXPECT_EQ(sus::to_bits<i64>(0.51_f32), 0_i64);
    EXPECT_EQ(sus::to_bits<i64>(0.9999_f32), 0_i64);
    EXPECT_EQ(sus::to_bits<i64>(1_f32), 1_i64);
    EXPECT_LT(sus::to_bits<i64>((9223372036854775807_f32).next_toward(0_f32)),
              i64::MAX);
    EXPECT_EQ(sus::to_bits<i64>(9223372036854775807_f32), i64::MAX);
    EXPECT_EQ(sus::to_bits<i64>(92233720368547758075.00001_f32), i64::MAX);
    EXPECT_EQ(sus::to_bits<i64>(9223372036854775808_f32), i64::MAX);
    EXPECT_EQ(sus::to_bits<i64>(9999999999999999999_f32), i64::MAX);
    EXPECT_EQ(sus::to_bits<i64>(f32::INFINITY), i64::MAX);
  }

  // Ints to f32.
  {
    EXPECT_EQ(sus::to_bits<f32>(0_i8), 0_f32);
    EXPECT_EQ(sus::to_bits<f32>(0_u8), 0_f32);
    EXPECT_EQ(sus::to_bits<f32>(i16::MIN), -32768_f32);
    EXPECT_EQ(sus::to_bits<f32>(i16::MAX), 32767_f32);
    // These values are rounded in an implementation-defined way.
    EXPECT_EQ(sus::to_bits<f32>(i32::MIN), -2147483600_f32);
    EXPECT_EQ(sus::to_bits<f32>(i32::MAX), 2147483600_f32);
    EXPECT_EQ(sus::to_bits<f32>(i64::MIN), -9223372036854775808_f32);
    EXPECT_EQ(sus::to_bits<f32>(i64::MAX), 9223372036854775808_f32);
    EXPECT_EQ(sus::to_bits<f32>(u64::MIN), 0_f32);
    EXPECT_EQ(sus::to_bits<f32>(u64::MAX), 18446744073709551616e+0_f32);
  }
}

TEST(ConvertToBits, f64) {
  static_assert(std::same_as<decltype(sus::to_bits<u16>(0_f64)), u16>);

  // Float to smaller unsigned.
  {
    EXPECT_EQ(sus::to_bits<u16>(f64::NAN), 0_u16);

    EXPECT_EQ(sus::to_bits<u16>(0_f64), u16::MIN);
    EXPECT_EQ(sus::to_bits<u16>(-0_f64), u16::MIN);
    EXPECT_EQ(sus::to_bits<u16>(-0.00001_f64), u16::MIN);
    EXPECT_EQ(sus::to_bits<u16>(-99999999_f64), u16::MIN);
    EXPECT_EQ(sus::to_bits<u16>(f64::NEG_INFINITY), u16::MIN);

    EXPECT_EQ(sus::to_bits<u16>(0.1_f64), 0_u16);
    EXPECT_EQ(sus::to_bits<u16>(0.51_f64), 0_u16);
    EXPECT_EQ(sus::to_bits<u16>(0.9999_f64), 0_u16);
    EXPECT_EQ(sus::to_bits<u16>(1_f64), 1_u16);
    EXPECT_EQ(sus::to_bits<u16>(65535_f64), u16::MAX);
    EXPECT_EQ(sus::to_bits<u16>(65535.00001_f64), u16::MAX);
    EXPECT_EQ(sus::to_bits<u16>(65536_f64), u16::MAX);
    EXPECT_EQ(sus::to_bits<u16>(999999999_f64), u16::MAX);
    EXPECT_EQ(sus::to_bits<u16>(f64::INFINITY), u16::MAX);
  }
  // Float to smaller signed.
  {
    EXPECT_EQ(sus::to_bits<i16>(f64::NAN), 0_i16);

    EXPECT_EQ(sus::to_bits<i16>(0_f64), 0_i16);
    EXPECT_EQ(sus::to_bits<i16>(-0_f64), 0_i16);
    EXPECT_EQ(sus::to_bits<i16>(-0.00001_f64), 0_i16);
    EXPECT_EQ(sus::to_bits<i16>(-0.9999_f64), 0_i16);
    EXPECT_EQ(sus::to_bits<i16>(-1_f64), -1_i16);
    EXPECT_EQ(sus::to_bits<i16>(-32767.999_f64), -32767_i16);
    EXPECT_EQ(sus::to_bits<i16>(-32768_f64), i16::MIN);
    EXPECT_EQ(sus::to_bits<i16>(-32768.00001_f64), i16::MIN);
    EXPECT_EQ(sus::to_bits<i16>(-99999999_f64), i16::MIN);
    EXPECT_EQ(sus::to_bits<i16>(f64::NEG_INFINITY), i16::MIN);

    EXPECT_EQ(sus::to_bits<i16>(0.1_f64), 0_i16);
    EXPECT_EQ(sus::to_bits<i16>(0.51_f64), 0_i16);
    EXPECT_EQ(sus::to_bits<i16>(0.9999_f64), 0_i16);
    EXPECT_EQ(sus::to_bits<i16>(1_f64), 1_i16);
    EXPECT_EQ(sus::to_bits<i16>(65535_f64), i16::MAX);
    EXPECT_EQ(sus::to_bits<i16>(65535.00001_f64), i16::MAX);
    EXPECT_EQ(sus::to_bits<i16>(65536_f64), i16::MAX);
    EXPECT_EQ(sus::to_bits<i16>(999999999_f64), i16::MAX);
    EXPECT_EQ(sus::to_bits<i16>(f64::INFINITY), i16::MAX);
  }

  // Float to unsigned.
  {
    EXPECT_EQ(sus::to_bits<u64>(f64::NAN), 0_u64);

    EXPECT_EQ(sus::to_bits<u64>(0_f64), u64::MIN);
    EXPECT_EQ(sus::to_bits<u64>(-0_f64), u64::MIN);
    EXPECT_EQ(sus::to_bits<u64>(-0.00001_f64), u64::MIN);
    EXPECT_EQ(sus::to_bits<u64>(-99999999_f64), u64::MIN);
    EXPECT_EQ(sus::to_bits<u64>(f64::NEG_INFINITY), u64::MIN);

    EXPECT_EQ(sus::to_bits<u64>(0.1_f64), 0_u64);
    EXPECT_EQ(sus::to_bits<u64>(0.51_f64), 0_u64);
    EXPECT_EQ(sus::to_bits<u64>(0.9999_f64), 0_u64);
    EXPECT_EQ(sus::to_bits<u64>(1_f64), 1_u64);
    EXPECT_LT(sus::to_bits<u64>((18446744073709551615_f64).next_toward(0_f64)),
              u64::MAX);
    EXPECT_EQ(sus::to_bits<u64>(18446744073709551615_f64), u64::MAX);
    EXPECT_EQ(sus::to_bits<u64>(18446744073709551615.00001_f64), u64::MAX);
    EXPECT_EQ(sus::to_bits<u64>(18446744073709551615_f64 + 1_f64), u64::MAX);
    EXPECT_EQ(sus::to_bits<u64>(18446744073709551615_f64 * 2_f64), u64::MAX);
    EXPECT_EQ(sus::to_bits<u64>(f64::INFINITY), u64::MAX);
  }
  // Float to signed.
  {
    EXPECT_EQ(sus::to_bits<i64>(f64::NAN), 0_i64);

    EXPECT_EQ(sus::to_bits<i64>(0_f64), 0_i64);
    EXPECT_EQ(sus::to_bits<i64>(-0_f64), 0_i64);
    EXPECT_EQ(sus::to_bits<i64>(-0.00001_f64), 0_i64);
    EXPECT_EQ(sus::to_bits<i64>(-0.9999_f64), 0_i64);
    EXPECT_EQ(sus::to_bits<i64>(-1_f64), -1_i64);
    EXPECT_GT(sus::to_bits<i64>((-9223372036854775808_f64).next_toward(0_f64)),
              i64::MIN);
    EXPECT_EQ(sus::to_bits<i64>(-9223372036854775808_f64), i64::MIN);
    EXPECT_EQ(sus::to_bits<i64>(-9223372036854775808.00001_f64), i64::MIN);
    EXPECT_EQ(sus::to_bits<i64>(-9223372036854775808_f64 * 2_f64), i64::MIN);
    EXPECT_EQ(sus::to_bits<i64>(f64::NEG_INFINITY), i64::MIN);

    EXPECT_EQ(sus::to_bits<i64>(0.1_f64), 0_i64);
    EXPECT_EQ(sus::to_bits<i64>(0.51_f64), 0_i64);
    EXPECT_EQ(sus::to_bits<i64>(0.9999_f64), 0_i64);
    EXPECT_EQ(sus::to_bits<i64>(1_f64), 1_i64);
    EXPECT_LT(sus::to_bits<i64>((9223372036854775807_f64).next_toward(0_f64)),
              i64::MAX);
    EXPECT_EQ(sus::to_bits<i64>(9223372036854775807_f64), i64::MAX);
    EXPECT_EQ(sus::to_bits<i64>(9223372036854775807.00001_f64), i64::MAX);
    EXPECT_EQ(sus::to_bits<i64>(9223372036854775807_f64 * 2_f64), i64::MAX);
    EXPECT_EQ(sus::to_bits<i64>(f64::INFINITY), i64::MAX);
  }

  // Ints to f64.
  {
    EXPECT_EQ(sus::to_bits<f64>(0_i8), 0_f64);
    EXPECT_EQ(sus::to_bits<f64>(0_u8), 0_f64);
    EXPECT_EQ(sus::to_bits<f64>(i16::MIN), -32768_f64);
    EXPECT_EQ(sus::to_bits<f64>(i16::MAX), 32767_f64);
    // These values are rounded in an implementation-defined way.
    EXPECT_EQ(sus::to_bits<f64>(i32::MIN), -2147483648_f64);
    EXPECT_EQ(sus::to_bits<f64>(i32::MAX), 2147483648_f64);
    EXPECT_EQ(sus::to_bits<f64>(i64::MIN), -9223372036854775808_f64);
    EXPECT_EQ(sus::to_bits<f64>(i64::MAX), 9223372036854775808_f64);
    EXPECT_EQ(sus::to_bits<f64>(u64::MIN), 0_f64);
    EXPECT_EQ(sus::to_bits<f64>(u64::MAX), 18446744073709551616e+0_f64);
  }
}

TEST(ConvertToBits, stdbyte) {
  EXPECT_EQ(sus::to_bits<u8>(std::byte{0xff}), 0xff_u8);
  EXPECT_EQ(sus::to_bits<u32>(std::byte{0xff}), 0xff_u32);
  EXPECT_EQ(sus::to_bits<i8>(std::byte{0xff}), -1_i8);
  EXPECT_EQ(sus::to_bits<i32>(std::byte{0xff}), 0xff_i32);
  EXPECT_EQ(sus::to_bits<f32>(std::byte{0xff}), 0xff_f32);
  EXPECT_EQ(sus::to_bits<f64>(std::byte{0xff}), 0xff_f64);

  EXPECT_EQ(sus::to_bits<std::byte>(0xff_u8), std::byte{0xff});
  EXPECT_EQ(sus::to_bits<std::byte>(0xff_u32), std::byte{0xff});
  EXPECT_EQ(sus::to_bits<std::byte>(-1_i8), std::byte{0xff});
  EXPECT_EQ(sus::to_bits<std::byte>(0xff_i32), std::byte{0xff});
  EXPECT_EQ(sus::to_bits<std::byte>(0xff_f32), std::byte{0xff});
  EXPECT_EQ(sus::to_bits<std::byte>(0xff_f64), std::byte{0xff});

  // Truncating from integer.
  EXPECT_EQ(sus::to_bits<std::byte>(-2_i32), std::byte{0xfe});
  EXPECT_EQ(sus::to_bits<std::byte>(259_i32), std::byte{3});

  // Saturating from float.
  EXPECT_EQ(sus::to_bits<std::byte>(-1_f64), std::byte{0x00});
  EXPECT_EQ(sus::to_bits<std::byte>(256_f64), std::byte{0xff});
}

}  // namespace
