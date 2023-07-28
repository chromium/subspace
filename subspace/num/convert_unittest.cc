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
struct CheckAsBits {
  static_assert(sus::construct::AsBits<Self, char>);
  static_assert(sus::construct::AsBits<Self, signed char>);
  static_assert(sus::construct::AsBits<Self, unsigned char>);
  static_assert(sus::construct::AsBits<Self, int8_t>);
  static_assert(sus::construct::AsBits<Self, int16_t>);
  static_assert(sus::construct::AsBits<Self, int32_t>);
  static_assert(sus::construct::AsBits<Self, int64_t>);
  static_assert(sus::construct::AsBits<Self, uint8_t>);
  static_assert(sus::construct::AsBits<Self, uint16_t>);
  static_assert(sus::construct::AsBits<Self, uint32_t>);
  static_assert(sus::construct::AsBits<Self, uint64_t>);
  static_assert(sus::construct::AsBits<Self, size_t>);
  static_assert(sus::construct::AsBits<Self, intptr_t>);
  static_assert(sus::construct::AsBits<Self, uintptr_t>);
  static_assert(sus::construct::AsBits<Self, std::byte>);
  static_assert(sus::construct::AsBits<Self, float>);
  static_assert(sus::construct::AsBits<Self, double>);

  static_assert(sus::construct::AsBits<Self, i8>);
  static_assert(sus::construct::AsBits<Self, i16>);
  static_assert(sus::construct::AsBits<Self, i32>);
  static_assert(sus::construct::AsBits<Self, i64>);
  static_assert(sus::construct::AsBits<Self, u8>);
  static_assert(sus::construct::AsBits<Self, u16>);
  static_assert(sus::construct::AsBits<Self, u32>);
  static_assert(sus::construct::AsBits<Self, u64>);
  static_assert(sus::construct::AsBits<Self, isize>);
  static_assert(sus::construct::AsBits<Self, usize>);
  static_assert(sus::construct::AsBits<Self, uptr>);

  static_assert(sus::construct::AsBits<char, Self>);
  static_assert(sus::construct::AsBits<signed char, Self>);
  static_assert(sus::construct::AsBits<unsigned char, Self>);
  static_assert(sus::construct::AsBits<int8_t, Self>);
  static_assert(sus::construct::AsBits<int16_t, Self>);
  static_assert(sus::construct::AsBits<int32_t, Self>);
  static_assert(sus::construct::AsBits<int64_t, Self>);
  static_assert(sus::construct::AsBits<uint8_t, Self>);
  static_assert(sus::construct::AsBits<uint16_t, Self>);
  static_assert(sus::construct::AsBits<uint32_t, Self>);
  static_assert(sus::construct::AsBits<uint64_t, Self>);
  static_assert(sus::construct::AsBits<size_t, Self>);
  static_assert(sus::construct::AsBits<intptr_t, Self>);
  static_assert(sus::construct::AsBits<uintptr_t, Self>);
  static_assert(sus::construct::AsBits<std::byte, Self>);
  static_assert(sus::construct::AsBits<float, Self>);
  static_assert(sus::construct::AsBits<double, Self>);

  static_assert(sus::construct::AsBits<i8, Self>);
  static_assert(sus::construct::AsBits<i16, Self>);
  static_assert(sus::construct::AsBits<i32, Self>);
  static_assert(sus::construct::AsBits<i64, Self>);
  static_assert(sus::construct::AsBits<u8, Self>);
  static_assert(sus::construct::AsBits<u16, Self>);
  static_assert(sus::construct::AsBits<u32, Self>);
  static_assert(sus::construct::AsBits<u64, Self>);
  static_assert(sus::construct::AsBits<isize, Self>);
  static_assert(sus::construct::AsBits<usize, Self>);
  static_assert(sus::construct::AsBits<uptr, Self>);
};

TEST(ConvertAsBits, Satisfies) {
  CheckAsBits<char>();
  CheckAsBits<signed char>();
  CheckAsBits<unsigned char>();
  CheckAsBits<std::byte>();
  CheckAsBits<int8_t>();
  CheckAsBits<int16_t>();
  CheckAsBits<int32_t>();
  CheckAsBits<int64_t>();
  CheckAsBits<uint8_t>();
  CheckAsBits<uint16_t>();
  CheckAsBits<uint32_t>();
  CheckAsBits<uint64_t>();
  CheckAsBits<size_t>();
  CheckAsBits<intptr_t>();
  CheckAsBits<uintptr_t>();

  CheckAsBits<i8>();
  CheckAsBits<i16>();
  CheckAsBits<i32>();
  CheckAsBits<i64>();
  CheckAsBits<u8>();
  CheckAsBits<u16>();
  CheckAsBits<u32>();
  CheckAsBits<u64>();
  CheckAsBits<isize>();
  CheckAsBits<usize>();
  CheckAsBits<uptr>();
}

