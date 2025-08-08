#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "bitlib/bit-algorithms/count.hpp"
#include "bitlib/bit-algorithms/to_from_string.hpp"
#include "bitlib/bit-containers/bit_array.hpp"
#include "bitlib/bit-containers/bit_array_dynamic_extent.hpp"
#include "fixtures.hpp"

// Third-party libraries
#include "gtest/gtest.h"

TEST(ToString, Blah) {
  auto num = 0x20'DEADBEEF_b;
  EXPECT_EQ(static_cast<uint32_t>(num), 0xDEADBEEF);
  auto str = bit::to_string<bit::string::typical(16)>(num);
  EXPECT_EQ(str, "DEADBEEF");
  num = 0x20'0EADBEEF_b;
  str = bit::to_string<bit::string::typical(16)>(num);
  EXPECT_EQ(str, "EADBEEF");
  str = bit::to_string<bit::string::typical(16, true)>(num);
  EXPECT_EQ(str, "0EADBEEF");
  auto num2 = 0x23'10EADBEEF_b;
  EXPECT_EQ(num2.size(), 35);
  str = bit::to_string<bit::string::typical(16)>(num2);
  EXPECT_EQ(str, "10EADBEEF");
}

TEST(ToString, base10) {
  auto num = 10'123_b;
  auto str = bit::to_string<bit::string::typical(10)>(num);
  EXPECT_EQ(str, "123");
}

TEST(ToString, Streaming) {
  std::stringstream sstr;
  auto num = 10'123_b;
  sstr << num;
  EXPECT_EQ(sstr.str(), "123");
}

TEST(FromString, Blah) {
  bit::bit_array<16> arr_16;
  bit::from_string<bit::string::typical(16)>("DEADBEEF", arr_16);
  EXPECT_EQ(arr_16, 0x10'BEEF_b);
  bit::bit_array<18> arr_18;
  bit::from_string<bit::string::typical(16)>("BEEF", arr_18);
  EXPECT_EQ(arr_18, 0x12'0BEEF_b);
  bit::from_string<bit::string::typical(10)>("123", arr_16);
  EXPECT_EQ(arr_16, 16'123_b);
}

TEST(FromString, IntoBookendRange) {
  for (const auto word : get_random_vec<uint32_t>(64)) {
    for (int i = 1; i < 8; i++) {
      auto bits = bit::bit_array<32>(word);
      auto bits2 = bits;
      bit::from_string<bit::string::typical(16)>(
          "123ABC", bits(i, i + 4 * 6));
      bits2(i, i + 4 * 6) = 0x18'123ABC_b;
      EXPECT_EQ(bits, bits2);
    }
    for (int i = 1; i < 8; i++) {
      auto bits = bit::bit_array<32>(word);
      auto bits2 = bits;
      bit::from_string<bit::string::typical(16)>(
          "F0000F", bits(i, i + 4 * 6));
      bits2(i, i + 4 * 6) = 0x18'F0000F_b;
      EXPECT_EQ(bits, bits2);
    }
  }
}

TEST(FromString, IntoBookendRangeOverflow) {
  for (int i = 1; i < 8; i++) {
    auto bits = 0x20'DEADBEEF_b;
    auto bits2 = bits;
    bit::from_string<bit::string::typical(16)>(
        "F123ABC", bits(i, i + 4 * 6));
    bits2(i, i + 4 * 6) = 0x18'123ABC_b;
    EXPECT_EQ(bits, bits2);
  }
  for (int i = 1; i < 8; i++) {
    auto bits = 0x20'00000000_b;
    auto bits2 = bits;
    bit::from_string<bit::string::typical(16)>(
        "F123ABC", bits(i, i + 4 * 6));
    bits2(i, i + 4 * 6) = 0x18'123ABC_b;
    EXPECT_EQ(bits, bits2);
  }
}

TEST(FromString, IntoBookendRangeLarge) {
  for (int i = 0; i < 128; i++) {
    for (auto b : {bit::bit0, bit::bit1}) {
      auto bits = bit::bit_array<>(200 + i, b);
      auto bits2 = bits;
      auto setbits = bit::bit_array<>(200 + i, b ? bit::bit0 : bit::bit1);
      auto str = bit::to_string<bit::string::typical(16, true)>(setbits);
      for (int j = 64 / 4; j < 128 / 4; j++) {
        for (int k = 0; k < 64; k++) {
          bit::from_string<bit::string::typical(16)>(str.substr(str.length() - 1 - j, j), bits(k, k + j * 4));
          bits2(k, k + j * 4) = setbits(0, j * 4);
          EXPECT_EQ(bits, bits2);
        }
      }
    }
  }
}