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

TEST(ConvertToBits, f32) {
  // Bit value 0b1100'0000'0101'0101'1101'0101'0000'1110.
  constexpr auto v = -3.34112882614_f32;

  // Float to smaller unsigned.
  {
    auto i = sus::to_bits<u16>(v);
    static_assert(std::same_as<decltype(i), u16>);
    EXPECT_EQ(i, 0b1101'0101'0000'1110_u16);
    auto f = sus::to_bits<f32>(i);
    EXPECT_EQ(f, 7.64296208412e-41_f32);
  }
  // Float to larger unsigned.
  {
    auto i = sus::to_bits<u64>(v);
    static_assert(std::same_as<decltype(i), u64>);
    EXPECT_EQ(i, 0b1100'0000'0101'0101'1101'0101'0000'1110);
    auto f = sus::to_bits<f32>(i);
    EXPECT_EQ(f, v);
  }
  // Float to smaller signed.
  {
    auto i = sus::to_bits<i16>(v);
    static_assert(std::same_as<decltype(i), i16>);
    EXPECT_EQ(i, sus::to_bits<i16>(0b1101'0101'0000'1110_u16));
    EXPECT_EQ(i, -10994);
    auto f = sus::to_bits<f32>(i);
    EXPECT_EQ(f, 7.64296208412e-41_f32);
  }
  // Float to larger signed.
  {
    auto i = sus::to_bits<i64>(v);
    static_assert(std::same_as<decltype(i), i64>);
    EXPECT_EQ(i, sus::to_bits<i64>(0b1100'0000'0101'0101'1101'0101'0000'1110));
    EXPECT_EQ(i, 3226850574);
    auto f = sus::to_bits<f32>(i);
    EXPECT_EQ(f, v);
  }
  // Float to double.
  {
    auto i = sus::to_bits<f64>(v);
    static_assert(std::same_as<decltype(i), f64>);
    EXPECT_EQ(i, 1.59427601287650712395167736662e-314_f64);
    auto f = sus::to_bits<f32>(i);
    EXPECT_EQ(f, v);
  }
}

TEST(ConvertToBits, f64) {
  // Bit value
  // 0b10101000'10101010'10101010'00101010'10101000'10101010'10101110'11101110.
  constexpr auto v = -8.66220694718676082014277885371e-113_f64;

  // Double to smaller unsigned.
  {
    auto i = sus::to_bits<u16>(v);
    static_assert(std::same_as<decltype(i), u16>);
    EXPECT_EQ(i, 0b10101110'11101110_u16);
    auto f = sus::to_bits<f64>(i);
    EXPECT_EQ(f, 2.21252477520627027413151036822e-319_f64);
  }
  // Double to unsigned.
  {
    auto i = sus::to_bits<u64>(v);
    static_assert(std::same_as<decltype(i), u64>);
    EXPECT_EQ(i, 0b10101000'10101010'10101010'00101010'10101000'10101010'10101110'11101110_u64);
    auto f = sus::to_bits<f64>(i);
    EXPECT_EQ(f, v);
  }
  // Double to smaller signed.
  {
    auto i = sus::to_bits<i16>(v);
    static_assert(std::same_as<decltype(i), i16>);
    EXPECT_EQ(i, sus::to_bits<i16>(0b10101110'11101110_u16));
    EXPECT_EQ(i, -20754);
    auto f = sus::to_bits<f64>(i);
    EXPECT_EQ(f, 2.21252477520627027413151036822e-319_f64);
  }
  // Double to signed.
  {
    auto i = sus::to_bits<i64>(v);
    static_assert(std::same_as<decltype(i), i64>);
    EXPECT_EQ(i, sus::to_bits<i64>(0b10101000'10101010'10101010'00101010'10101000'10101010'10101110'11101110_u64));
    EXPECT_EQ(i, -6293030429101740306);
    auto f = sus::to_bits<f64>(i);
    EXPECT_EQ(f, v);
  }
  // Double to float.
  {
    auto i = sus::to_bits<f32>(v);
    static_assert(std::same_as<decltype(i), f32>);
    EXPECT_EQ(i, -1.89496550775e-14_f32);
    auto f = sus::to_bits<f64>(i);
    EXPECT_EQ(f, 1.39808630771690684819284234772e-314_f64);
  }
}

}  // namespace
