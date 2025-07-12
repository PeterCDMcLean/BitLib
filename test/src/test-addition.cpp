#include <limits>

#include "bitlib/bitlib.hpp"
#include "gtest/gtest.h"

TEST(BitAddition, Basic) {
  auto bits = 0xF'0F_b;
  EXPECT_EQ(0, bit::addition(bits.begin(), bits.end(), static_cast<uint8_t>(0x1u)));
  EXPECT_EQ(0xF'10_b, bits);
}

TEST(BitAddition, LowBits) {
  auto bits = 0x7'F_b;
  EXPECT_EQ(0, bit::addition(bits.begin(), bits.end(), 1'1_b));
  EXPECT_EQ(0x7'10_b, bits);
}