TEST(ConvertAsBits, u8) {
  using Self = u8;
  // Negative to larger unsigned self.
  {
    auto i = sus::as_bits<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX - Self::try_from(i8::MAX).unwrap());
  }
  // Larger unsigned to smaller self.
  {
    auto i = sus::as_bits<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX);
  }
  // Unsigned self to signed.
  {
    auto i = sus::as_bits<int8_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), int8_t>);
    EXPECT_EQ(i, int8_t{-1});
  }
}

TEST(ConvertAsBits, u16) {
  using Self = u16;
  // Smaller negative to larger unsigned self.
  {
    auto i = sus::as_bits<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX - Self::try_from(i8::MAX).unwrap());
  }
  // Larger unsigned to smaller self.
  {
    auto i = sus::as_bits<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX);
  }
  // Larger unsigned self to smaller signed.
  {
    auto i = sus::as_bits<int16_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), int16_t>);
    EXPECT_EQ(i, int16_t{-1});
  }
}

TEST(ConvertAsBits, u32) {
  using Self = u32;
  // Smaller negative to larger unsigned self.
  {
    auto i = sus::as_bits<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX - Self::try_from(i8::MAX).unwrap());
  }
  // Larger unsigned to smaller self.
  {
    auto i = sus::as_bits<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX);
  }
  // Larger unsigned self to smaller signed.
  {
    auto i = sus::as_bits<int16_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), int16_t>);
    EXPECT_EQ(i, int16_t{-1});
  }
}

TEST(ConvertAsBits, u64) {
  using Self = u64;
  // Smaller negative to larger unsigned self.
  {
    auto i = sus::as_bits<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX - Self::try_from(i8::MAX).unwrap());
  }
  // Larger unsigned to smaller self.
  {
    auto i = sus::as_bits<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX);
  }
  // Larger unsigned self to smaller signed.
  {
    auto i = sus::as_bits<int16_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), int16_t>);
    EXPECT_EQ(i, int16_t{-1});
  }
}

TEST(ConvertAsBits, uptr) {
  using Self = uptr;
  // Smaller negative to larger unsigned self.
  {
    auto i = sus::as_bits<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, static_cast<uintptr_t>(i8::MIN_PRIMITIVE));
  }
  // Larger unsigned to smaller self.
  {
    auto i = sus::as_bits<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX_BIT_PATTERN);
  }
  // Larger unsigned self to smaller signed.
  {
    auto i = sus::as_bits<int16_t>(Self::MAX_BIT_PATTERN);
    static_assert(std::same_as<decltype(i), int16_t>);
    EXPECT_EQ(i, int16_t{-1});
  }
}

TEST(ConvertAsBits, usize) {
  using Self = usize;
  // Smaller negative to larger unsigned self.
  {
    auto i = sus::as_bits<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX - Self::try_from(i8::MAX).unwrap());
  }
  // Larger unsigned to smaller self.
  {
    auto i = sus::as_bits<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, Self::MAX);
  }
  // Larger unsigned self to smaller signed.
  {
    auto i = sus::as_bits<int16_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), int16_t>);
    EXPECT_EQ(i, int16_t{-1});
  }
}

TEST(ConvertAsBits, i8) {
  using Self = i8;
  // Smaller unsigned to larger signed self.
  {
    auto i = sus::as_bits<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, i8::MIN);
  }
  // Larger unsigned to smaller signed self.
  {
    auto i = sus::as_bits<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, -1_i8);
  }
  // Signed self to unsigned.
  {
    auto i = sus::as_bits<uint8_t>(-1_i8);
    static_assert(std::same_as<decltype(i), uint8_t>);
    EXPECT_EQ(i, u8::MAX_PRIMITIVE);
  }
}

TEST(ConvertAsBits, i16) {
  using Self = i16;
  // Smaller unsigned to larger signed self.
  {
    auto i = sus::as_bits<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, i8::MIN);
  }
  // Larger unsigned to smaller signed self.
  {
    auto i = sus::as_bits<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, -1_i8);
  }
  // Larger signed self to smaller unsigned.
  {
    auto i = sus::as_bits<uint16_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), uint16_t>);
    EXPECT_EQ(i, i16::MAX_PRIMITIVE);
  }
}

TEST(ConvertAsBits, i32) {
  using Self = i32;
  // Smaller unsigned to larger signed self.
  {
    auto i = sus::as_bits<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, i8::MIN);
  }
  // Larger unsigned to smaller signed self.
  {
    auto i = sus::as_bits<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, -1_i8);
  }
  // Larger signed self to smaller unsigned.
  {
    auto i = sus::as_bits<uint16_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), uint16_t>);
    EXPECT_EQ(i, u16::MAX_PRIMITIVE);
  }
}

TEST(ConvertAsBits, i64) {
  using Self = i64;
  // Smaller unsigned to larger signed self.
  {
    auto i = sus::as_bits<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, i8::MIN);
  }
  // Larger unsigned to smaller signed self.
  {
    auto i = sus::as_bits<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, -1_i8);
  }
  // Larger signed self to smaller unsigned.
  {
    auto i = sus::as_bits<uint16_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), uint16_t>);
    EXPECT_EQ(i, u16::MAX_PRIMITIVE);
  }
}

