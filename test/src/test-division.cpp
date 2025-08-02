#include <limits>

#include "bitlib/bitlib.hpp"
#include "gtest/gtest.h"

TEST(BitDivision, Basic) {
  auto bits = 0xF'0F_b;
  EXPECT_EQ((15 % 2), bit::division(bits.begin(), bits.end(), static_cast<uint8_t>(0x2u)));
  EXPECT_EQ(0xF'07_b, bits);
}

TEST(BitDivision, AllOnes) {
  auto bits = 0xF'00FF_b;
  EXPECT_EQ((255 % 255), bit::division(bits.begin(), bits.end(), 8'255_b));
  EXPECT_EQ(0xF'0001_b, bits);
}

TEST(BitDivision, LowBits) {
  auto bits = 0xF'F_b;
  EXPECT_EQ((15 % 2), bit::division(bits.begin(), bits.end(), 2'2_b));
  EXPECT_EQ(0xF'07_b, bits);
}
