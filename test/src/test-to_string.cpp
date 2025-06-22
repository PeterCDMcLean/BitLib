#include <stdexcept>
#include <string_view>
#include <utility>

#include "bitlib/bit-algorithms/count.hpp"
#include "bitlib/bit-algorithms/to_string.hpp"
#include "bitlib/bit-containers/bit_array.hpp"
#include "bitlib/bit-containers/bit_array_dynamic_extent.hpp"
#include "fixtures.hpp"

// Third-party libraries
#include "gtest/gtest.h"

TEST(ToString, Blah) {
  auto num = 0x20'DEADBEEF_b;
  EXPECT_EQ(static_cast<uint32_t>(num), 0xDEADBEEF);
  auto str = bit::to_string<16>(num);
  EXPECT_EQ(str, "DEADBEEF");
  num = 0x20'0EADBEEF_b;
  str = bit::to_string<16>(num);
  EXPECT_EQ(str, "EADBEEF");
  str = bit::to_string<16, true>(num);
  EXPECT_EQ(str, "0EADBEEF");
  auto num2 = 0x23'10EADBEEF_b;
  EXPECT_EQ(num2.size(), 35);
  str = bit::to_string<16>(num2);
  EXPECT_EQ(str, "10EADBEEF");
}
