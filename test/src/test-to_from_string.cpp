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