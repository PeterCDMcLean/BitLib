#include <limits>

#include "bitlib/bitlib.hpp"
#include "gtest/gtest.h"

TEST(BitMultiplication, Basic) {
  auto bits = 0xF'0F_b;
  EXPECT_EQ(0, bit::multiplication(bits.begin(), bits.end(), static_cast<uint8_t>(0x2u)));
  EXPECT_EQ(0xF'1E_b, bits);
}

TEST(BitMultiplication, AllOnes) {
  auto bits = 0xF'00FF_b;
  EXPECT_EQ(1, bit::multiplication(bits.begin(), bits.end(), 8'255_b));
  EXPECT_EQ(0xF'7E01_b, bits);
}

TEST(BitMultiplication, LowBits) {
  auto bits = 0xF'F_b;
  EXPECT_EQ(0, bit::multiplication(bits.begin(), bits.end(), 2'2_b));
  EXPECT_EQ(0xF'1E_b, bits);
}