TEST(ConvertAsBits, isize) {
  using Self = isize;
  // Smaller unsigned to larger signed self.
  {
    auto i = sus::as_bits<Self>(i8::MIN);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, i8::MIN);
  }
  // Larger unsigned to smaller signed self.
  {
    auto i = sus::as_bits<Self>(u64::MAX);
    static_assert(std::same_as<decltype(i), Self>);
    EXPECT_EQ(i, -1_i8);
  }
  // Larger signed self to smaller unsigned.
  {
    auto i = sus::as_bits<uint16_t>(Self::MAX);
    static_assert(std::same_as<decltype(i), uint16_t>);
    EXPECT_EQ(i, u16::MAX_PRIMITIVE);
  }
}

TEST(ConvertAsBits, f32) {
  // Bit value 0b1100'0000'0101'0101'1101'0101'0000'1110.
  constexpr auto v = -3.34112882614_f32;

  // Float to smaller unsigned.
  {
    auto i = sus::as_bits<u16>(v);
    static_assert(std::same_as<decltype(i), u16>);
    EXPECT_EQ(i, 0b1101'0101'0000'1110_u16);
    auto f = sus::as_bits<f32>(i);
    EXPECT_EQ(f, 7.64296208412e-41_f32);
  }
  // Float to larger unsigned.
  {
    auto i = sus::as_bits<u64>(v);
    static_assert(std::same_as<decltype(i), u64>);
    EXPECT_EQ(i, 0b1100'0000'0101'0101'1101'0101'0000'1110);
    auto f = sus::as_bits<f32>(i);
    EXPECT_EQ(f, v);
  }
  // Float to smaller signed.
  {
    auto i = sus::as_bits<i16>(v);
    static_assert(std::same_as<decltype(i), i16>);
    EXPECT_EQ(i, sus::as_bits<i16>(0b1101'0101'0000'1110_u16));
    EXPECT_EQ(i, -10994);
    auto f = sus::as_bits<f32>(i);
    EXPECT_EQ(f, 7.64296208412e-41_f32);
  }
  // Float to larger signed.
  {
    auto i = sus::as_bits<i64>(v);
    static_assert(std::same_as<decltype(i), i64>);
    EXPECT_EQ(i, sus::as_bits<i64>(0b1100'0000'0101'0101'1101'0101'0000'1110));
    EXPECT_EQ(i, 3226850574);
    auto f = sus::as_bits<f32>(i);
    EXPECT_EQ(f, v);
  }
  // Float to double.
  {
    auto i = sus::as_bits<f64>(v);
    static_assert(std::same_as<decltype(i), f64>);
    EXPECT_EQ(i, 1.59427601287650712395167736662e-314_f64);
    auto f = sus::as_bits<f32>(i);
    EXPECT_EQ(f, v);
  }
}

TEST(ConvertAsBits, f64) {
  // Bit value
  // 0b10101000'10101010'10101010'00101010'10101000'10101010'10101110'11101110.
  constexpr auto v = -8.66220694718676082014277885371e-113_f64;

  // Double to smaller unsigned.
  {
    auto i = sus::as_bits<u16>(v);
    static_assert(std::same_as<decltype(i), u16>);
    EXPECT_EQ(i, 0b10101110'11101110_u16);
    auto f = sus::as_bits<f64>(i);
    EXPECT_EQ(f, 2.21252477520627027413151036822e-319_f64);
  }
  // Double to unsigned.
  {
    auto i = sus::as_bits<u64>(v);
    static_assert(std::same_as<decltype(i), u64>);
    EXPECT_EQ(i, 0b10101000'10101010'10101010'00101010'10101000'10101010'10101110'11101110_u64);
    auto f = sus::as_bits<f64>(i);
    EXPECT_EQ(f, v);
  }
  // Double to smaller signed.
  {
    auto i = sus::as_bits<i16>(v);
    static_assert(std::same_as<decltype(i), i16>);
    EXPECT_EQ(i, sus::as_bits<i16>(0b10101110'11101110_u16));
    EXPECT_EQ(i, -20754);
    auto f = sus::as_bits<f64>(i);
    EXPECT_EQ(f, 2.21252477520627027413151036822e-319_f64);
  }
  // Double to signed.
  {
    auto i = sus::as_bits<i64>(v);
    static_assert(std::same_as<decltype(i), i64>);
    EXPECT_EQ(i, sus::as_bits<i64>(0b10101000'10101010'10101010'00101010'10101000'10101010'10101110'11101110_u64));
    EXPECT_EQ(i, -6293030429101740306);
    auto f = sus::as_bits<f64>(i);
    EXPECT_EQ(f, v);
  }
  // Double to float.
  {
    auto i = sus::as_bits<f32>(v);
    static_assert(std::same_as<decltype(i), f32>);
    EXPECT_EQ(i, -1.89496550775e-14_f32);
    auto f = sus::as_bits<f64>(i);
    EXPECT_EQ(f, 1.39808630771690684819284234772e-314_f64);
  }
}

}  // namespace
